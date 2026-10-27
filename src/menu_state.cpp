#include "menu_state.hpp"
#include "game_state.hpp"
#include "state_machine.hpp"

#include <SFML/Audio/Listener.hpp>
#include <imgui.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <iostream>
#include <memory>
#include <utility>

namespace horde
{
namespace
{
constexpr auto s_FrameLimitPresets = std::array<uint32_t, 5>{60, 120, 144, 240, 0};

void CenterTextInRect(sf::Text& text, const sf::FloatRect& rect)
{
    const auto bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f});
    text.setPosition({rect.position.x + rect.size.x / 2.0f, rect.position.y + rect.size.y / 2.0f});
}

void LeftAlignTextInRow(sf::Text& text, float x, float centerY)
{
    const auto bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x, bounds.position.y + bounds.size.y / 2.0f});
    text.setPosition({x, centerY});
}

auto FormatWindowMode(WindowMode mode) -> std::string
{
    switch (mode)
    {
    case WindowMode::Windowed:
        return "Windowed";
    case WindowMode::Borderless:
        return "Borderless";
    case WindowMode::Fullscreen:
        return "Fullscreen";
    }
    return "Windowed";
}

auto FormatFrameLimit(uint32_t limit) -> std::string
{
    if (limit == 0)
    {
        return "Unlimited";
    }
    return std::format("{} FPS", limit);
}
} // namespace

MenuState::MenuState(
    StateMachine& stateMachine,
    sf::RenderWindow& window,
    GameSettings& settings,
    ApplySettingsCallback onApplySettings)
    : m_StateMachine(stateMachine)
    , m_Window(window)
    , m_Settings(settings)
    , m_PendingSettings(settings)
    , m_OnApplySettings(std::move(onApplySettings))
    , m_AvailableResolutions(GameSettings::GetAvailableResolutions())
{
    m_HasFont = m_Font.openFromFile(RESOURCES_PATH "fonts/font.ttf");
    if (!m_HasFont)
    {
        std::cerr << "Warning: MenuState could not load font from " << RESOURCES_PATH "fonts/font.ttf\n";
    }

    RebuildLayout();
}

MenuState::MenuState(StateMachine& stateMachine, sf::RenderWindow& window)
    : m_StateMachine(stateMachine)
    , m_Window(window)
    , m_FallbackSettings(SettingsSerializer::LoadFromFile(GameSettings::GetDefaultPath()))
    , m_Settings(m_FallbackSettings)
    , m_PendingSettings(m_FallbackSettings)
    , m_AvailableResolutions(GameSettings::GetAvailableResolutions())
{
    m_HasFont = m_Font.openFromFile(RESOURCES_PATH "fonts/font.ttf");
    if (!m_HasFont)
    {
        std::cerr << "Warning: MenuState could not load font from " << RESOURCES_PATH "fonts/font.ttf\n";
    }

    RebuildLayout();
}

void MenuState::OnEnter()
{
}

void MenuState::OnExit()
{
}

