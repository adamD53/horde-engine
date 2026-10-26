#include "debug_ui.hpp"

#include <imgui.h>

namespace horde
{
void DebugUI::Render()
{
    if (!m_Visible)
    {
        return;
    }

    if (ImGui::Begin("Debug UI", &m_Visible, ImGuiWindowFlags_NoFocusOnAppearing))
    {
        // Blank debug window
    }
    ImGui::End();
}

void DebugUI::Toggle()
{
    m_Visible = !m_Visible;
}

bool DebugUI::IsVisible() const
{
    return m_Visible;
}

void DebugUI::SetVisible(bool visible)
{
    m_Visible = visible;
}
}
