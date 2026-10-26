#pragma once

namespace horde
{
class DebugUI
{
public:
    DebugUI() = default;
    ~DebugUI() = default;

    void Render();
    void Toggle();
    [[nodiscard]] bool IsVisible() const;
    void SetVisible(bool visible);

private:
    bool m_Visible = false;
};
}