void MenuState::RebuildLayout()
{
    const auto windowSize = sf::Vector2f{m_Window.getSize()};
    const auto centerX = windowSize.x / 2.0f;
    const auto centerY = windowSize.y / 2.0f;

    // 1. Main Menu Layout
    if (m_HasFont)
    {
        m_TitleText.emplace(m_Font, "HORDE SURVIVOR", 56);
        m_TitleText->setFillColor(sf::Color{235, 240, 250});
        m_TitleText->setOutlineThickness(3.0f);
        m_TitleText->setOutlineColor(sf::Color{20, 25, 35});
        const auto titleBounds = m_TitleText->getLocalBounds();
        m_TitleText->setOrigin({titleBounds.position.x + titleBounds.size.x / 2.0f,
                                titleBounds.position.y + titleBounds.size.y / 2.0f});
        m_TitleText->setPosition({centerX, centerY - 170.0f});
    }

    m_MainButtons.clear();
    const auto mainBtnSize = sf::Vector2f{260.0f, 58.0f};
    const auto mainBtnSpacing = 76.0f;
    const auto startY = centerY - 40.0f;

    const auto makeMainButton = [&](const std::string& text,
                                    float yPos,
                                    sf::Color baseCol,
                                    sf::Color hoverCol,
                                    std::function<void()> callback)
    {
        auto btn = Button{};
        btn.baseColor = baseCol;
        btn.hoverColor = hoverCol;
        btn.onClick = std::move(callback);
        btn.shape.setSize(mainBtnSize);
        btn.shape.setOrigin(mainBtnSize / 2.0f);
        btn.shape.setPosition({centerX, yPos});
        btn.shape.setFillColor(baseCol);
        btn.shape.setOutlineThickness(2.5f);
        btn.shape.setOutlineColor(sf::Color{220, 225, 235});

        if (m_HasFont)
        {
            btn.label.emplace(m_Font, text, 26);
            btn.label->setFillColor(sf::Color::White);
            CenterTextInRect(*btn.label, btn.shape.getGlobalBounds());
        }

        m_MainButtons.push_back(std::move(btn));
    };

    makeMainButton(
        "Play",
        startY,
        sf::Color{55, 120, 175},
        sf::Color{80, 150, 210},
        [this]() { StartGame(); });

    makeMainButton(
        "Settings",
        startY + mainBtnSpacing,
        sf::Color{65, 80, 105},
        sf::Color{90, 110, 140},
        [this]() { OpenSettings(); });

    makeMainButton(
        "Exit",
        startY + mainBtnSpacing * 2.0f,
        sf::Color{140, 45, 50},
        sf::Color{175, 65, 70},
        [this]() { m_Window.close(); });

    // 2. Settings Panel Layout
    const auto panelSize = sf::Vector2f{std::min(740.0f, windowSize.x - 40.0f), std::min(640.0f, windowSize.y - 40.0f)};
    m_SettingsPanel.setSize(panelSize);
    m_SettingsPanel.setOrigin(panelSize / 2.0f);
    m_SettingsPanel.setPosition({centerX, centerY});
    m_SettingsPanel.setFillColor(sf::Color{24, 26, 34, 245});
    m_SettingsPanel.setOutlineThickness(2.5f);
    m_SettingsPanel.setOutlineColor(sf::Color{90, 115, 145});

    const auto panelTop = centerY - panelSize.y / 2.0f;
    const auto labelX = centerX - panelSize.x / 2.0f + 44.0f;
    const auto controlCenterX = centerX + panelSize.x / 2.0f - 195.0f;

    if (m_HasFont)
    {
        m_SettingsHeader.emplace(m_Font, "SETTINGS", 36);
        m_SettingsHeader->setFillColor(sf::Color{235, 240, 250});
        const auto hb = m_SettingsHeader->getLocalBounds();
        m_SettingsHeader->setOrigin({hb.position.x + hb.size.x / 2.0f, hb.position.y + hb.size.y / 2.0f});
        m_SettingsHeader->setPosition({centerX, panelTop + 42.0f});
    }

    const auto setupCycleRow = [&](CycleRow& row,
                                   const std::string& title,
                                   float rowY,
                                   std::function<void()> onPrev,
                                   std::function<void()> onNext)
    {
        if (m_HasFont)
        {
            row.label.emplace(m_Font, title, 22);
            row.label->setFillColor(sf::Color{215, 220, 230});
            LeftAlignTextInRow(*row.label, labelX, rowY);

            row.valueText.emplace(m_Font, "", 22);
            row.valueText->setFillColor(sf::Color::White);
        }

        const auto arrowSize = sf::Vector2f{38.0f, 36.0f};
        const auto arrowSpan = 135.0f;

        row.prevButton.baseColor = sf::Color{55, 65, 85};
        row.prevButton.hoverColor = sf::Color{85, 105, 135};
        row.prevButton.onClick = std::move(onPrev);
        row.prevButton.shape.setSize(arrowSize);
        row.prevButton.shape.setOrigin(arrowSize / 2.0f);
        row.prevButton.shape.setPosition({controlCenterX - arrowSpan, rowY});
        row.prevButton.shape.setFillColor(row.prevButton.baseColor);
        row.prevButton.shape.setOutlineThickness(1.5f);
        row.prevButton.shape.setOutlineColor(sf::Color{140, 155, 180});

        row.nextButton.baseColor = sf::Color{55, 65, 85};
        row.nextButton.hoverColor = sf::Color{85, 105, 135};
        row.nextButton.onClick = std::move(onNext);
        row.nextButton.shape.setSize(arrowSize);
        row.nextButton.shape.setOrigin(arrowSize / 2.0f);
        row.nextButton.shape.setPosition({controlCenterX + arrowSpan, rowY});
        row.nextButton.shape.setFillColor(row.nextButton.baseColor);
        row.nextButton.shape.setOutlineThickness(1.5f);
        row.nextButton.shape.setOutlineColor(sf::Color{140, 155, 180});

        if (m_HasFont)
        {
            row.prevButton.label.emplace(m_Font, "<", 22);
            row.prevButton.label->setFillColor(sf::Color::White);
            CenterTextInRect(*row.prevButton.label, row.prevButton.shape.getGlobalBounds());

            row.nextButton.label.emplace(m_Font, ">", 22);
            row.nextButton.label->setFillColor(sf::Color::White);
            CenterTextInRect(*row.nextButton.label, row.nextButton.shape.getGlobalBounds());
        }
    };

    const auto setupSliderRow = [&](SliderRow& row,
                                    const std::string& title,
                                    float rowY,
                                    float initialValue,
                                    std::function<void(float)> onChanged)
    {
        row.value = initialValue;
        row.onValueChanged = std::move(onChanged);

        if (m_HasFont)
        {
            row.label.emplace(m_Font, title, 22);
            row.label->setFillColor(sf::Color{215, 220, 230});
            LeftAlignTextInRow(*row.label, labelX, rowY);

            row.valueText.emplace(m_Font, "", 20);
            row.valueText->setFillColor(sf::Color{200, 215, 235});
        }

        const auto trackWidth = 220.0f;
        const auto trackHeight = 10.0f;
        const auto trackLeft = controlCenterX - 135.0f;

        row.track.setSize({trackWidth, trackHeight});
        row.track.setOrigin({0.0f, trackHeight / 2.0f});
        row.track.setPosition({trackLeft, rowY});
        row.track.setFillColor(sf::Color{45, 50, 65});
        row.track.setOutlineThickness(1.5f);
        row.track.setOutlineColor(sf::Color{100, 115, 140});

        row.fill.setSize({trackWidth * (row.value / 100.0f), trackHeight});
        row.fill.setOrigin({0.0f, trackHeight / 2.0f});
        row.fill.setPosition({trackLeft, rowY});
        row.fill.setFillColor(sf::Color{70, 145, 210});

        constexpr auto knobRadius = 10.0f;
        row.knob.setRadius(knobRadius);
        row.knob.setOrigin({knobRadius, knobRadius});
        row.knob.setPosition({trackLeft + trackWidth * (row.value / 100.0f), rowY});
        row.knob.setFillColor(sf::Color{235, 240, 250});
        row.knob.setOutlineThickness(2.0f);
        row.knob.setOutlineColor(sf::Color{40, 90, 140});
    };

    const auto firstRowY = panelTop + 105.0f;
    const auto rowStep = 55.0f;

    setupCycleRow(
        m_WindowModeRow,
        "Window Mode",
        firstRowY + rowStep * 0.0f,
        [this]()
        {
            const auto current = static_cast<int>(m_PendingSettings.windowMode);
            m_PendingSettings.windowMode = static_cast<WindowMode>((current + 2) % 3);
            RefreshSettingsValues();
        },
        [this]()
        {
            const auto current = static_cast<int>(m_PendingSettings.windowMode);
            m_PendingSettings.windowMode = static_cast<WindowMode>((current + 1) % 3);
            RefreshSettingsValues();
        });

    setupCycleRow(
        m_ResolutionRow,
        "Resolution",
        firstRowY + rowStep * 1.0f,
        [this]()
        {
            if (m_AvailableResolutions.empty())
            {
                return;
            }
            m_ResolutionIndex =
                (m_ResolutionIndex + m_AvailableResolutions.size() - 1) % m_AvailableResolutions.size();
            m_PendingSettings.resolution = m_AvailableResolutions[m_ResolutionIndex];
            RefreshSettingsValues();
        },
        [this]()
        {
            if (m_AvailableResolutions.empty())
            {
                return;
            }
            m_ResolutionIndex = (m_ResolutionIndex + 1) % m_AvailableResolutions.size();
            m_PendingSettings.resolution = m_AvailableResolutions[m_ResolutionIndex];
            RefreshSettingsValues();
        });

    setupCycleRow(
        m_VsyncRow,
        "VSync",
        firstRowY + rowStep * 2.0f,
        [this]()
        {
            m_PendingSettings.vsync = !m_PendingSettings.vsync;
            RefreshSettingsValues();
        },
        [this]()
        {
            m_PendingSettings.vsync = !m_PendingSettings.vsync;
            RefreshSettingsValues();
        });

    setupCycleRow(
        m_FrameLimitRow,
        "Frame Limit",
        firstRowY + rowStep * 3.0f,
        [this]()
        {
            if (m_PendingSettings.vsync)
            {
                return;
            }
            m_FrameLimitIndex =
                (m_FrameLimitIndex + s_FrameLimitPresets.size() - 1) % s_FrameLimitPresets.size();
            m_PendingSettings.frameRateLimit = s_FrameLimitPresets[m_FrameLimitIndex];
            RefreshSettingsValues();
        },
        [this]()
        {
            if (m_PendingSettings.vsync)
            {
                return;
            }
            m_FrameLimitIndex = (m_FrameLimitIndex + 1) % s_FrameLimitPresets.size();
            m_PendingSettings.frameRateLimit = s_FrameLimitPresets[m_FrameLimitIndex];
            RefreshSettingsValues();
        });

    setupSliderRow(
        m_MasterVolumeRow,
        "Master Volume",
        firstRowY + rowStep * 4.0f,
        m_PendingSettings.masterVolume,
        [this](float val)
        {
            m_PendingSettings.masterVolume = val;
            sf::Listener::setGlobalVolume(val);
        });

    setupSliderRow(
        m_MusicVolumeRow,
        "Music Volume",
        firstRowY + rowStep * 5.0f,
        m_PendingSettings.musicVolume,
        [this](float val) { m_PendingSettings.musicVolume = val; });

    setupSliderRow(
        m_SfxVolumeRow,
        "SFX Volume",
        firstRowY + rowStep * 6.0f,
        m_PendingSettings.sfxVolume,
        [this](float val) { m_PendingSettings.sfxVolume = val; });

    setupCycleRow(
        m_StatsOverlayRow,
        "Show FPS / Stats HUD",
        firstRowY + rowStep * 7.0f,
        [this]()
        {
            m_PendingSettings.showStatsOverlay = !m_PendingSettings.showStatsOverlay;
            RefreshSettingsValues();
        },
        [this]()
        {
            m_PendingSettings.showStatsOverlay = !m_PendingSettings.showStatsOverlay;
            RefreshSettingsValues();
        });

    // Bottom Apply & Back Buttons
    const auto bottomY = panelTop + panelSize.y - 48.0f;
    const auto actionBtnSize = sf::Vector2f{180.0f, 46.0f};

    m_ApplyButton.baseColor = sf::Color{45, 130, 85};
    m_ApplyButton.hoverColor = sf::Color{65, 165, 110};
    m_ApplyButton.onClick = [this]() { ApplyPendingSettings(); };
    m_ApplyButton.shape.setSize(actionBtnSize);
    m_ApplyButton.shape.setOrigin(actionBtnSize / 2.0f);
    m_ApplyButton.shape.setPosition({centerX - 110.0f, bottomY});
    m_ApplyButton.shape.setFillColor(m_ApplyButton.baseColor);
    m_ApplyButton.shape.setOutlineThickness(2.0f);
    m_ApplyButton.shape.setOutlineColor(sf::Color{210, 230, 220});

    m_BackButton.baseColor = sf::Color{90, 60, 65};
    m_BackButton.hoverColor = sf::Color{125, 80, 85};
    m_BackButton.onClick = [this]() { CloseSettings(); };
    m_BackButton.shape.setSize(actionBtnSize);
    m_BackButton.shape.setOrigin(actionBtnSize / 2.0f);
    m_BackButton.shape.setPosition({centerX + 110.0f, bottomY});
    m_BackButton.shape.setFillColor(m_BackButton.baseColor);
    m_BackButton.shape.setOutlineThickness(2.0f);
    m_BackButton.shape.setOutlineColor(sf::Color{220, 210, 215});

    if (m_HasFont)
    {
        m_ApplyButton.label.emplace(m_Font, "Apply", 23);
        m_ApplyButton.label->setFillColor(sf::Color::White);
        CenterTextInRect(*m_ApplyButton.label, m_ApplyButton.shape.getGlobalBounds());

        m_BackButton.label.emplace(m_Font, "Back", 23);
        m_BackButton.label->setFillColor(sf::Color::White);
        CenterTextInRect(*m_BackButton.label, m_BackButton.shape.getGlobalBounds());
    }

    RefreshSettingsValues();
}

