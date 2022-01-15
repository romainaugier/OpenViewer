// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "app.h"

namespace Interface
{
    Application::Application(Logger* logger, Core::Loader* loader, Core::Ocio* ocio)
    {
        this->m_Logger = logger;
        this->m_Loader = loader;
        this->m_OcioModule = ocio;

        this->m_SettingsInterface.m_Settings.Initialize();

        const int cacheMode = this->m_SettingsInterface.m_Settings.m_UserSettings["cache_mode"].get<int>();
        char* settingsName = "cache_max_size";

        if (cacheMode == 2) settingsName = "cache_max_ram_usage";

        this->m_Loader->Initialize(cacheMode,
                                   this->m_SettingsInterface.m_Settings.m_UserSettings[settingsName].get<uint64_t>(),
                                   this->m_SettingsInterface.m_Settings.m_UserSettings["autodetect_file_sequences"].get<bool>());

		this->m_Loader->SetOpenExrThreadCount(this->m_SettingsInterface.m_Settings.m_UserSettings["openexr_threads"].get<uint8_t>());

        // Make sure all keys are set to 0
        for (int i = 0; i < GLFW_KEY_COUNT; i++) this->m_Shortcuts.m_Pressed[i] = false;
    }

    Display* Application::GetActiveDisplay() noexcept
    {
        for (const auto& [id, displayPair] : this->m_Displays)
        {
            if (displayPair.second->m_IsActive) return displayPair.second;
        }

        return nullptr;
    }

    void Application::UpdateDisplays() noexcept
    {
        for (auto it = this->m_Displays.cbegin(); it != this->m_Displays.cend();)
        {
            if (!it.value().second->m_IsOpen)
            {
                this->m_Logger->Log(LogLevel_Debug, "[DISPLAY] : Releasing display %d", it.value().second->m_DisplayID);

                this->m_Loader->SetMediaInactive(it.value().second->m_MediaID);

                it.value().second->Release();
                this->m_Displays.erase(it++);
                --this->m_DisplayCount;

                this->m_Logger->Log(LogLevel_Debug, "[DISPLAY] : Display count : %d", this->m_DisplayCount);
            }
            else
            {
                ++it;
            }
        }
    }

