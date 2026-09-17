#pragma once

#include <map>

#include "app/GameRegistry.h"

namespace app {

// Best score per game. In-memory for now; persistence arrives with settings.
class HighScores {
public:
    int best(GameId id) const {
        const auto it = best_.find(id);
        return it == best_.end() ? 0 : it->second;
    }

    // Returns true when `score` is a new record.
    bool submit(GameId id, int score) {
        if (score <= best(id)) {
            return false;
        }
        best_[id] = score;
        return true;
    }

private:
    std::map<GameId, int> best_;
};

}  // namespace app