void MenuState::RefreshSettingsValues()
{
    // Sync indices
    m_ResolutionIndex = 0;
    for (size_t i = 0; i < m_AvailableResolutions.size(); ++i)
    {
        if (m_AvailableResolutions[i] == m_PendingSettings.resolution)
        {
            m_ResolutionIndex = i;
            break;
        }
    }

    m_FrameLimitIndex = 0;
    for (size_t i = 0; i < s_FrameLimitPresets.size(); ++i)
    {
        if (s_FrameLimitPresets[i] == m_PendingSettings.frameRateLimit)
        {
            m_FrameLimitIndex = i;
            break;
        }
    }

    m_FrameLimitRow.enabled = !m_PendingSettings.vsync;

    const auto setCycleValueText = [](CycleRow& row, const std::string& text)
    {
        if (!row.valueText)
        {
            return;
        }
        row.valueText->setString(text);
        row.valueText->setFillColor(row.enabled ? sf::Color::White : sf::Color{120, 125, 135});
        const auto prevPos = row.prevButton.shape.getPosition();
        const auto nextPos = row.nextButton.shape.getPosition();
        const auto midX = (prevPos.x + nextPos.x) / 2.0f;
        const auto midY = prevPos.y;
        const auto bounds = row.valueText->getLocalBounds();
        row.valueText->setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                                  bounds.position.y + bounds.size.y / 2.0f});
        row.valueText->setPosition({midX, midY});
    };

    setCycleValueText(m_WindowModeRow, FormatWindowMode(m_PendingSettings.windowMode));
    setCycleValueText(
        m_ResolutionRow,
        std::format("{} x {}", m_PendingSettings.resolution.width, m_PendingSettings.resolution.height));
    setCycleValueText(m_VsyncRow, m_PendingSettings.vsync ? "On" : "Off");
    setCycleValueText(
        m_FrameLimitRow,
        m_PendingSettings.vsync ? "VSync Locked" : FormatFrameLimit(m_PendingSettings.frameRateLimit));
    setCycleValueText(m_StatsOverlayRow, m_PendingSettings.showStatsOverlay ? "On" : "Off");

    const auto updateSliderVisuals = [](SliderRow& slider, float val)
    {
        slider.value = std::clamp(val, 0.0f, 100.0f);
        const auto trackPos = slider.track.getPosition();
        const auto trackSize = slider.track.getSize();
        const auto ratio = slider.value / 100.0f;

        slider.fill.setSize({trackSize.x * ratio, trackSize.y});
        slider.knob.setPosition({trackPos.x + trackSize.x * ratio, trackPos.y});

        if (slider.valueText)
        {
            slider.valueText->setString(std::format("{:.0f}%", slider.value));
            LeftAlignTextInRow(*slider.valueText, trackPos.x + trackSize.x + 22.0f, trackPos.y);
        }
    };

    updateSliderVisuals(m_MasterVolumeRow, m_PendingSettings.masterVolume);
    updateSliderVisuals(m_MusicVolumeRow, m_PendingSettings.musicVolume);
    updateSliderVisuals(m_SfxVolumeRow, m_PendingSettings.sfxVolume);
}