    void Application::HandleShortcuts() noexcept
    {
        for (uint16_t i = 0; i < GLFW_KEY_COUNT; i++)
        {
            if (!this->m_Shortcuts.m_Pressed[i]) continue;
            
            switch (i)
            {
                case GLFW_KEY_Q: // Q is A letter on french keyboards
                {
                    // Set alpha as the current channel to be displayed
                    if (this->m_OcioModule->m_CurrentChannelIdx != 4) 
                        this->m_OcioModule->m_CurrentChannelIdx = 4;
                    else
                        this->m_OcioModule->m_CurrentChannelIdx = 0;

                    this->m_OcioModule->UpdateChannelHot();
                    this->m_OcioModule->UpdateProcessor();
                    this->Changed();
                    break;
                }

                case GLFW_KEY_E:
                {
                    // Show media explorer window
                    this->ShowMediaExplorerWindow();
                    break;
                }

                case GLFW_KEY_F:
                {
                    // Frame current display view
                    this->GetActiveDisplay()->NeedFrame();
                    break;
                }

                case GLFW_KEY_G:
                {
                    // Home current display view
                    this->GetActiveDisplay()->NeedHome();
                    break;
                }

                case GLFW_KEY_H:
                {
                    // Mirror current displayed image horizontally
                    this->GetActiveDisplay()->MirrorHorizontal();
                    break;
                }

                case GLFW_KEY_I:
                {
                    // Show info windows
                    this->ShowImageInfosWindow();
                    this->ShowPixelInfosWindow();
                    break;
                }

                case GLFW_KEY_O:
                {
                    // TODO : Open file
                    if (this->m_Shortcuts.m_Pressed[GLFW_KEY_LEFT_CONTROL] ||
                        this->m_Shortcuts.m_Pressed[GLFW_KEY_RIGHT_CONTROL])
                    {
                        ifd::FileDialog::Instance().Open("FolderOpenDialog", "Open a directory", "");
                        break;
                    }
                    else
                    {
                        ifd::FileDialog::Instance().Open("SingleFileOpenDialog", 
                        "Select an image file", "Image File (*.exr,*.png;*.jpg;*.jpeg;*.bmp;*.tga){.exr,.png,.jpg,.jpeg,.bmp,.tga},.*");
                        break;
                    }
                }

                case GLFW_KEY_V:
                {
                    // Mirror current displayed image vertically
                    this->GetActiveDisplay()->MirrorVertical();
                    break;
                }

                case GLFW_KEY_W: 
                {
                    // Close active display
                    break;
                }

                case GLFW_KEY_SPACE:
                {
                    // Play/Pause
                    if (this->m_Playbar->IsPlaying()) this->m_Playbar->Pause();
                    else this->m_Playbar->Play();
                    break;
                }

                // Arrows

                case GLFW_KEY_UP:
                {
                    // Go up in menubar menus
                    const uint8_t modeCount = this->m_Menubar->GetModeCount();
                    const uint8_t currentMode = this->m_Menubar->GetMode();
                    const uint8_t newMode = currentMode == 0 ? (modeCount - 1) : (currentMode - 1); 

                    this->m_Menubar->SetMode(newMode);
                    break;
                }

                case GLFW_KEY_DOWN:
                {
                    // Go down in menubar menus
                    const uint8_t modeCount = this->m_Menubar->GetModeCount();
                    const uint8_t currentMode = this->m_Menubar->GetMode();
                    const uint8_t newMode = currentMode == (modeCount - 1) ? 0 : currentMode + 1; 
                    
                    this->m_Menubar->SetMode(newMode);
                    break;
                }

                case GLFW_KEY_LEFT:
                {
                    // Go one frame left in the playbar
                    if (this->m_Shortcuts.m_Pressed[GLFW_KEY_LEFT_CONTROL] ||
                        this->m_Shortcuts.m_Pressed[GLFW_KEY_RIGHT_CONTROL])
                    {
                        this->m_Playbar->GoFirstFrame();
                    }
                    else this->m_Playbar->GoPreviousFrame();

                    break;
                }
                
                case GLFW_KEY_RIGHT:
                {
                    // Go one frame right in the playbar
                    if (this->m_Shortcuts.m_Pressed[GLFW_KEY_LEFT_CONTROL] ||
                        this->m_Shortcuts.m_Pressed[GLFW_KEY_RIGHT_CONTROL])
                    {
                        this->m_Playbar->GoLastFrame();
                    }
                    else this->m_Playbar->GoNextFrame();

                    break;
                }
            }

            glfwWaitEventsTimeout(0.75);
        }

        if (ifd::FileDialog::Instance().IsDone("SingleFileOpenDialog"))
        {
            if (ifd::FileDialog::Instance().HasResult())
            {
                const auto& res = ifd::FileDialog::Instance().GetResults();
                const std::string fp = res[0].u8string();

                if (!this->m_Loader->m_HasBeenInitialized)
                {
                    this->m_Loader->Initialize(this->m_SettingsInterface.m_Settings.m_UserSettings["cache_mode"].get<int>(),
                                               this->m_SettingsInterface.m_Settings.m_UserSettings["cache_max_size"].get<uint64_t>());
                }

                this->m_Loader->Load(fp);
                this->m_Loader->LoadImageToCache(this->m_Playbar->m_Frame);
            }

            ifd::FileDialog::Instance().Close();
        }

        if (ifd::FileDialog::Instance().IsDone("FolderOpenDialog"))
        {
            if (ifd::FileDialog::Instance().HasResult())
            {
                const auto& res = ifd::FileDialog::Instance().GetResult();
                const std::string fp = res.u8string();

                if (!this->m_Loader->m_HasBeenInitialized)
                {
                    this->m_Loader->Initialize(this->m_SettingsInterface.m_Settings.m_UserSettings["cache_mode"].get<int>(),
                                               this->m_SettingsInterface.m_Settings.m_UserSettings["cache_max_size"].get<uint64_t>());
                }

                this->m_Loader->Load(fp);
                this->m_Loader->LoadImageToCache(this->m_Playbar->m_Frame);
            }

            ifd::FileDialog::Instance().Close();
        }
    }

