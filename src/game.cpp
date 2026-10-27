#include "game.hpp"
#include "menu_state.hpp"

#include <SFML/Audio/Listener.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>

namespace horde
{
Game::Game(const GameSpecification& spec)
    : m_Title(spec.title)
    , m_Settings(SettingsSerializer::LoadFromFile(GameSettings::GetDefaultPath()))
{
    ApplySettings(m_Settings, true);

    m_StateMachine.ChangeState(std::make_unique<MenuState>(
        m_StateMachine,
        m_Window,
        m_Settings,
        [this](const GameSettings& updated) { ApplySettings(updated); }));
    m_StateMachine.ProcessStateChange();
}

Game::~Game()
{
    if (m_ImGuiInitialized)
    {
        ImGui::SFML::Shutdown(m_Window);
        m_ImGuiInitialized = false;
    }
}

void Game::ApplySettings(const GameSettings& newSettings, bool forceRecreate)
{
    const auto modeChanged = (newSettings.windowMode != m_Settings.windowMode);
    const auto resChanged = (newSettings.resolution != m_Settings.resolution);
    const auto needsRecreate =
        forceRecreate || modeChanged || (resChanged && newSettings.windowMode != WindowMode::Windowed);

    m_Settings = newSettings;

    if (needsRecreate)
    {
        if (m_ImGuiInitialized)
        {
            ImGui::SFML::Shutdown(m_Window);
            m_ImGuiInitialized = false;
        }

        const auto desktopMode = sf::VideoMode::getDesktopMode();
        auto targetMode = sf::VideoMode{sf::Vector2u{m_Settings.resolution.width, m_Settings.resolution.height}};

        if (m_Settings.windowMode == WindowMode::Fullscreen)
        {
            if (!targetMode.isValid())
            {
                targetMode = desktopMode;
            }
            m_Window.create(targetMode, m_Title, sf::State::Fullscreen);
        }
        else if (m_Settings.windowMode == WindowMode::Borderless)
        {
            m_Window.create(targetMode, m_Title, sf::Style::None, sf::State::Windowed);
            const auto posX = static_cast<int>(desktopMode.size.x > targetMode.size.x
                                                   ? (desktopMode.size.x - targetMode.size.x) / 2
                                                   : 0);
            const auto posY = static_cast<int>(desktopMode.size.y > targetMode.size.y
                                                   ? (desktopMode.size.y - targetMode.size.y) / 2
                                                   : 0);
            m_Window.setPosition({posX, posY});
        }
        else
        {
            m_Window.create(targetMode, m_Title, sf::Style::Default, sf::State::Windowed);
        }

        if (!ImGui::SFML::Init(m_Window))
        {
            std::cerr << "Failed to initialize ImGui-SFML backend.\n";
        }
        else
        {
            m_ImGuiInitialized = true;
        }
    }
    else if (resChanged && m_Settings.windowMode == WindowMode::Windowed)
    {
        const auto newSize = sf::Vector2u{m_Settings.resolution.width, m_Settings.resolution.height};
        m_Window.setSize(newSize);
        m_Window.setView(sf::View{sf::FloatRect{{0.f, 0.f}, sf::Vector2f{newSize}}});
    }

    m_Window.setVerticalSyncEnabled(m_Settings.vsync);
    m_Window.setFramerateLimit(m_Settings.vsync ? 0u : m_Settings.frameRateLimit);
    sf::Listener::setGlobalVolume(std::clamp(m_Settings.masterVolume, 0.0f, 100.0f));

    if (auto* state = m_StateMachine.GetActiveState())
    {
        const auto currentSize = m_Window.getSize();
        state->OnEvent(sf::Event{sf::Event::Resized{currentSize}});
    }

    (void)SettingsSerializer::SaveToFile(m_Settings, GameSettings::GetDefaultPath());
}

auto Game::GetSettings() -> GameSettings&
{
    return m_Settings;
}

auto Game::GetSettings() const -> const GameSettings&
{
    return m_Settings;
}

void Game::Run()
{
    auto clock = sf::Clock{};

    while (m_Window.isOpen())
    {
        m_StateMachine.ProcessStateChange();

        while (const std::optional event = m_Window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(m_Window, *event);

            if (event->is<sf::Event::Closed>())
            {
                m_Window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::F1)
                {
                    m_DebugUI.Toggle();
                }
                else if (keyPressed->code == sf::Keyboard::Key::Escape &&
                         ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
                {
                    ImGui::SetWindowFocus(nullptr);
                    continue;
                }
            }

            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (!ImGui::GetIO().WantCaptureMouse)
                {
                    ImGui::SetWindowFocus(nullptr);
                }
            }

            const bool imguiFocused =
                ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || ImGui::IsAnyItemActive();

            if (imguiFocused)
            {
                const bool isInputEvent = event->is<sf::Event::MouseButtonPressed>() ||
                                          event->is<sf::Event::MouseButtonReleased>() ||
                                          event->is<sf::Event::MouseMoved>() ||
                                          event->is<sf::Event::MouseWheelScrolled>() ||
                                          event->is<sf::Event::KeyPressed>() ||
                                          event->is<sf::Event::KeyReleased>() ||
                                          event->is<sf::Event::TextEntered>();

                if (isInputEvent)
                {
                    continue;
                }
            }

            if (auto* state = m_StateMachine.GetActiveState())
            {
                state->OnEvent(*event);
            }
        }

        m_StateMachine.ProcessStateChange();

        const auto dt = std::min(clock.restart().asSeconds(), 0.1f);

        ImGui::SFML::Update(m_Window, sf::seconds(dt));

        if (auto* state = m_StateMachine.GetActiveState())
        {
            state->OnUpdate(dt);
        }

        if (auto* state = m_StateMachine.GetActiveState())
        {
            state->OnRender(m_Window);
            state->OnImGuiRender(m_DebugUI);
        }

        m_DebugUI.Render();

        ImGui::SFML::Render(m_Window);

        m_Window.display();
    }
}
}
