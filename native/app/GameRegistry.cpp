#include "app/GameRegistry.h"

#include "app/games/DodgeGame.h"

namespace app {

const std::vector<GameInfo>& allGames() {
    static const std::vector<GameInfo> kGames = {
        {GameId::Dodge, "DODGE", "Tap left / right to dodge", engine::Color::rgb8(255, 138, 101), true},
        {GameId::Jump, "JUMP", "Tap to jump", engine::Color::rgb8(129, 199, 132), false},
        {GameId::Tap, "TAP", "Tap as fast as you can", engine::Color::rgb8(100, 181, 246), false},
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
    case GameId::Tap:
        break;
    }
    return nullptr;
}

}  // namespace app
