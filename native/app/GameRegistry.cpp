#include "app/GameRegistry.h"

#include "app/games/DodgeGame.h"
#include "app/games/JumpGame.h"
#include "app/games/TapGame.h"

namespace app {

const std::vector<GameInfo>& allGames() {
    static const std::vector<GameInfo> kGames = {
        {GameId::Dodge, "DODGE", "Hold left / right to run", 2, engine::Color::rgb8(239, 83, 80), true},
        {GameId::Jump, "JUMP", "Tap to jump, tap again mid-air", 1, engine::Color::rgb8(124, 179, 66), true},
        {GameId::Tap, "TAP", "10 seconds. Smash the crate", 0, engine::Color::rgb8(41, 182, 246), true},
    };
    return kGames;
}

const GameInfo& gameInfo(GameId id) {
    for (const GameInfo& info : allGames()) {
        if (info.id == id) {
            return info;
        }
    }
    return allGames().front();
}

std::unique_ptr<IMiniGame> createGame(GameId id, const GameAssets& assets) {
    switch (id) {
    case GameId::Dodge:
        return std::make_unique<DodgeGame>(assets);
    case GameId::Jump:
        return std::make_unique<JumpGame>(assets);
    case GameId::Tap:
        return std::make_unique<TapGame>(assets);
    }
    return nullptr;
}

}  // namespace app
