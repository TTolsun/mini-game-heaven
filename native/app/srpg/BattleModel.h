#pragma once

#include <array>
#include <string>
#include "app/srpg/BattleData.h"

namespace app::srpg {
struct BattleState {
    int lesson = 0;
    int round = 1;
    Cell roshi{3,4};
    Cell enemy{3,2};
    int facing = 2;
    int health = kHealth;
    int enemyHealth = 30;
    int ki = 0;
    int chargeChain = 0;
    int insights = 0;
    int successes = 0;
    int entryInsights = 0;
    int entrySuccesses = 0;
    Phase phase = Phase::Playing;
    friend bool operator==(const BattleState&, const BattleState&) = default;
};
struct Command { Cell destination; Action action = Action::Guard; };
struct Outcome {
    bool valid = false;
    int dealt = 0;
    int received = 0;
    int kiChange = 0;
    int newInsights = 0;
    bool learned = false;
    bool mastered = false;
    bool pushed = false;
    bool wallHit = false;
    const char* message = "행동을 선택하세요.";
};

// A complete command and the enemy response form one atomic, deterministic
// round. Presentation never changes rules; preview executes a copy of the model.
class BattleModel {
public:
    const BattleState& state() const { return state_; }
    const Lesson& lesson() const { return kLessonData[state_.lesson]; }
    bool blocked(Cell cell) const;
    bool threatened(Cell cell) const;
    std::array<int, kCells> movement() const;
    Outcome preview(Command command) const;
    Outcome execute(Command command);
    bool nextLesson();
    void retry();
    void restart();
    bool save(const std::string& path) const;
    bool load(const std::string& path);
private:
    BattleState state_;
    bool visible(Cell from, Cell to) const;
    void prepareEnemy();
    void resetLesson(int lesson, int insights, int successes);
    static bool validState(const BattleState& state);
};
} // namespace app::srpg
