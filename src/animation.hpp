#pragma once

#include <SFML/Graphics.hpp>
#include <array>

namespace horde
{
struct AnimationState
{
    uint32_t row;
    uint32_t startColumn;
    uint32_t frameCount;
    float frameDuration;
    bool loop;
};

template <typename Enum, size_t Count>
class Animator
{
public:
    void AddState(Enum stateId, AnimationState state)
    {
        m_States[static_cast<size_t>(stateId)] = state;
    }

    void Play(Enum stateId)
    {
        if (m_CurrentStateId != stateId)
        {
            m_CurrentStateId = stateId;
            m_CurrentFrame = 0;
            m_ElapsedTime = 0.0f;
        }
    }

    void Update(float dt)
    {
        const auto& state = m_States[static_cast<size_t>(m_CurrentStateId)];
        if (state.frameCount <= 1) return;

        m_ElapsedTime += dt;
        if (m_ElapsedTime >= state.frameDuration)
        {
            m_ElapsedTime -= state.frameDuration;
            m_CurrentFrame++;
            if (m_CurrentFrame >= state.frameCount)
            {
                m_CurrentFrame = state.loop ? 0 : state.frameCount - 1;
            }
        }
    }

    void ApplyToSprite(sf::Sprite& sprite, const sf::Vector2i& cellSize, const sf::Vector2i& spriteSize) const
    {
        const auto& state = m_States[static_cast<size_t>(m_CurrentStateId)];
        const int frameX = (state.startColumn + m_CurrentFrame) * cellSize.x;
        const int frameY = state.row * cellSize.y;

        const int offsetX = frameX + (cellSize.x - spriteSize.x) / 2;
        const int offsetY = frameY + (cellSize.y - spriteSize.y) / 2;

        sprite.setTextureRect(sf::IntRect({offsetX, offsetY}, spriteSize));
    }

private:
    std::array<AnimationState, Count> m_States{};
    Enum m_CurrentStateId{};
    float m_ElapsedTime = 0.0f;
    uint32_t m_CurrentFrame = 0;
};
}