void MenuState::UpdateSliderFromMouse(SliderRow& slider, float mouseX)
{
    const auto trackPos = slider.track.getPosition();
    const auto trackWidth = slider.track.getSize().x;
    if (trackWidth <= 0.0f)
    {
        return;
    }

    const auto ratio = std::clamp((mouseX - trackPos.x) / trackWidth, 0.0f, 1.0f);
    const auto newValue = std::round(ratio * 100.0f);
    slider.value = newValue;
    if (slider.onValueChanged)
    {
        slider.onValueChanged(newValue);
    }
    RefreshSettingsValues();
}

void MenuState::OnUpdate([[maybe_unused]] float dt)
{
    const bool imguiFocused =
        ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || ImGui::IsAnyItemActive();

    const auto mousePos = m_Window.mapPixelToCoords(sf::Mouse::getPosition(m_Window));

    const auto updateButtonHover = [&](Button& btn, bool enabled = true)
    {
        if (!enabled)
        {
            btn.shape.setFillColor(sf::Color{40, 42, 50});
            return;
        }
        if (!imguiFocused && btn.shape.getGlobalBounds().contains(mousePos))
        {
            btn.shape.setFillColor(btn.hoverColor);
        }
        else
        {
            btn.shape.setFillColor(btn.baseColor);
        }
    };

    if (m_SubMenu == SubMenu::Main)
    {
        for (auto& btn : m_MainButtons)
        {
            updateButtonHover(btn);
        }
    }
    else
    {
        for (auto* row : {&m_WindowModeRow, &m_ResolutionRow, &m_VsyncRow, &m_FrameLimitRow, &m_StatsOverlayRow})
        {
            updateButtonHover(row->prevButton, row->enabled);
            updateButtonHover(row->nextButton, row->enabled);
        }

        for (auto* slider : {&m_MasterVolumeRow, &m_MusicVolumeRow, &m_SfxVolumeRow})
        {
            if (slider->dragging)
            {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !imguiFocused)
                {
                    UpdateSliderFromMouse(*slider, mousePos.x);
                }
                else
                {
                    slider->dragging = false;
                }
            }
        }

        updateButtonHover(m_ApplyButton);
        updateButtonHover(m_BackButton);
    }
}

