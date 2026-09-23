#pragma once

#include <SFML/Graphics.hpp>

#include "tinyxml2.h"

#include "entity.hpp"
#include "collision_grid.hpp"

#include <unordered_map>
#include <vector>

namespace horde
{
struct Tileset
{
    uint32_t firstGid;
    uint32_t lastGid;
    const sf::Texture* tex;
    int tileW;
    int tileH;
    int columns;
    std::vector<std::vector<sf::FloatRect>> collision;
    bool depthSorted = false;
    std::vector<float> depthOffsets;
};

struct Layer
{
    std::vector<sf::VertexArray> vertices;
    tinyxml2::XMLElement* data;
    std::string name;
};

struct DepthBatch
{
    sf::VertexArray vertices{sf::PrimitiveType::Triangles};
    const sf::Texture* texture = nullptr;
    float depth = 0.f;
    size_t layerIndex = 0;
};

struct DepthActor
{
    const sf::Drawable* drawable;
    float depth;
};

class Terrain : public sf::Drawable, sf::Transformable
{
public:
    Terrain() = default;
    ~Terrain() = default;

public:
    void Load(const std::string& tmx_file, std::unordered_map<std::string, std::unique_ptr<sf::Texture>>& textures);
    auto GetStaticEntities() const -> const std::vector<std::unique_ptr<Entity>>&;
    auto ResolveMovement(sf::FloatRect feet, sf::Vector2f movement) const -> sf::Vector2f;
    void DrawWithActor(sf::RenderTarget& target, const sf::Drawable& actor, float feetY) const;
    void DrawWithActors(sf::RenderTarget& target, std::vector<DepthActor> actors) const;
    auto GetSize() const -> sf::Vector2f;
    auto IsBlocked(sf::FloatRect body) const -> bool;
    auto FindPath(sf::Vector2f start, sf::Vector2f goal, sf::Vector2f bodySize) const -> std::vector<sf::Vector2f>;

private:
    std::vector<Tileset> m_Tilesets;
    std::vector<int> m_GidToTileset;
    std::vector<std::unique_ptr<Entity>> m_StaticEntities;
    tinyxml2::XMLDocument m_Document;
    std::vector<Layer> m_Layers;
    std::vector<DepthBatch> m_DepthBatches;

    uint32_t m_MapWidth = 0;
    uint32_t m_MapHeight = 0;
    sf::Vector2f m_TileSize;
    CollisionGrid m_Collision;

private:
    auto LoadXML(const std::string& tmx_file) -> tinyxml2::XMLElement*;
    void ParseTilesets(
        tinyxml2::XMLElement* map,
        std::unordered_map<std::string, std::unique_ptr<sf::Texture>>& textures);
    auto ParseCSV(tinyxml2::XMLElement* map) -> std::vector<uint32_t>;
    auto GetDataFromLayer(tinyxml2::XMLElement* map) -> tinyxml2::XMLElement*;
    void BuildGridMap();
    void BuildLayerVertices(tinyxml2::XMLElement* map);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
}
