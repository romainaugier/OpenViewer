// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "menubar.h"

namespace Interface
{
	void Menubar::Draw(Settings_Windows& currentSettings, 
				       Application& app,
				       ImPlaybar& playbar, 
				       Core::Ocio& ocio, 
				       Profiler& prof, 
				       bool& change) noexcept
	{
		ImGui::SetNextWindowBgAlpha(currentSettings.settings.interface_windows_bg_alpha);

		ImGui::BeginMainMenuBar();
		{
			// Classic menu
			if (this->m_BarMode == 0)
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Open Single File"))
					{
						ifd::FileDialog::Instance().Open("SingleFileOpenDialog", "Select an image file", "Image File (*.exr,*.png;*.jpg;*.jpeg;*.bmp;*.tga){.exr,.png,.jpg,.jpeg,.bmp,.tga},.*");
						this->m_HasOpenedIFD = 1;
					}

					if (ImGui::MenuItem("Open Folder"))
					{
						ifd::FileDialog::Instance().Open("FolderOpenDialog", "Open a directory", "");
						this->m_HasOpenedIFD = true;
					}

					if (ImGui::MenuItem("Media Explorer"))
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

						app.m_Loader->Load(fp);
						app.m_Loader->LoadImageToCache(playbar.m_Frame);
					}

					ifd::FileDialog::Instance().Close();
				}

				if (ifd::FileDialog::Instance().IsDone("FolderOpenDialog"))
				{
					if (ifd::FileDialog::Instance().HasResult())
					{
						const auto& res = ifd::FileDialog::Instance().GetResult();
						const std::string fp = res.u8string();

						app.m_Loader->Load(fp);
						app.m_Loader->LoadImageToCache(playbar.m_Frame);
					}

					ifd::FileDialog::Instance().Close();
				}

				this->m_HasOpenedIFD = false;
				

				if (ImGui::BeginMenu("Plot"))
				{
					if (ImGui::MenuItem("Histogram")) {}
					if (ImGui::MenuItem("Waveform")) {}
					if (ImGui::MenuItem("Parade"))
					{
						if (currentSettings.settings.parade) currentSettings.settings.parade = false;
						else currentSettings.settings.parade = true;
					}
					if (ImGui::MenuItem("Vector Scope")) {}
					if (ImGui::MenuItem("Custom")) {}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Infos"))
				{
					if (ImGui::MenuItem("Sequence Infos")) { app.showMediaInfosWindow = true; }
					if (ImGui::MenuItem("Image Infos")) { app.showImageInfosWindow = true; }
					
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Settings"))
				{
					if (ImGui::MenuItem("Playback")) { currentSettings.p_open_playback_window = true; }
					if (ImGui::MenuItem("OCIO")) { currentSettings.p_open_ocio_window = true; }
					if (ImGui::MenuItem("Interface")) { currentSettings.p_open_interface_window = true; }
					if (ImGui::MenuItem("Performance")) { currentSettings.p_open_performance_window = true; }


					ImGui::EndMenu();
				}

				const ImVec2 leftSpace = ImGui::GetContentRegionAvail();

				ImGui::Dummy(ImVec2(leftSpace.x - 500.0f, leftSpace.y));

				ImGui::Checkbox("Use Cache", &currentSettings.settings.m_UseCache);
				if (ImGui::IsItemEdited())
				{
					playbar.Pause();
					playbar.m_Frame = 0;

					// If we use the cache, and the loader is already initialized, just initialize the cache
					if (currentSettings.settings.m_UseCache)
					{
						app.m_Loader->m_UseCache = true;

						if (app.m_Loader->m_Cache->m_HasBeenInitialized)
						{
							app.m_Loader->m_Cache->Resize(currentSettings.settings.m_CacheSize);
						}

						// Load the first image
						app.m_Loader->LoadImageToCache(0);

						// Launch the sequence worker
						app.m_Loader->m_Workers.emplace_back(&Core::Loader::LoadSequenceToCache, app.m_Loader, 0, 0);

						// Launch the bg loader
						app.m_Loader->LaunchCacheLoader();
					}
					else
					{
						app.m_Loader->StopCacheLoader();

						app.m_Loader->m_UseCache = false;
						app.m_Loader->m_Cache->Release();

						uint64_t biggestImageSize = 0;

						for (const auto& media : app.m_Loader->m_Medias)
						{
							for (const auto& image : media.m_Images)
							{
								biggestImageSize = image.m_Stride > biggestImageSize ? image.m_Stride : biggestImageSize;
							}
						}

						app.m_Loader->m_Cache->Initialize(biggestImageSize, app.m_Logger, false);

						app.m_Loader->LoadImageToCache(0);
					}
				}

				if (currentSettings.settings.m_UseCache)
				{
					ImGui::Dummy(ImVec2(5.0f, leftSpace.y));

					ImGui::Text("Size (MB) : ");

					ImGui::SetNextItemWidth(50);
					ImGui::PushID(50);
					ImGui::InputInt("", &currentSettings.settings.m_CacheSize, 0);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						if (currentSettings.settings.m_UseCache)
						{
							playbar.Pause();
							playbar.m_Frame = 0;

							app.m_Loader->StopCacheLoader();

							app.m_Loader->m_Cache->Resize(currentSettings.settings.m_CacheSize);

							// Launch the sequence worker
							app.m_Loader->m_Workers.emplace_back(&Core::Loader::LoadSequenceToCache, app.m_Loader, 0, 0);

							// Launch the bg loader
							app.m_Loader->LaunchCacheLoader();
						}
					}
					
					ImGui::PopID();

					ImGui::Dummy(ImVec2(5.0f, leftSpace.y));

					const float cacheUsage = static_cast<float>(app.m_Loader->m_Cache->m_BytesSize) / static_cast<float>(app.m_Loader->m_Cache->m_BytesCapacity) * 100.0f;

					ImGui::Text("Usage : %.2f%%", cacheUsage);
				}
			}

