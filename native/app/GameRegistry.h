#pragma once

#include <memory>
#include <vector>

#include "app/IMiniGame.h"
#include "engine/math/Color.h"

namespace app {

class GameAssets;

enum class GameId {
    Dodge,
    Jump,
    Tap,
};

struct GameInfo {
    GameId id;
    const char* title;
    const char* hint;      // one line shown on the menu card
    engine::Color accent;  // card colour
    bool available;        // false until the game is implemented
};

// Adding a game: implement IMiniGame, then register it in GameRegistry.cpp.
const std::vector<GameInfo>& allGames();
const GameInfo& gameInfo(GameId id);
std::unique_ptr<IMiniGame> createGame(GameId id, const GameAssets& assets);

}  // namespace app