    void Application::CacheSettingsChanged() noexcept
    {
        if (this->m_SettingsInterface.m_Settings.m_CacheSettingsChanged)
        {
            this->m_Playbar->Pause();
            this->m_Playbar->m_Frame = 0;
            
            // Find the biggest image we have in cache to make sure enough space will be allocated in the cache 
            // Especially when size is set manually
            uint64_t biggestImageSize = 0;

            for (const auto& media : this->m_Loader->m_Medias)
            {
                for (const auto& image : media.m_Images)
                {
                    biggestImageSize = image.m_Stride > biggestImageSize ? image.m_Stride : biggestImageSize;
                }
            }

            const int cacheMode = this->m_SettingsInterface.m_Settings.m_UserSettings["cache_mode"].get<int>();

            // If we use the cache, and the loader is already initialized, just initialize the cache
            if (cacheMode > 0)
            {
                this->m_Loader->m_CacheMode = cacheMode;

                uint64_t newCacheSize = static_cast<uint64_t>(this->m_SettingsInterface.m_Settings.m_UserSettings["cache_max_size"].get<uint64_t>()) * 1000000;

				if (cacheMode == 2)
				{
					newCacheSize = Utils::GetTotalSystemMemory() * this->m_SettingsInterface.m_Settings.m_UserSettings["cache_max_ram_usage"].get<uint64_t>() / 100;
				}

                if (newCacheSize < biggestImageSize)
                {
                    this->m_Logger->Log(LogLevel_Warning, 
                                        "[CACHE] : New size (%d MB) is not enough, setting size to %f MB",
                                        this->m_SettingsInterface.m_Settings.m_UserSettings["cache_max_size"].get<uint64_t>(),
                                        static_cast<float>(biggestImageSize) / 1000000.0f);

                    this->m_SettingsInterface.m_Settings.m_UserSettings["cache_max_size"] = static_cast<int>(biggestImageSize / 1000000);
                }

                if (this->m_Loader->m_Cache->m_HasBeenInitialized)
                {
                    this->m_Loader->m_Cache->Resize(newCacheSize);
                }
                else
                {
                    this->m_Loader->m_Cache->Initialize(newCacheSize, this->m_Logger);
                }

                // Load the first image
                this->m_Loader->LoadImageToCache(0);

                // Launch the sequence worker
                this->m_Loader->m_Workers.emplace_back(&Core::Loader::LoadSequenceToCache, this->m_Loader, 0, 0);

                // Launch the bg loader
                this->m_Loader->LaunchCacheLoader();
            }
            else
            {
                this->m_Loader->StopCacheLoader();

                this->m_Loader->m_CacheMode = 0;
                this->m_Loader->m_Cache->Release();

                this->m_Loader->m_Cache->Initialize(biggestImageSize, this->m_Logger, false);

                this->m_Loader->LoadImageToCache(0);
            }
        }

        this->m_SettingsInterface.m_Settings.m_CacheSettingsChanged = false;
    }

    void Application::Release() noexcept
    {
        this->m_SettingsInterface.m_Settings.WriteUserSettings();

        for (auto& [id, display] : this->m_Displays)
        {
            display.second->Release();
        }

        this->m_Logger->Log(LogLevel_Diagnostic, "[MAIN] : Released OpenViewer");
    }

