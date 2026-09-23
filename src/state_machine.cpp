#include "state_machine.hpp"

#include <utility>

void StateMachine::ChangeState(std::unique_ptr<State> newState)
{
    m_NextState = std::move(newState);
    m_IsChanging = true;
}

void StateMachine::ProcessStateChange()
{
    if (m_IsChanging && m_NextState)
    {
        if (m_ActiveState)
        {
            m_ActiveState->OnExit();
        }
        m_ActiveState = std::move(m_NextState);
        m_ActiveState->OnEnter();
        m_IsChanging = false;
    }
}

auto StateMachine::HasActiveState() const -> bool
{
    return m_ActiveState != nullptr;
}

auto StateMachine::GetActiveState() -> State*
{
    return m_ActiveState.get();
}

auto StateMachine::GetActiveState() const -> const State*
{
    return m_ActiveState.get();
}
