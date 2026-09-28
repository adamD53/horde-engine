#pragma once

#include "global_config.hpp"
#include "object_pool.hpp"

#include <SFML/System/Vector2.hpp>
#include <numbers>
#include <variant>

namespace horde
{
class Enemy;
using EntityVariant = std::variant<Enemy>;

class MeleeCombatSystem
{
public:
    static constexpr float degToRad = std::numbers::pi_v<float> / 180.0f;
    static constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;

    MeleeCombatSystem(float range = config::meleeAttackRange,
                      float sweepAngle = config::meleeSweepAngle,
                      float attackDelay = config::meleeAttackDelay,
                      float damage = config::meleeDamage);

    void Update(float dt);
    [[nodiscard]] auto CanAttack() const -> bool;
    void Attack(const sf::Vector2f& attackerPos,
                const sf::Vector2f& targetPos,
                ObjectPool<EntityVariant>& enemyPool);

    void SetRange(float range);
    void SetSweepAngle(float angle);
    void SetAttackDelay(float delay);
    void SetDamage(float damage);

    [[nodiscard]] auto GetRange() const -> float;
    [[nodiscard]] auto GetSweepAngle() const -> float;
    [[nodiscard]] auto GetAttackDelay() const -> float;
    [[nodiscard]] auto GetDamage() const -> float;

private:
    float m_Range;
    float m_SweepAngle;
    float m_AttackDelay;
    float m_Damage;
    float m_Timer{0.0f};
};
}