    void Menubar::Draw(Application& app,
				       ImPlaybar& playbar, 
				       Core::Ocio& ocio, 
				       Profiler& prof, 
				       bool& change) noexcept
	{
		ImGui::BeginMainMenuBar();
		{
			// Classic menu
			if (this->m_BarMode == 0)
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Open Single File", "CTRL + O"))
					{
						ifd::FileDialog::Instance().Open("SingleFileOpenDialog", "Select an image file", "Image File (*.exr,*.png;*.jpg;*.jpeg;*.bmp;*.tga){.exr,.png,.jpg,.jpeg,.bmp,.tga},.*");
						this->m_HasOpenedIFD = true;
					}

					if (ImGui::MenuItem("Open Folder", "CTRL + K"))
					{
						ifd::FileDialog::Instance().Open("FolderOpenDialog", "Open a directory", "");
						this->m_HasOpenedIFD = true;
					}

					if (ImGui::MenuItem("Media Explorer", "M"))
					{
						app.showMediaExplorerWindow = true;
					}

					ImGui::EndMenu();
				}

				if (ifd::FileDialog::Instance().IsDone("SingleFileOpenDialog"))
				{
					if (ifd::FileDialog::Instance().HasResult())
					{
						const auto& res = ifd::FileDialog::Instance().GetResults();
						const std::string fp = res[0].u8string();

						if (!app.m_Loader->m_HasBeenInitialized)
						{
							app.m_Loader->Initialize(app.m_SettingsInterface.m_Settings.m_UserSettings["cache_mode"].get<int>(),
                                                     app.m_SettingsInterface.m_Settings.m_UserSettings["cache_max_size"].get<uint64_t>());
						}

						app.m_Loader->Load(fp);
						app.m_Loader->LoadImageToCache(app.m_Playbar->m_Frame);
					}

					ifd::FileDialog::Instance().Close();
				}

				if (ifd::FileDialog::Instance().IsDone("FolderOpenDialog"))
				{
					if (ifd::FileDialog::Instance().HasResult())
					{
						const auto& res = ifd::FileDialog::Instance().GetResult();
						const std::string fp = res.u8string();

						if (!app.m_Loader->m_HasBeenInitialized)
						{
							app.m_Loader->Initialize(app.m_SettingsInterface.m_Settings.m_UserSettings["cache_mode"].get<int>(),
                                                     app.m_SettingsInterface.m_Settings.m_UserSettings["cache_max_size"].get<uint64_t>());
						}

						app.m_Loader->Load(fp);
						app.m_Loader->LoadImageToCache(app.m_Playbar->m_Frame);
					}

					ifd::FileDialog::Instance().Close();
				}

				this->m_HasOpenedIFD = false;
				
				// if (ImGui::BeginMenu("Plot"))
				// {
				// 	if (ImGui::MenuItem("Histogram")) {}
				// 	if (ImGui::MenuItem("Waveform")) {}
				// 	if (ImGui::MenuItem("Parade"))
				// 	{
				// 		if (currentSettings.settings.parade) currentSettings.settings.parade = false;
				// 		else currentSettings.settings.parade = true;
				// 	}
				// 	if (ImGui::MenuItem("Vector Scope")) {}
				// 	if (ImGui::MenuItem("Custom")) {}

				// 	ImGui::EndMenu();
				// }

				if (ImGui::BeginMenu("Infos"))
				{
					if (ImGui::MenuItem("Image Infos")) { app.showImageInfosWindow = true; }
					if (ImGui::MenuItem("Pixel Infos")) { app.showPixelInfosWindow = true; }

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Settings"))
				{
					if (ImGui::MenuItem("Settings")) { app.m_SettingsInterface.ShowSettingsWindow(); }
					if (ImGui::MenuItem("Debug")) { app.m_SettingsInterface.ShowDebugWindow(); }


					ImGui::EndMenu();
				}
			}

			// Cache Menu
			else if (this->m_BarMode == 1)
			{
				int cacheMode = app.m_SettingsInterface.m_Settings.m_UserSettings["cache_mode"].get<int>();

				if (cacheMode == 0)
                {
                    ImGui::Text("Mode : Minimal");

                    ImGui::Dummy(ImVec2(10.0f, 10.0f));
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
					ImGui::Dummy(ImVec2(10.0f, 10.0f));

					const float cacheCapacity = static_cast<float>(app.m_Loader->m_Cache->m_BytesCapacity) / 1000000.0f;
					ImGui::Text("Capacity : %.2f MB", cacheCapacity);
                
                    ImGui::Dummy(ImVec2(10.0f, 10.0f));
                    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                    ImGui::Dummy(ImVec2(10.0f, 10.0f));
                } 
                else if (cacheMode == 1)
				{
					ImGui::Text("Mode : Manual");

					ImGui::Dummy(ImVec2(10.0f, 10.0f));
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
					ImGui::Dummy(ImVec2(10.0f, 10.0f));
					
					ImGui::Text("Capacity (MB) : ");

                    int cacheCapacity = app.m_SettingsInterface.m_Settings.m_UserSettings["cache_max_size"].get<uint64_t>();

					ImGui::SetNextItemWidth(50);
					ImGui::PushID(1);
					ImGui::InputInt("", &cacheCapacity, 0);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
                        app.m_SettingsInterface.m_Settings.m_UserSettings["cache_max_size"] = cacheCapacity;
						app.m_SettingsInterface.m_Settings.m_CacheSettingsChanged = true;
					}
					
					ImGui::PopID();

					ImGui::Dummy(ImVec2(10.0f, 10.0f));
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
					ImGui::Dummy(ImVec2(10.0f, 10.0f));
				}
				else if (cacheMode == 2)
				{
					ImGui::Text("Mode : Smart");
					
					ImGui::Dummy(ImVec2(10.0f, 10.0f));
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
					ImGui::Dummy(ImVec2(10.0f, 10.0f));

					const float cacheCapacity = static_cast<float>(app.m_Loader->m_Cache->m_BytesCapacity) / 1000000.0f;
					ImGui::Text("Capacity : %.2f MB", cacheCapacity);
                
                    ImGui::Dummy(ImVec2(10.0f, 10.0f));
                    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                    ImGui::Dummy(ImVec2(10.0f, 10.0f));
                }
				
                const float cacheSize = static_cast<float>(app.m_Loader->m_Cache->m_BytesSize) / 1000000.0f;
                ImGui::Text("Size : %.2f MB", cacheSize);

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                ImGui::Dummy(ImVec2(10.0f, 10.0f));

                const float cacheUsage = static_cast<float>(app.m_Loader->m_Cache->m_BytesSize) / static_cast<float>(app.m_Loader->m_Cache->m_BytesCapacity) * 100.0f;
                ImGui::Text("Usage : %.2f%%", cacheUsage);

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                ImGui::Dummy(ImVec2(10.0f, 10.0f));

                ImGui::Text("Images In Cache : %d", app.m_Loader->m_Cache->m_Size);
			}

