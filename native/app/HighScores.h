#pragma once

#include <map>
#include <string>

#include "app/GameRegistry.h"

namespace app {

// Best score per game, persisted as a tiny text file in the app's data dir.
class HighScores {
public:
    void load(const std::string& dir);

    int best(GameId id) const {
        const auto it = best_.find(id);
        return it == best_.end() ? 0 : it->second;
    }

    // Returns true when `score` is a new record. Saves immediately.
    bool submit(GameId id, int score);

private:
    void save() const;

    std::map<GameId, int> best_;
    std::string path_;
};

}  // namespace app