void MenuState::OnRender(sf::RenderWindow& window)
{
    window.setView(window.getDefaultView());
    window.clear(sf::Color{22, 24, 30});

    const auto drawButton = [&](const Button& btn)
    {
        window.draw(btn.shape);
        if (btn.label)
        {
            window.draw(*btn.label);
        }
    };

    if (m_SubMenu == SubMenu::Main)
    {
        if (m_TitleText)
        {
            window.draw(*m_TitleText);
        }

        for (const auto& btn : m_MainButtons)
        {
            drawButton(btn);
        }
    }
    else
    {
        window.draw(m_SettingsPanel);
        if (m_SettingsHeader)
        {
            window.draw(*m_SettingsHeader);
        }

        for (const auto* row : {&m_WindowModeRow, &m_ResolutionRow, &m_VsyncRow, &m_FrameLimitRow, &m_StatsOverlayRow})
        {
            if (row->label)
            {
                window.draw(*row->label);
            }
            drawButton(row->prevButton);
            drawButton(row->nextButton);
            if (row->valueText)
            {
                window.draw(*row->valueText);
            }
        }

        for (const auto* slider : {&m_MasterVolumeRow, &m_MusicVolumeRow, &m_SfxVolumeRow})
        {
            if (slider->label)
            {
                window.draw(*slider->label);
            }
            window.draw(slider->track);
            window.draw(slider->fill);
            window.draw(slider->knob);
            if (slider->valueText)
            {
                window.draw(*slider->valueText);
            }
        }

        drawButton(m_ApplyButton);
        drawButton(m_BackButton);
    }
}

