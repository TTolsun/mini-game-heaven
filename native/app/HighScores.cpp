#define LOG_TAG "HighScores"
#include "app/HighScores.h"

#include <cstdio>

#include "engine/core/Log.h"

namespace app {

void HighScores::load(const std::string& dir) {
    path_ = dir + "/highscores.txt";
    best_.clear();

    FILE* file = std::fopen(path_.c_str(), "r");
    if (file == nullptr) {
        LOGI("no saved scores yet");
        return;
    }
    int id = 0;
    int score = 0;
    while (std::fscanf(file, "%d %d", &id, &score) == 2) {
        best_[static_cast<GameId>(id)] = score;
    }
    std::fclose(file);
    LOGI("loaded %zu scores", best_.size());
}

bool HighScores::submit(GameId id, int score) {
    if (score <= best(id)) {
        return false;
    }
    best_[id] = score;
    save();
    return true;
}

void HighScores::save() const {
    if (path_.empty()) {
        return;
    }
    // Write to a temp file then rename so a crash mid-write cannot corrupt the scores.
    const std::string temp = path_ + ".tmp";
    FILE* file = std::fopen(temp.c_str(), "w");
    if (file == nullptr) {
        LOGE("cannot write %s", temp.c_str());
        return;
    }
    for (const auto& [id, score] : best_) {
        std::fprintf(file, "%d %d\n", static_cast<int>(id), score);
    }
    std::fclose(file);
    std::rename(temp.c_str(), path_.c_str());
}

}  // namespace app
