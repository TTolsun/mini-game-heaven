#include "app/srpg/BattleModel.h"

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <limits>
#include <string>

using namespace app::srpg;

Outcome turn(BattleModel& model, Cell cell, Action action) {
    const auto before = model.state();
    const Outcome prediction = model.preview({cell,action});
    assert(model.state() == before);
    const Outcome actual = model.execute({cell,action});
    assert(actual.valid && actual.valid == prediction.valid);
    assert(actual.dealt == prediction.dealt && actual.received == prediction.received);
    assert(actual.kiChange == prediction.kiChange && actual.newInsights == prediction.newInsights);
    assert(actual.pushed == prediction.pushed && actual.wallHit == prediction.wallHit);
    return actual;
}
void writeState(const std::string& file, const BattleState& s) {
    std::ofstream out(file);
    out << "WUTEN 1\n" << s.lesson << ' ' << s.round << ' ' << s.roshi.x << ' ' << s.roshi.y << ' '
        << s.enemy.x << ' ' << s.enemy.y << ' ' << s.facing << ' ' << s.health << ' ' << s.enemyHealth << ' '
        << s.ki << ' ' << s.chargeChain << ' ' << s.insights << ' ' << s.successes << ' '
        << s.entryInsights << ' ' << s.entrySuccesses << ' ' << static_cast<int>(s.phase) << '\n';
}
void learnAndWin(BattleModel& model) {
    turn(model,{3,4},Action::Guard);
    assert(model.state().insights == 4 && model.state().health == 67);
    const auto repeated = turn(model,{3,4},Action::Guard);
    assert(repeated.newInsights == 0 && model.state().insights == 4);
    assert(turn(model,{4,3},Action::Observe).learned);
    assert(model.state().insights == kFullInsight);
    turn(model,{4,3},Action::Charge);
    turn(model,{4,2},Action::Technique);
    assert(model.state().successes == 1 && model.state().ki == 0);
    turn(model,{3,3},Action::Charge);
    assert(turn(model,{3,3},Action::Technique).mastered);
    assert(model.state().successes == kMasteryUses);
    turn(model,{4,2},Action::Charge);
    turn(model,{4,2},Action::Technique);
    assert(model.state().phase == Phase::Victory);
}
void finishMastered(BattleModel& model) {
    // A simple deterministic tactical policy: charge outside the telegraph,
    // then use the learned technique. Evaluate all legal destinations/actions.
    while (model.state().phase == Phase::Playing) {
        int bestScore = std::numeric_limits<int>::min();
        Command best{model.state().roshi,Action::Guard};
        for (int cell = 0; cell < kCells; ++cell) for (int action = 0; action < 5; ++action) {
            const Command candidate{cellAt(cell),static_cast<Action>(action)};
            const Outcome result = model.preview(candidate);
            if (!result.valid) continue;
            const auto enemy = model.state().enemy;
            const int range = std::abs(candidate.destination.x-enemy.x)+std::abs(candidate.destination.y-enemy.y);
            int score = result.dealt*15 - result.received*30 - range*5;
            if (candidate.action == Action::Charge && model.state().ki < kMasterCost) score += result.kiChange*8;
            if (result.dealt == model.state().enemyHealth) score += 10000;
            if (score > bestScore) { bestScore = score; best = candidate; }
        }
        turn(model,best.destination,best.action);
    }
    assert(model.state().phase == Phase::Victory);
    std::printf("LESSON %d: victory round=%d health=%d mastery=%d\n",model.state().lesson+1,model.state().round,model.state().health,model.state().successes);
}
int main(int argc, char** argv) {
    const std::string file = std::string(argc > 1 ? argv[1] : ".") + "/srpg-test-save.txt";
    BattleModel model;
    const auto initial = model.state();
    for (const Command invalid : {Command{{-1,0},Action::Guard}, Command{{3,2},Action::Guard},
         Command{{0,0},Action::Guard}, Command{{3,4},Action::Technique}, Command{{3,4},static_cast<Action>(99)}}) {
        assert(!model.execute(invalid).valid && model.state() == initial);
    }
    assert(!model.nextLesson());
    // Charged while exposed: 20 gained, 10 lost, 10+6 damage, chain broken.
    const auto unsafe = turn(model,{3,4},Action::Charge);
    assert(unsafe.received == 16 && model.state().ki == 10 && model.state().chargeChain == 0);
    model.retry(); assert(model.state() == initial);
    learnAndWin(model);
    assert(model.save(file));
    BattleModel restored;
    assert(restored.load(file) && restored.state() == model.state());
    assert(restored.save(file)); // Atomic replacement of an existing save.
    assert(!restored.execute({restored.state().roshi,Action::Guard}).valid);
    assert(restored.nextLesson());
    const auto entry = restored.state();
    assert(entry.insights == kFullInsight && entry.successes == kMasteryUses);
    assert(restored.blocked({1,1}) && restored.movement()[index({1,1})] == -1);
    finishMastered(restored);
    restored.retry(); assert(restored.state() == entry);
    finishMastered(restored);
    assert(restored.nextLesson());
    finishMastered(restored);
    assert(!restored.nextLesson());

    // Blocked push becomes collision damage and interrupts the telegraphed hit.
    auto wall = entry;
    wall.roshi={3,1}; wall.enemy={2,1}; wall.facing=1; wall.ki=100;
    writeState(file,wall); assert(restored.load(file));
    const auto collision = turn(restored,wall.roshi,Action::Technique);
    assert(collision.wallHit && collision.dealt == kMasterDamage+kWallDamage && collision.received == 0);
    // A clear push moves the opponent and cancels its old attack line.
    auto push = entry;
    push.roshi={3,4}; push.enemy={3,3}; push.facing=2; push.ki=100;
    writeState(file,push); assert(restored.load(file));
    const auto knockback = turn(restored,push.roshi,Action::Technique);
    assert(knockback.pushed && knockback.received == 0 && restored.state().enemy == (Cell{3,2}));

    // A wall blocks both attack propagation and observation (supercover LOS).
    auto hidden = entry;
    hidden.roshi={1,2}; hidden.enemy={1,0}; hidden.insights=0; hidden.entryInsights=0;
    hidden.successes=0; hidden.entrySuccesses=0; hidden.facing=2;
    writeState(file,hidden); assert(restored.load(file));
    assert(!restored.threatened(hidden.roshi));
    assert(turn(restored,hidden.roshi,Action::Observe).newInsights == 0);
    // Retry rolls back discoveries made inside the current lesson.
    model.restart(); turn(model,{3,4},Action::Guard); model.retry(); assert(model.state() == initial);

    // Rejected saves never partially mutate a running battle.
    const auto stable = restored.state();
    for (int kind=0; kind<6; ++kind) {
        auto bad = initial;
        if (kind==0) bad.roshi={99,0};
        if (kind==1) bad.enemy=bad.roshi;
        if (kind==2) bad.successes=1;
        if (kind==3) bad.phase=Phase::Victory;
        if (kind==4) bad.entryInsights=7;
        if (kind==5) bad.health=-1;
        writeState(file,bad); assert(!restored.load(file) && restored.state()==stable);
    }
    { std::ofstream out(file); out << "WUTEN 1\n0 1 3"; }
    assert(!restored.load(file) && restored.state()==stable);
    writeState(file,initial);
    { std::ofstream out(file,std::ios::app); out << "trailing data"; }
    assert(!restored.load(file));

    auto lastTurn = initial; lastTurn.round=kLessonData[0].roundLimit;
    lastTurn.enemyHealth=kBasicDamage; lastTurn.roshi={4,2};
    writeState(file,lastTurn); assert(restored.load(file));
    turn(restored,lastTurn.roshi,Action::Strike); assert(restored.state().phase==Phase::Victory);
    lastTurn.enemyHealth=30;
    writeState(file,lastTurn); assert(restored.load(file));
    turn(restored,lastTurn.roshi,Action::Guard); assert(restored.state().phase==Phase::Defeat);
    assert(restored.save(file));
    BattleModel resumed; assert(resumed.load(file) && resumed.state()==restored.state());

    const auto start=std::chrono::steady_clock::now();
    int checksum=0;
    for (int i=0;i<10000;++i) { BattleModel sample; checksum += sample.preview({{4,3},Action::Observe}).newInsights; }
    const double elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    std::printf("10000 previews: %.2f ms (model only; checksum=%d)\n",elapsed,checksum);
    std::remove(file.c_str());
    std::puts("PASS: insight diversity, preview parity, mastery, three lessons, ki risk, geometry, retry, save validation and final-turn objectives");
}
