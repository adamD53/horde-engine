#include "melee_combat_system.hpp"
#include "enemy.hpp"

#include <cmath>
#include <numbers>

namespace horde
{
MeleeCombatSystem::MeleeCombatSystem(float range, float sweepAngle, float attackDelay, float damage)
    : m_Range(range)
    , m_SweepAngle(sweepAngle)
    , m_AttackDelay(attackDelay)
    , m_Damage(damage)
    , m_Timer(attackDelay)
{
}

void MeleeCombatSystem::Update(float dt)
{
    m_Timer += dt;
}

auto MeleeCombatSystem::CanAttack() const -> bool
{
    return m_Timer >= m_AttackDelay;
}

void MeleeCombatSystem::Attack(const sf::Vector2f& attackerPos,
                              const sf::Vector2f& targetPos,
                              ObjectPool<EntityVariant>& enemyPool)
{
    if (!CanAttack()) return;

    m_Timer = 0.0f;

    const auto toTarget = targetPos - attackerPos;
    const auto aimAngle = std::atan2(toTarget.y, toTarget.x);
    const auto halfSweepRad = (m_SweepAngle * 0.5f) * degToRad;

    for (auto& enemyVariant : enemyPool.GetActive())
    {
        std::visit(
            [&](auto& enemy)
            {
                if (enemy.IsDead()) return;

                const auto bounds = enemy.GetBounds();
                const auto enemyCenter = sf::Vector2f{
                    bounds.position.x + bounds.size.x * 0.5f,
                    bounds.position.y + bounds.size.y * 0.5f
                };
                const auto toEnemy = enemyCenter - attackerPos;
                const auto dist = toEnemy.length();

                if (dist <= m_Range)
                {
                    const auto enemyAngle = std::atan2(toEnemy.y, toEnemy.x);
                    const auto angleDiff = std::remainder(enemyAngle - aimAngle, twoPi);
                    if (std::abs(angleDiff) <= halfSweepRad)
                    {
                        enemy.TakeDamage(m_Damage);
                    }
                }
            },
            enemyVariant);
    }
}

void MeleeCombatSystem::SetRange(float range)
{
    m_Range = range;
}

void MeleeCombatSystem::SetSweepAngle(float angle)
{
    m_SweepAngle = angle;
}

void MeleeCombatSystem::SetAttackDelay(float delay)
{
    m_AttackDelay = delay;
}

void MeleeCombatSystem::SetDamage(float damage)
{
    m_Damage = damage;
}

auto MeleeCombatSystem::GetRange() const -> float
{
    return m_Range;
}

auto MeleeCombatSystem::GetSweepAngle() const -> float
{
    return m_SweepAngle;
}

auto MeleeCombatSystem::GetAttackDelay() const -> float
{
    return m_AttackDelay;
}

auto MeleeCombatSystem::GetDamage() const -> float
{
    return m_Damage;
}
}
