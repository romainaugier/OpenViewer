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
                case GLFW_KEY_E:
                {
                    this->ShowMediaExplorerWindow();
                    break;
                }

                case GLFW_KEY_O:
                {
                    if (this->m_Shortcuts.m_Pressed[GLFW_KEY_LEFT_CONTROL] ||
                        this->m_Shortcuts.m_Pressed[GLFW_KEY_RIGHT_CONTROL])
                    {
                        printf("Ctrl + O\n");
                        break;
                    }
                    else
                    {
                        
                    }
                }

                case GLFW_KEY_SPACE:
                {
                    if (this->m_Playbar->IsPlaying()) this->m_Playbar->Pause();
                    else this->m_Playbar->Play();
                    break;
                }

                // Arrows

                case GLFW_KEY_UP:
                {
                    const uint8_t modeCount = this->m_Menubar->GetModeCount();
                    const uint8_t currentMode = this->m_Menubar->GetMode();
                    const uint8_t newMode = currentMode == 0 ? (modeCount - 1) : (currentMode - 1); 

                    this->m_Menubar->SetMode(newMode);
                    break;
                }

                case GLFW_KEY_DOWN:
                {
                    const uint8_t modeCount = this->m_Menubar->GetModeCount();
                    const uint8_t currentMode = this->m_Menubar->GetMode();
                    const uint8_t newMode = currentMode == (modeCount - 1) ? 0 : currentMode + 1; 
                    
                    this->m_Menubar->SetMode(newMode);
                    break;
                }

                case GLFW_KEY_LEFT:
                {
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
                    if (this->m_Shortcuts.m_Pressed[GLFW_KEY_LEFT_CONTROL] ||
                        this->m_Shortcuts.m_Pressed[GLFW_KEY_RIGHT_CONTROL])
                    {
                        this->m_Playbar->GoLastFrame();
                    }
                    else this->m_Playbar->GoNextFrame();

                    break;
                }
            }

            glfwWaitEventsTimeout(0.25);
        }
    }

    void Application::Release() noexcept
    {
        for (auto& [id, display] : this->m_Displays)
        {
            display.second->Release();
        }

        this->m_Logger->Log(LogLevel_Diagnostic, "[MAIN] : Released OpenViewer");
    }
}