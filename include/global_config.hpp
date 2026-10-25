#pragma once

#include <SFML/System/Vector2.hpp>
#include <cstdint>

namespace horde::config
{
inline constexpr uint32_t playerTextureHeight = 32;
inline constexpr uint32_t playerTextureWidth = 32;
inline constexpr sf::Vector2u singleTileSize{16, 16};
inline constexpr float cameraZoomLevel = 0.4f;
inline constexpr float cameraSmoothSpeed = 3.f;
inline constexpr float shootingDelay = 0.1f;
inline constexpr float projectileSpeed = 200.f;
inline constexpr sf::Vector2f playerInitialPosition{536.f, 300.f};
inline constexpr uint32_t initialEnemyCount = 100;
inline constexpr uint32_t maxEnemyCount = 10000;
inline constexpr float enemyAttackDamage = 10.0f;
inline constexpr float minSpawnDistance = 200.f;
inline constexpr float maxSpawnDistance = 500.f;
inline constexpr uint32_t maxProjectileCount = 200;
inline constexpr float projectileDamage = 50.f;
inline constexpr float meleeAttackRange = 200.f;
inline constexpr float meleeSweepAngle = 150.f;
inline constexpr float meleeAttackDelay = 0.1f;
inline constexpr float meleeDamage = 50.f;
}