			// OCIO Menu
			else if (this->m_BarMode == 1)
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
				ImGui::SetNextItemWidth(100.0f);
				ImGui::Combo("", &ocio.current_channel_idx, &channels[0], IM_ARRAYSIZE(channels));
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					if (ocio.current_channel_idx == 0) // RGB
					{
						ocio.channel_hot[0] = 1;
						ocio.channel_hot[1] = 1;
						ocio.channel_hot[2] = 1;
						ocio.channel_hot[3] = 1;
					}
					else if (ocio.current_channel_idx == 1) // R
					{
						ocio.channel_hot[0] = 1;
						ocio.channel_hot[2] = 0;
						ocio.channel_hot[3] = 0;
						ocio.channel_hot[1] = 0;
					}
					else if (ocio.current_channel_idx == 2) // G
					{
						ocio.channel_hot[0] = 0;
						ocio.channel_hot[1] = 1;
						ocio.channel_hot[2] = 0;
						ocio.channel_hot[3] = 0;
					}						  
					else if (ocio.current_channel_idx == 3) // B
					{
						ocio.channel_hot[0] = 0;
						ocio.channel_hot[1] = 0;
						ocio.channel_hot[2] = 1;
						ocio.channel_hot[3] = 0;
					}
					else if (ocio.current_channel_idx == 4) // A
					{
						ocio.channel_hot[0] = 0;
						ocio.channel_hot[1] = 0;
						ocio.channel_hot[2] = 0;
						ocio.channel_hot[3] = 1;
					}
					else if (ocio.current_channel_idx == 5) // Luminance
					{
						ocio.channel_hot[0] = 1;
						ocio.channel_hot[1] = 1;
						ocio.channel_hot[2] = 1;
						ocio.channel_hot[3] = 0;
					}

					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				ImGui::Dummy(ImVec2(50.0f, avail_width.y));

				// Role
				ImGui::Text("Role");
				ImGui::PushID(1);
				static const float width = ImGui::CalcTextSize(ocio.current_role).x;
				ImGui::SetNextItemWidth(width);
				ImGui::Combo("", &ocio.current_role_idx, &ocio.roles[0], ocio.roles.size());
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.current_role = ocio.roles[ocio.current_role_idx];

					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				// Display
				ImGui::Text("Display");
				ImGui::PushID(2);
				ImGui::SetNextItemWidth(100.0f);
				ImGui::Combo("", &ocio.current_display_idx, &ocio.displays[0], ocio.displays.size());
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.current_display = ocio.displays[ocio.current_display_idx];

					ocio.GetOcioDisplayViews();
					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				// View
				ImGui::Text("View");
				ImGui::PushID(3);
				ImGui::SetNextItemWidth(100.0f);
				ImGui::Combo("", &ocio.current_view_idx, &ocio.views[0], ocio.views.size());
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.current_view = ocio.views[ocio.current_view_idx];
					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				// Look
				ImGui::Text("Look");
				ImGui::PushID(4);
				ImGui::SetNextItemWidth(100.0f);
				ImGui::Combo("", &ocio.current_look_idx, &ocio.looks[0], ocio.looks.size());
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.current_look = ocio.looks[ocio.current_look_idx];
					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				ImGui::Dummy(ImVec2(50.0f, avail_width.y));

				// Exponent
				ImGui::Text("Exp");
				ImGui::PushID(5);
				ImGui::SetNextItemWidth(150.0f);
				ImGui::SliderFloat("", &ocio.exposure_stops, -10.0f, 10.0f);
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.UpdateProcessor();

					//display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				ImGui::Dummy(ImVec2(20.0f, avail_width.y));

				// Gamma
				ImGui::Text("Gamma");
				ImGui::PushID(6);
				ImGui::SetNextItemWidth(150.0f);
				ImGui::SliderFloat("", &ocio.gamma, 0.0f, 4.0f);
				ImGui::PopID();

				if (ImGui::IsItemEdited())
				{
					ocio.UpdateProcessor();

					// display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}

				ImGui::Dummy(ImVec2(20.0f, avail_width.y));

				// Reset button for Exponent/Gamma
				if (ImGui::Button("Reset"))
				{
					ocio.exposure_stops = 0.0f;
					ocio.gamma = 1.0f;

					ocio.UpdateProcessor();

					// display.Update(loader, ocio, playbar.playbar_frame);

					change = true;
				}
			}

			// Combo to select the bar mode

			const ImVec2 avail_width = ImGui::GetContentRegionAvail();

			ImGui::Dummy(ImVec2(avail_width.x - 100.0f, avail_width.y));

			static const char* modes[] = {"Menu", "Ocio"};

			ImGui::PushID(99);
			ImGui::SetNextItemWidth(100.0f);
			ImGui::Combo("", &this->m_BarMode, &modes[0], IM_ARRAYSIZE(modes));
			ImGui::PopID();
		}

		ImGui::EndMainMenuBar();
	}
}