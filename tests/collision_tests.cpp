#include "collision_grid.hpp"
#include "terrain.hpp"
#include "tile_collision.hpp"
#include <filesystem>
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
        if (!textures.at("Roofs")->loadFromFile(std::filesystem::path(TEST_MAP_PATH).parent_path() / "Roofs.png"))
            throw std::runtime_error("Cannot load roof collision image");
        if (!textures.at("Props")->loadFromFile(std::filesystem::path(TEST_MAP_PATH).parent_path() / "Props.png"))
            throw std::runtime_error("Cannot load chimney collision image");
        if (!textures.at("Trees")->loadFromFile(std::filesystem::path(TEST_MAP_PATH).parent_path() / "Trees.png"))
            throw std::runtime_error("Cannot load tree image");
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
        for (const auto gid : {6u, 802u, 2376u})
        {
            auto position = tilePosition(gid);
            Expect(terrain.ResolveMovement({position + sf::Vector2f(-2.f, 7.f), {2.f, 2.f}}, {10.f, 0.f}).x,
                   0.f, "Solid map tile can be entered");
        }
        const auto floor = tilePosition(288);
        const auto roof = tilePosition(1396);
        Expect(terrain.ResolveMovement({roof + sf::Vector2f(5.f, 5.f), {2.f, 2.f}}, {1.f, 0.f}).x,
               1.f, "Transparent roof corner blocks movement");
        const auto roofImage = textures.at("Roofs")->copyToImage();
        const auto propsImage = textures.at("Props")->copyToImage();
        // Check every column across the sloping top, including the tile seams.
        for (auto x = 0; x < 128; ++x)
        {
            auto firstOpaque = 0;
            while (firstOpaque < 96 && roofImage.getPixel({static_cast<unsigned>(x), static_cast<unsigned>(firstOpaque)}).a == 0)
                ++firstOpaque;
            // Include the chimney artwork drawn over the roof in the map.
            if (x >= 16 && x < 48)
                for (auto y = 0; y < 48; ++y)
                    if (propsImage.getPixel({static_cast<unsigned>(x - 16), static_cast<unsigned>(64 + y)}).a != 0)
                    {
                        firstOpaque = std::min(firstOpaque, y);
                        break;
                    }
            const auto movement = terrain.ResolveMovement({roof + sf::Vector2f(static_cast<float>(x), -1.f), {1.f, 1.f}}, {0.f, 96.f});
            if (std::abs(movement.y - firstOpaque) > 0.001f)
                throw std::runtime_error("Roof column " + std::to_string(x) + ": expected " +
                                         std::to_string(firstOpaque) + ", got " + std::to_string(movement.y));
        }
        Expect(terrain.ResolveMovement({floor + sf::Vector2f(5.f, 5.f), {2.f, 2.f}}, {1.f, 0.f}).x,
               1.f, "Non-grass floor tile blocks movement");
        auto canopy = tilePosition(681);
        Expect(terrain.ResolveMovement({canopy + sf::Vector2f(5.f, 5.f), {2.f, 2.f}}, {1.f, 0.f}).x,
               1.f, "Map tree canopy is blocked");
        auto trunk = tilePosition(718);
        Expect(terrain.ResolveMovement({trunk + sf::Vector2f(5.f, 0.f), {2.f, 2.f}}, {0.f, 10.f}).y,
               0.f, "Map tree base is not solid");
        const auto treeImage = textures.at("Trees")->copyToImage();
        auto sample = sf::Vector2u{};
        auto found = false;
        for (auto y = 0u; y < 64 && !found; ++y)
            for (auto x = 0u; x < 48; ++x)
                if (treeImage.getPixel({x, y}).a == 255)
                {
                    sample = {x, y};
                    found = true;
                    break;
                }
        if (!found) throw std::runtime_error("Tree canopy has no opaque test pixel");
        auto target = sf::RenderTexture({48u, 80u});
        target.setView(sf::View(sf::FloatRect(canopy, {48.f, 80.f})));
        auto actor = sf::RectangleShape({48.f, 80.f});
        actor.setPosition(canopy);
        actor.setFillColor(sf::Color::Magenta);
        target.clear();
        terrain.DrawWithActor(target, actor, canopy.y - 1.f);
        target.display();
        if (target.getTexture().copyToImage().getPixel(sample) != treeImage.getPixel(sample))
            throw std::runtime_error("Tree does not hide an actor behind its base");
        target.clear();
        terrain.DrawWithActor(target, actor, canopy.y + 80.f);
        target.display();
        if (target.getTexture().copyToImage().getPixel(sample) != sf::Color::Magenta)
            throw std::runtime_error("Tree incorrectly hides an actor in front of its base");
        auto roofTarget = sf::RenderTexture({128u, 96u});
        roofTarget.setView(sf::View(sf::FloatRect(roof, {128.f, 96.f})));
        actor.setPosition(roof);
        actor.setSize({128.f, 96.f});
        roofTarget.clear();
        // The building base is eight rows below the roof top. Standing below
        // the six-row eave must not put the actor on top of the roof artwork.
        terrain.DrawWithActor(roofTarget, actor, roof.y + 127.f);
        roofTarget.display();
        const auto behindRoof = roofTarget.getTexture().copyToImage();
        roofTarget.clear();
        terrain.DrawWithActor(roofTarget, actor, roof.y + 128.f);
        roofTarget.display();
        const auto inFrontOfRoof = roofTarget.getTexture().copyToImage();
        for (const auto x : {2u, 125u})
        {
            auto edgeY = 95u;
            while (edgeY > 64 && roofImage.getPixel({x, edgeY}).a == 0) --edgeY;
            if (roofImage.getPixel({x, edgeY}).a != 255)
                throw std::runtime_error("Roof eave test pixel is not opaque");
            if (behindRoof.getPixel({x, edgeY}) != roofImage.getPixel({x, edgeY}))
                throw std::runtime_error("Roof edge does not hide an actor behind it");
            if (inFrontOfRoof.getPixel({x, edgeY}) != sf::Color::Magenta)
                throw std::runtime_error("Roof edge hides an actor in front of it");
        }
        if (behindRoof.getPixel({0u, 0u}) != sf::Color::Magenta)
            throw std::runtime_error("Transparent roof corner hides the actor");
        for (const auto x : {2.f, 116.f})
        {
            const auto feet = sf::FloatRect(roof + sf::Vector2f(x, 114.f), {10.f, 6.f});
            const auto movement = terrain.ResolveMovement(feet, {0.f, -100.f});
            const auto feetY = feet.position.y + feet.size.y + movement.y;
            if (feetY < roof.y + 96.f || feetY >= roof.y + 128.f)
                throw std::runtime_error("Player does not stop beneath the roof overhang");
            actor.setSize({10.f, 24.f});
            actor.setPosition({roof.x + x, feetY - 24.f});
            roofTarget.clear();
            terrain.DrawWithActor(roofTarget, actor, feetY);
            roofTarget.display();
            const auto rendered = roofTarget.getTexture().copyToImage();
            const auto sampleX = static_cast<unsigned>(x + 5.f);
            auto edgeY = 95u;
            while (edgeY > 64 && roofImage.getPixel({sampleX, edgeY}).a == 0) --edgeY;
            if (rendered.getPixel({sampleX, edgeY}) != roofImage.getPixel({sampleX, edgeY}))
                throw std::runtime_error("Player approaching the eave is drawn on the roof");
        }
        std::cout << "Collision, tree depth, and roof depth tests passed\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
