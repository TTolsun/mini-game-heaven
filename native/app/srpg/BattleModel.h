#pragma once
#include <array>
#include <string>
#include "app/srpg/BattleData.h"
namespace app::srpg {
struct HeroProgress {
 int level=1,xp=0,training=0,gear=0,insights=0,practice=0;
 friend bool operator==(const HeroProgress&,const HeroProgress&)=default;
};
struct Unit {
 Cell cell; int kind=0,health=0,ki=0,chain=0,effort=0;
 bool active=false,acted=false,guard=false,chased=false,stunned=false;
 friend bool operator==(const Unit&,const Unit&)=default;
};
struct BattleState {
 Phase phase=Phase::Story; int lesson=0,round=1,choice=-1,bond=0,supplies=6,medicine=2,entryMedicine=2;
 int skills=kInitialSkills,discovered=0,entryDiscovered=0,claimed=0,chases=0;
 bool trained=false;
 std::array<bool,3> deployed{true,true,true};
 std::array<int,3> slots{0,1,2};
 std::array<HeroProgress,3> heroes{},entryHeroes{};
 std::array<Unit,kUnits> units{};
 friend bool operator==(const BattleState&,const BattleState&)=default;
};
struct Command { int actor=0; Cell destination; Action action=Action::Wait; int target=-1,skill=-1; };
enum class EventKind { Move, Hit, Push, Collision, Chase, Heal, Ki, Insight };
struct BattleEvent { EventKind kind; int actor,target,amount; Cell from,to; };
struct Outcome {
 bool valid=false; int dealt=0,received=0,kiChange=0,chases=0; bool learned=false;
 std::array<BattleEvent,40> events{}; int count=0;
 const char* message="행동을 선택하세요.";
 void add(EventKind kind,int actor,int target,int amount,Cell from={},Cell to={});
};
class BattleModel {
public:
 const BattleState& state() const { return state_; }
 const Lesson& lesson() const { return kLessonData[state_.lesson]; }
 bool blocked(Cell cell) const;
 int occupant(Cell cell) const;
 int maxHealth(int hero) const;
 int moveRange(int hero) const;
 bool threatened(Cell cell) const;
 bool visible(Cell from,Cell to) const;
 std::array<int,kCells> movement(int actor) const;
 bool chooseStory(int choice);
 bool train(int hero,int training);
 bool equip(int hero,int gear);
 bool research(int skill);
 bool canResearch(int skill) const;
 bool rest();
 bool prepare();
 bool returnToBase();
 bool toggleDeploy(int hero);
 bool place(int hero,int slot);
 bool startBattle();
 Outcome preview(Command command) const;
 Outcome execute(Command command);
 Outcome endTurn();
 bool nextLesson();
 bool retry();
 void restart();
 bool save(const std::string& path) const;
 bool load(const std::string& path);
private:
 friend struct BattleModelTestAccess;
 BattleState state_;
 void setupBattle();
 bool alive(int unit) const;
 bool attackRange(Cell from,Cell to,int range) const;
 void damage(int actor,int target,int value,Outcome& out,bool counter=false);
 void push(int actor,int target,int steps,Outcome& out);
 void follow(int actor,int target,Outcome& out);
 void enemyTurn(Outcome& out);
 void resolve();
 static bool validState(const BattleState& state);
};
}
