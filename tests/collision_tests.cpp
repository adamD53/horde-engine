#include "collision_grid.hpp"
#include "terrain.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <sstream>

void Expect(float actual, float expected, const char* message)
{
    if (std::abs(actual - expected) > 0.001f) throw std::runtime_error(message);
}

int main()
{
    try
    {
        CollisionGrid grid;
        grid.Reset(8, 8, {16.f, 16.f});
        grid.Add(2, 2, {{0.f, 0.f}, {16.f, 16.f}});
        Expect(grid.Move({{8.f, 8.f}, {8.f, 8.f}}, {12.f, 0.f}).x, 12.f, "Grass blocks movement");
        Expect(grid.Move({{8.f, 34.f}, {8.f, 8.f}}, {100.f, 0.f}).x, 16.f, "Fast movement tunnels through wall");
        Expect(grid.Move({{64.f, 34.f}, {8.f, 8.f}}, {-100.f, 0.f}).x, -16.f, "Left sweep fails");
        Expect(grid.Move({{34.f, 8.f}, {8.f, 8.f}}, {0.f, 100.f}).y, 16.f, "Down sweep fails");
        Expect(grid.Move({{34.f, 64.f}, {8.f, 8.f}}, {0.f, -100.f}).y, -16.f, "Up sweep fails");
        auto slide = grid.Move({{24.f, 34.f}, {8.f, 8.f}}, {10.f, 5.f});
        Expect(slide.x, 0.f, "Wall contact is not blocked");
        Expect(slide.y, 5.f, "Cannot slide along wall");
        Expect(grid.Move({{8.f, 8.f}, {8.f, 8.f}}, {-100.f, 0.f}).x, -8.f, "World boundary fails");
        grid.Reset(8, 8, {16.f, 16.f});
        grid.Add(2, 4, {{2.f, 2.f}, {10.f, 12.f}});
        Expect(grid.Move({{34.f, 32.f}, {4.f, 4.f}}, {8.f, 0.f}).x, 8.f, "Tree canopy blocks movement");
        Expect(grid.Move({{36.f, 48.f}, {4.f, 4.f}}, {0.f, 80.f}).y, 14.f, "Tree trunk does not block movement");

        // Exercise the actual TMX metadata and Terrain integration, not just geometry.
        std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures;
        for (const char* name : {"Water", "Floors_Tiles", "Trees", "Props", "Roofs", "Walls"})
            textures.emplace(name, std::make_unique<sf::Texture>());
        Terrain terrain;
        terrain.Load(TEST_MAP_PATH, textures);
        Expect(terrain.ResolveMovement({{531.f, 294.f}, {10.f, 6.f}}, {0.f, 4.f}).y, 4.f, "Spawn is blocked");
        Expect(terrain.ResolveMovement({{531.f, 294.f}, {10.f, 6.f}}, {-100.f, 0.f}).x, -3.f, "Map water is not solid");
        terrain.Load(TEST_MAP_PATH, textures);
        Expect(terrain.ResolveMovement({{531.f, 294.f}, {10.f, 6.f}}, {-100.f, 0.f}).x, -3.f, "Reload changes collision");
        tinyxml2::XMLDocument map;
        if (map.LoadFile(TEST_MAP_PATH) != tinyxml2::XML_SUCCESS) throw std::runtime_error("Cannot inspect test map");
        auto tilePosition = [&](unsigned wanted)
        {
            for (auto* layer = map.RootElement()->FirstChildElement("layer"); layer; layer = layer->NextSiblingElement("layer"))
            {
                std::istringstream csv(layer->FirstChildElement("data")->GetText());
                std::string value;
                unsigned index = 0;
                while (std::getline(csv, value, ','))
                {
                    if (std::stoul(value) == wanted) return sf::Vector2f((index % 80) * 16.f, (index / 80) * 16.f);
                    ++index;
                }
            }
            throw std::runtime_error("Missing representative test tile");
        };
        for (unsigned gid : {6u, 288u, 871u, 1396u, 2376u})
        {
            auto position = tilePosition(gid);
            Expect(terrain.ResolveMovement({position + sf::Vector2f(-2.f, 7.f), {2.f, 2.f}}, {10.f, 0.f}).x,
                   0.f, "Solid map tile can be entered");
        }
        auto canopy = tilePosition(681);
        Expect(terrain.ResolveMovement({canopy + sf::Vector2f(5.f, 5.f), {2.f, 2.f}}, {1.f, 0.f}).x,
               1.f, "Map tree canopy is blocked");
        auto trunk = tilePosition(718);
        Expect(terrain.ResolveMovement({trunk + sf::Vector2f(5.f, 0.f), {2.f, 2.f}}, {0.f, 10.f}).y,
               0.f, "Map tree base is not solid");
        std::cout << "Collision tests passed\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
