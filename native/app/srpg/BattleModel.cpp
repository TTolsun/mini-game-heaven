#include "app/srpg/BattleModel.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <string>

namespace app::srpg {
namespace {
int distance(Cell first, Cell second) {
    return std::abs(first.x - second.x) + std::abs(first.y - second.y);
}
Cell plus(Cell first, Cell second) { return {first.x + second.x, first.y + second.y}; }
int directionTo(Cell from, Cell to) {
    const int dx = to.x - from.x;
    const int dy = to.y - from.y;
    if (std::abs(dx) > std::abs(dy)) return dx > 0 ? 1 : 3;
    return dy > 0 ? 2 : 0;
}
}
bool BattleModel::blocked(Cell cell) const {
    return !inside(cell) || lesson().tiles[cell.y][cell.x] == '#';
}
bool BattleModel::threatened(Cell cell) const {
    Cell at = state_.enemy;
    for (int step = 0; step < kStrikeRange; ++step) {
        at = plus(at, kDirections[state_.facing]);
        if (blocked(at)) return false;
        if (at == cell) return true;
    }
    return false;
}
std::array<int, kCells> BattleModel::movement() const {
    std::array<int, kCells> cost;
    cost.fill(-1);
    std::array<Cell, kCells> queue{};
    int begin = 0, end = 0;
    queue[end++] = state_.roshi;
    cost[index(state_.roshi)] = 0;
    while (begin < end) {
        const Cell current = queue[begin++];
        if (cost[index(current)] == kMove) continue;
        for (const Cell direction : kDirections) {
            const Cell next = plus(current, direction);
            if (blocked(next) || next == state_.enemy || cost[index(next)] >= 0) continue;
            cost[index(next)] = cost[index(current)] + 1;
            queue[end++] = next;
        }
    }
    return cost;
}
bool BattleModel::visible(Cell from, Cell to) const {
    // Supercover LOS: even a diagonal ray may not cut through a wall corner.
    const int dx = std::abs(to.x - from.x), dy = std::abs(to.y - from.y);
    const int sx = to.x > from.x ? 1 : -1, sy = to.y > from.y ? 1 : -1;
    int ix = 0, iy = 0;
    while (ix < dx || iy < dy) {
        const int decision = (1 + 2 * ix) * dy - (1 + 2 * iy) * dx;
        if (decision == 0) {
            if (blocked({from.x + sx, from.y}) || blocked({from.x, from.y + sy})) return false;
            from.x += sx; from.y += sy; ++ix; ++iy;
        } else if (decision < 0) { from.x += sx; ++ix; }
        else { from.y += sy; ++iy; }
        if (blocked(from)) return false;
    }
    return true;
}
Outcome BattleModel::preview(Command command) const {
    BattleModel copy = *this;
    return copy.execute(command);
}
Outcome BattleModel::execute(Command command) {
    Outcome out;
    out.message = "행동할 수 없는 위치입니다.";
    const int action = static_cast<int>(command.action);
    if (state_.phase != Phase::Playing || !inside(command.destination) || action < 0 || action > 4) return out;
    if (movement()[index(command.destination)] < 0) return out;
    const int range = distance(command.destination, state_.enemy);
    const bool technique = command.action == Action::Technique;
    const bool masteredBefore = state_.successes == kMasteryUses;
    const int techniqueCost = masteredBefore ? kMasterCost : kCopyCost;
    if ((command.action == Action::Strike || technique) && range != 1) {
        out.message = "타격과 붕권은 인접한 상대에게 사용합니다."; return out;
    }
    if (technique && state_.insights != kFullInsight) {
        out.message = "동작 · 발놀림 · 충격을 간파하면 모방이 열립니다."; return out;
    }
    if (technique && state_.ki < techniqueCost) {
        out.message = "기력이 부족합니다. 안전한 자리에서 기를 모으세요."; return out;
    }
    const BattleState before = state_;
    const bool startedThreatened = threatened(before.roshi);
    const bool hit = threatened(command.destination);
    const bool observing = command.action == Action::Observe;
    const bool guarding = command.action == Action::Guard;
    const bool charging = command.action == Action::Charge;
    const bool sees = range <= kObserveRange && visible(command.destination, state_.enemy);
    state_.roshi = command.destination;
    out.valid = true;
    out.message = "상대는 발을 고쳐 다음 공격을 준비합니다.";

    if (command.action == Action::Strike || technique) {
        const int damage = technique ? (masteredBefore ? kMasterDamage : kCopyDamage) : kBasicDamage;
        out.dealt = std::min(damage, state_.enemyHealth);
        state_.enemyHealth -= out.dealt;
        if (technique) {
            state_.ki -= techniqueCost;
            state_.successes = std::min(kMasteryUses, state_.successes + 1);
            if (masteredBefore && state_.enemyHealth > 0) {
                const Cell away{state_.enemy.x - state_.roshi.x, state_.enemy.y - state_.roshi.y};
                const Cell landing = plus(state_.enemy, away);
                if (blocked(landing)) {
                    out.wallHit = true;
                    const int extra = std::min(kWallDamage, state_.enemyHealth);
                    out.dealt += extra; state_.enemyHealth -= extra;
                } else { state_.enemy = landing; out.pushed = true; }
            }
        }
    }
    if (charging) {
        if (state_.roshi != before.roshi) state_.chargeChain = 0;
        state_.ki = std::min(kMaxKi, state_.ki + kChargeGains[state_.chargeChain]);
        state_.chargeChain = std::min(2, state_.chargeChain + 1);
    } else { state_.chargeChain = 0; }

    // Insights require an actual surviving opponent's release. A killed or
    // interrupted enemy cannot be observed; each distinct clue is awarded once.
    const bool released = state_.enemyHealth > 0 && !out.pushed && !out.wallHit;
    if (released) {
        if (observing && sees) state_.insights |= static_cast<int>(Insight::Motion);
        if (startedThreatened && !hit && sees) state_.insights |= static_cast<int>(Insight::Footwork);
        if (hit && guarding) state_.insights |= static_cast<int>(Insight::Impact);
        if (hit) {
            int damage = lesson().enemyDamage;
            if (guarding) damage = (damage + 1) / 2;
            if (charging || (technique && !masteredBefore)) damage += kExposedDamage;
            out.received = std::min(damage, state_.health);
            state_.health -= out.received;
            if (charging) {
                state_.ki = std::max(0, state_.ki - kInterruptedKi);
                state_.chargeChain = 0;
            }
        }
    }
    out.kiChange = state_.ki - before.ki;
    out.newInsights = state_.insights & ~before.insights;
    out.learned = before.insights != kFullInsight && state_.insights == kFullInsight;
    out.mastered = !masteredBefore && state_.successes == kMasteryUses;
    if (out.mastered) out.message = "체득! 철산격이 로시류 붕권이 되었습니다.";
    else if (out.learned) out.message = "간파 완료! 철산격 모방을 사용할 수 있습니다.";
    else if (out.newInsights != 0) out.message = "깨달음! 다른 상황에서도 상대의 움직임을 살펴보세요.";
    else if (out.wallHit) out.message = "벽 충돌! 상대의 공격 자세가 무너졌습니다.";
    else if (out.pushed) out.message = "밀쳐내기! 예고된 공격을 끊었습니다.";
    else if (charging && hit) out.message = "집중이 끊겼습니다. 기를 모을 안전한 자리가 필요합니다.";
    else if (charging) out.message = "기 축적! 같은 자리에서 이어 모으면 더 빠릅니다.";

    if (state_.health == 0) state_.phase = Phase::Defeat;
    else if (state_.enemyHealth == 0) state_.phase = Phase::Victory;
    else if (state_.round >= lesson().roundLimit) state_.phase = Phase::Defeat;
    else { ++state_.round; prepareEnemy(); }
    return out;
}
void BattleModel::prepareEnemy() {
    // BFS towards the player avoids a greedy AI getting stuck behind pillars.
    if (distance(state_.enemy, state_.roshi) > kStrikeRange || !visible(state_.enemy, state_.roshi)) {
        std::array<int, kCells> costs;
        costs.fill(-1);
        std::array<Cell, kCells> queue{};
        int begin = 0, end = 0;
        queue[end++] = state_.roshi; costs[index(state_.roshi)] = 0;
        while (begin < end) {
            const Cell current = queue[begin++];
            for (const Cell direction : kDirections) {
                const Cell next = plus(current, direction);
                if (blocked(next) || costs[index(next)] >= 0) continue;
                costs[index(next)] = costs[index(current)] + 1; queue[end++] = next;
            }
        }
        Cell best = state_.enemy;
        int bestCost = costs[index(best)] < 0 ? kCells : costs[index(best)];
        for (const Cell direction : kDirections) {
            const Cell next = plus(state_.enemy, direction);
            if (blocked(next) || next == state_.roshi) continue;
            const int cost = costs[index(next)];
            if (cost >= 0 && cost < bestCost) { best = next; bestCost = cost; }
        }
        state_.enemy = best;
    }
    state_.facing = directionTo(state_.enemy, state_.roshi);
}
void BattleModel::resetLesson(int value, int insights, int successes) {
    state_ = {};
    state_.lesson = value;
    state_.roshi = lesson().start;
    state_.enemy = lesson().opponent;
    state_.enemyHealth = lesson().enemyHealth;
    state_.insights = state_.entryInsights = insights;
    state_.successes = state_.entrySuccesses = successes;
    state_.facing = directionTo(state_.enemy, state_.roshi);
}
bool BattleModel::nextLesson() {
    if (state_.phase != Phase::Victory || state_.lesson + 1 >= kLessons) return false;
    resetLesson(state_.lesson + 1, state_.insights, state_.successes); return true;
}
void BattleModel::retry() { resetLesson(state_.lesson, state_.entryInsights, state_.entrySuccesses); }
void BattleModel::restart() { resetLesson(0, 0, 0); }
bool BattleModel::validState(const BattleState& value) {
    if (value.lesson < 0 || value.lesson >= kLessons || !inside(value.roshi) || !inside(value.enemy)) return false;
    const auto& data = kLessonData[value.lesson];
    if (value.roshi == value.enemy || data.tiles[value.roshi.y][value.roshi.x] == '#' || data.tiles[value.enemy.y][value.enemy.x] == '#') return false;
    if (value.round < 1 || value.round > data.roundLimit || value.health < 0 || value.health > kHealth ||
        value.enemyHealth < 0 || value.enemyHealth > data.enemyHealth || value.facing < 0 || value.facing > 3 ||
        value.ki < 0 || value.ki > kMaxKi || value.chargeChain < 0 || value.chargeChain > 2 ||
        value.insights < 0 || value.insights > kFullInsight || value.entryInsights < 0 || value.entryInsights > kFullInsight ||
        value.successes < 0 || value.successes > kMasteryUses || value.entrySuccesses < 0 || value.entrySuccesses > value.successes) return false;
    if ((value.entryInsights & value.insights) != value.entryInsights ||
        (value.successes > 0 && value.insights != kFullInsight) ||
        (value.entrySuccesses > 0 && value.entryInsights != kFullInsight)) return false;
    switch (value.phase) {
    case Phase::Playing: return value.health > 0 && value.enemyHealth > 0;
    case Phase::Victory: return value.health > 0 && value.enemyHealth == 0;
    case Phase::Defeat: return value.enemyHealth > 0 && (value.health == 0 || value.round == data.roundLimit);
    }
    return false;
}
bool BattleModel::save(const std::string& path) const {
    if (!validState(state_)) return false;
    const std::string temporary = path + ".tmp";
    {
        std::ofstream out(temporary, std::ios::trunc);
        const auto& s = state_;
        out << "WUTEN 1\n" << s.lesson << ' ' << s.round << ' ' << s.roshi.x << ' ' << s.roshi.y << ' '
            << s.enemy.x << ' ' << s.enemy.y << ' ' << s.facing << ' ' << s.health << ' ' << s.enemyHealth << ' '
            << s.ki << ' ' << s.chargeChain << ' ' << s.insights << ' ' << s.successes << ' '
            << s.entryInsights << ' ' << s.entrySuccesses << ' ' << static_cast<int>(s.phase) << '\n';
        out.flush();
        if (!out) return false;
        out.close();
        if (!out) return false;
    }
    // Android/POSIX rename atomically replaces an existing destination.
    return std::rename(temporary.c_str(), path.c_str()) == 0;
}
bool BattleModel::load(const std::string& path) {
    std::ifstream in(path);
    std::string magic;
    int version = 0, phase = 0;
    BattleState candidate;
    auto& s = candidate;
    if (!(in >> magic >> version) || magic != "WUTEN" || version != 1) return false;
    if (!(in >> s.lesson >> s.round >> s.roshi.x >> s.roshi.y >> s.enemy.x >> s.enemy.y >> s.facing
          >> s.health >> s.enemyHealth >> s.ki >> s.chargeChain >> s.insights >> s.successes
          >> s.entryInsights >> s.entrySuccesses >> phase) || phase < 0 || phase > 2) return false;
    in >> std::ws;
    if (!in.eof()) return false;
    s.phase = static_cast<Phase>(phase);
    if (!validState(candidate)) return false;
    state_ = candidate; return true;
}
} // namespace app::srpg