void MenuState::OnEvent(const sf::Event& event)
{
    if (const auto* resized = event.getIf<sf::Event::Resized>())
    {
        m_Window.setView(sf::View{sf::FloatRect{{0.0f, 0.0f}, sf::Vector2f{resized->size}}});
        RebuildLayout();
        return;
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (m_SubMenu == SubMenu::Main)
        {
            if (keyPressed->code == sf::Keyboard::Key::Space || keyPressed->code == sf::Keyboard::Key::Enter)
            {
                StartGame();
            }
            else if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                m_Window.close();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            CloseSettings();
        }
        else if (keyPressed->code == sf::Keyboard::Key::Enter)
        {
            ApplyPendingSettings();
        }
    }
    else if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mousePressed->button != sf::Mouse::Button::Left)
        {
            return;
        }

        const auto clickPos = m_Window.mapPixelToCoords(mousePressed->position);

        if (m_SubMenu == SubMenu::Main)
        {
            for (const auto& btn : m_MainButtons)
            {
                if (btn.shape.getGlobalBounds().contains(clickPos) && btn.onClick)
                {
                    btn.onClick();
                    return;
                }
            }
        }
        else
        {
            for (auto* row : {&m_WindowModeRow, &m_ResolutionRow, &m_VsyncRow, &m_FrameLimitRow, &m_StatsOverlayRow})
            {
                if (!row->enabled)
                {
                    continue;
                }
                if (row->prevButton.shape.getGlobalBounds().contains(clickPos) && row->prevButton.onClick)
                {
                    row->prevButton.onClick();
                    return;
                }
                if (row->nextButton.shape.getGlobalBounds().contains(clickPos) && row->nextButton.onClick)
                {
                    row->nextButton.onClick();
                    return;
                }
            }

            for (auto* slider : {&m_MasterVolumeRow, &m_MusicVolumeRow, &m_SfxVolumeRow})
            {
                auto hitBox = slider->track.getGlobalBounds();
                hitBox.position.y -= 10.0f;
                hitBox.size.y += 20.0f;
                if (hitBox.contains(clickPos) || slider->knob.getGlobalBounds().contains(clickPos))
                {
                    slider->dragging = true;
                    UpdateSliderFromMouse(*slider, clickPos.x);
                    return;
                }
            }

            if (m_ApplyButton.shape.getGlobalBounds().contains(clickPos) && m_ApplyButton.onClick)
            {
                m_ApplyButton.onClick();
                return;
            }

            if (m_BackButton.shape.getGlobalBounds().contains(clickPos) && m_BackButton.onClick)
            {
                m_BackButton.onClick();
                return;
            }
        }
    }
    else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>())
    {
        if (mouseReleased->button == sf::Mouse::Button::Left)
        {
            for (auto* slider : {&m_MasterVolumeRow, &m_MusicVolumeRow, &m_SfxVolumeRow})
            {
                slider->dragging = false;
            }
        }
    }
}

void MenuState::StartGame()
{
    m_StateMachine.ChangeState(std::make_unique<GameState>(m_StateMachine, m_Window));
}

void MenuState::OpenSettings()
{
    m_PendingSettings = m_Settings;
    RefreshSettingsValues();
    m_SubMenu = SubMenu::Settings;
}

void MenuState::CloseSettings()
{
    sf::Listener::setGlobalVolume(m_Settings.masterVolume);
    m_PendingSettings = m_Settings;
    RefreshSettingsValues();
    m_SubMenu = SubMenu::Main;
}

void MenuState::ApplyPendingSettings()
{
    m_Settings = m_PendingSettings;
    if (m_OnApplySettings)
    {
        m_OnApplySettings(m_Settings);
    }
    else
    {
        (void)SettingsSerializer::SaveToFile(m_Settings, GameSettings::GetDefaultPath());
    }
    RebuildLayout();
}
} // namespace horde
