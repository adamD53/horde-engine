#pragma once

#include "state.hpp"

#include <memory>

namespace horde
{
class StateMachine
{
public:
    StateMachine() = default;
    ~StateMachine() = default;

    void ChangeState(std::unique_ptr<State> newState);
    void ProcessStateChange();

    [[nodiscard]] auto HasActiveState() const -> bool;
    [[nodiscard]] auto GetActiveState() -> State*;
    [[nodiscard]] auto GetActiveState() const -> const State*;

private:
    std::unique_ptr<State> m_ActiveState;
    std::unique_ptr<State> m_NextState;
    bool m_IsChanging{false};
};
}