			// OCIO Menu
			else if (this->m_BarMode == 2)
			{
				// Here we have the OCIO menus to select the role, display and view
				// Each time we choose something, the item is updated, the ocio
				// processor too, and we redisplay

				const ImVec2 avail_width = ImGui::GetContentRegionAvail();

				// ImGui::Dummy(ImVec2(50.0f, avail_width.y));

				// Channels
				static const char* channels[] = {"RGB", "R", "G", "B", "A", "L"};

				ImGui::Text("Channels");
				ImGui::PushID(0);
				const float channelsWidth = ImGui::CalcTextSize(channels[ocio.m_CurrentChannelIdx]).x + 30.0f;
				ImGui::SetNextItemWidth(channelsWidth);
				ImGui::Combo("", &ocio.m_CurrentChannelIdx, &channels[0], IM_ARRAYSIZE(channels));
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.UpdateChannelHot();

					ocio.UpdateProcessor();

					change = true;
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

				// Role
				ImGui::Text("Role");
				ImGui::PushID(1);
				const float roleWidth = ImGui::CalcTextSize(ocio.m_CurrentRole.c_str()).x + 30.0f;
				ImGui::SetNextItemWidth(roleWidth);
				
				// ImGui::Combo("", &ocio.m_CurrentRoleIdx, &ocio.m_Roles[0], ocio.m_Roles.size());

				ImGui::Combo("", &ocio.m_CurrentRoleIdx, 
					         [](void* vec, int idx, const char** out_text){
					         	std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
					         	if (idx < 0 || idx >= vector ->size()) return false;
					         	*out_text = vector->at(idx).c_str();
					         	return true;
					         }, reinterpret_cast<void*>(&ocio.m_Roles), ocio.m_Roles.size());

				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.m_CurrentRole = ocio.m_Roles[ocio.m_CurrentRoleIdx];

					ocio.UpdateProcessor();

					change = true;
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

				// Display
				ImGui::Text("Display");
				ImGui::PushID(2);
				const float displayWidth = ImGui::CalcTextSize(ocio.m_CurrentDisplay.c_str()).x + 30.0f;
				ImGui::SetNextItemWidth(displayWidth);
				
				// ImGui::Combo("", &ocio.m_CurrentDisplayIdx, &ocio.m_Displays[0], ocio.m_Displays.size());
				
				ImGui::Combo("", &ocio.m_CurrentDisplayIdx, 
					         [](void* vec, int idx, const char** out_text){
					         	std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
					         	if (idx < 0 || idx >= vector ->size()) return false;
					         	*out_text = vector->at(idx).c_str();
					         	return true;
					         }, reinterpret_cast<void*>(&ocio.m_Displays), ocio.m_Displays.size());

				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.m_CurrentDisplay = ocio.m_Displays[ocio.m_CurrentDisplayIdx];

					ocio.GetOcioDisplayViews();
					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

				// View
				ImGui::Text("View");
				ImGui::PushID(3);
				const float viewWidth = ImGui::CalcTextSize(ocio.m_CurrentView.c_str()).x + 30.0f;
				ImGui::SetNextItemWidth(viewWidth);

				// ImGui::Combo("", &ocio.m_CurrentViewIdx, &ocio.m_Views[0], ocio.m_Views.size());
				
				ImGui::Combo("", &ocio.m_CurrentViewIdx, 
					         [](void* vec, int idx, const char** out_text){
					         	std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
					         	if (idx < 0 || idx >= vector ->size()) return false;
					         	*out_text = vector->at(idx).c_str();
					         	return true;
					         }, reinterpret_cast<void*>(&ocio.m_Views), ocio.m_Views.size());

				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.m_CurrentView = ocio.m_Views[ocio.m_CurrentViewIdx];
					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

				// Look
				ImGui::Text("Look");
				ImGui::PushID(4);
				const float lookWidth = ImGui::CalcTextSize(ocio.m_CurrentLook.c_str()).x + 30.0f;
				ImGui::SetNextItemWidth(lookWidth);
				
				//ImGui::Combo("", &ocio.m_CurrentLookIdx, &ocio.m_Looks[0], ocio.m_Looks.size());
				
				ImGui::Combo("", &ocio.m_CurrentLookIdx, 
					         [](void* vec, int idx, const char** out_text){
					         	std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
					         	if (idx < 0 || idx >= vector ->size()) return false;
					         	*out_text = vector->at(idx).c_str();
					         	return true;
					         }, reinterpret_cast<void*>(&ocio.m_Looks), ocio.m_Looks.size());
				
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.m_CurrentLook = ocio.m_Looks[ocio.m_CurrentLookIdx];
					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

				// Exponent
				ImGui::Text("Exp");
				ImGui::PushID(5);
				ImGui::SetNextItemWidth(150.0f);
				ImGui::SliderFloat("", &ocio.m_ExposureStops, -10.0f, 10.0f);
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

				// Gamma
				ImGui::Text("Gamma");
				ImGui::PushID(6);
				ImGui::SetNextItemWidth(150.0f);
				ImGui::SliderFloat("", &ocio.m_Gamma, 0.0f, 4.0f);
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.UpdateProcessor();

					// display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

				// Reset button for Exponent/Gamma
				if (ImGui::Button("Reset"))
				{
					ocio.m_ExposureStops = 0.0f;
					ocio.m_Gamma = 1.0f;

					ocio.UpdateProcessor();

					// display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}
			}

			// Display
			else if (this->m_BarMode == 3)
			{
				Display* activeDisplay = app.GetActiveDisplay();

				if (app.m_DisplayCount > 0 && activeDisplay != nullptr)
				{
					const ImVec2 availWidth = ImGui::GetContentRegionAvail();

					ImGui::Text("This section is in progress");

					ImGui::Dummy(ImVec2(10.0f, 10.0f));
					
					Core::Media* activeMedia = app.m_Loader->GetMedia(activeDisplay->m_MediaID);

					if (activeMedia->m_Layers.size() > 4)
					{
						ImGui::Text("Layer");
						ImGui::PushID(0);
						const float textWidth = ImGui::CalcTextSize(activeMedia->m_CurrentLayerStr.c_str()).x + 30.0f;
						ImGui::SetNextItemWidth(textWidth);
						
						ImGui::Combo("", &activeMedia->m_CurrentLayerID, 
								[](void* vec, int idx, const char** out_text){
									std::vector<Core::Layer>* vector = reinterpret_cast<std::vector<Core::Layer>*>(vec);
									if (idx < 0 || idx >= vector ->size()) return false;
									*out_text = vector->at(idx).first.c_str();
									return true;
								}, reinterpret_cast<void*>(&activeMedia->m_Layers), activeMedia->m_Layers.size());

						ImGui::PopID();

						if (ImGui::IsItemEdited())
						{
							activeMedia->UpdateCurrentLayer();

							app.m_Loader->m_Cache->Flush();

							// Force the frame reloading
							app.m_Loader->LoadImageToCache(app.m_Playbar->m_Frame, true);

							change = true;
						}
						
						ImGui::Dummy(ImVec2(10.0f, 10.0f));
					}

					ImGui::Text("Premultiply");
					IM_ID(2, ImGui::Checkbox("###", &app.GetActiveDisplay()->m_PremultiplyAlpha));
					ImGui::Dummy(ImVec2(10.0f, 10.0f));

					ImGui::Text("Background");
					static const char* backgroundModes[] = { "Black", "Gray", "Checker" };
					ImGui::SetNextItemWidth(75.0f);
					IM_ID(3, ImGui::Combo("###", &app.GetActiveDisplay()->m_BackGroundMode, &backgroundModes[0], IM_ARRAYSIZE(backgroundModes)));
				}
				else
				{
					ImGui::Text("No active display");
				}
			}

			// Combo to select the bar mode

			const ImVec2 avail_width = ImGui::GetContentRegionAvail();

			ImGui::Dummy(ImVec2(avail_width.x - 100.0f, avail_width.y));

			static const char* modes[] = {"Menu", "Cache", "OCIO", "Display"};

			ImGui::PushID(99);
			ImGui::SetNextItemWidth(100.0f);
			ImGui::Combo("", &this->m_BarMode, &modes[0], IM_ARRAYSIZE(modes));
			ImGui::PopID();
		}

		ImGui::EndMainMenuBar();
	}
}