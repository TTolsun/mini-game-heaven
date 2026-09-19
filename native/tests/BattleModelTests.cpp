#include "app/srpg/BattleModel.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <limits>
using namespace app::srpg;
namespace app::srpg {
struct BattleModelTestAccess { static BattleState& state(BattleModel& m) { return m.state_; } };
}
using Access=BattleModelTestAccess;
void launch(BattleModel& m) { assert(m.chooseStory(0)); assert(m.prepare()); assert(m.startBattle()); }
Outcome turn(BattleModel& m,Command c) {
 const auto before=m.state(); const auto predicted=m.preview(c); assert(m.state()==before);
 const auto actual=m.execute(c); assert(actual.valid && predicted.valid);
 assert(actual.dealt==predicted.dealt && actual.received==predicted.received && actual.chases==predicted.chases && actual.kiChange==predicted.kiChange && actual.count==predicted.count);
 for(int i=0;i<actual.count;++i) { assert(actual.events[i].kind==predicted.events[i].kind && actual.events[i].target==predicted.events[i].target && actual.events[i].amount==predicted.events[i].amount); }
 return actual;
}
Command bestCommand(const BattleModel& m) {
 int best=std::numeric_limits<int>::min(); Command command; const auto& s=m.state();
 for(int actor=0;actor<3;++actor) if(s.units[actor].active && s.units[actor].health>0 && !s.units[actor].acted) {
  const auto costs=m.movement(actor);
  for(int pos=0;pos<kCells;++pos) if(costs[pos]>=0) {
   const Cell dest=cellAt(pos);
   for(int action=0;action<=6;++action) {
    if(action==6) continue;
    const int count=action==1?kSkills:1;
    for(int skill=0;skill<count;++skill) {
     if(action==1 && (!(s.skills&(1<<skill)) || kSkillData[skill].owner!=actor)) continue;
     for(int target=0;target<kUnits;++target) {
      if(action!=0 && action!=1 && action!=4 && action!=5 && target>0) continue;
      Command candidate{actor,dest,static_cast<Action>(action),target,action==1?skill:-1};
      const auto out=m.preview(candidate); if(!out.valid) continue;
      int nearest=100; for(int enemy=3;enemy<kUnits;++enemy) if(s.units[enemy].active && s.units[enemy].health>0) nearest=std::min(nearest,std::abs(dest.x-s.units[enemy].cell.x)+std::abs(dest.y-s.units[enemy].cell.y));
      int score=out.dealt*12-out.received*16+out.chases*12-nearest*3;
      if(action==3 && s.units[actor].ki<30) score+=std::max(0,out.kiChange)*3;
      if(action==4 && !(s.discovered&(1<<s.units[target].kind))) score+=30;
      for(int e=0;e<out.count;++e) if(out.events[e].kind==EventKind::Heal) score+=out.events[e].amount*9;
      if(score>best) { best=score; command=candidate; }
     }
    }
   }
  }
 }
 assert(best!=std::numeric_limits<int>::min()); return command;
}
int main(int argc,char** argv) {
 const std::string path=std::string(argc>1?argv[1]:".")+"/srpg-campaign-test.txt";
 BattleModel m;
 assert(!m.prepare() && !m.startBattle() && !m.nextLesson());
 assert(m.chooseStory(0) && !m.chooseStory(0) && m.state().bond==1);
 assert(m.train(1,3) && !m.train(0,0)); assert(m.equip(0,1) && m.moveRange(0)==1);
 assert(m.equip(0,0)); assert(!m.research(1));
 assert(m.prepare()); assert(m.toggleDeploy(2) && !m.toggleDeploy(1) && !m.toggleDeploy(0));
 assert(m.toggleDeploy(2)); assert(m.place(0,1)); assert(m.state().slots[0]==1 && m.state().slots[1]==0);
 assert(m.startBattle());
 const auto initial=m.state();
 for(Command invalid:{Command{0,{-1,0},Action::Wait},Command{9,{3,5},Action::Guard},Command{0,m.state().units[3].cell,Action::Strike,3},Command{0,m.state().units[0].cell,Action::Skill,3,99}}) {
  assert(!m.execute(invalid).valid && m.state()==initial);
 }
 turn(m,{0,m.state().units[0].cell,Action::Guard}); assert(m.state().round==1);
 const auto acted=m.state(); assert(!m.execute({0,acted.units[0].cell,Action::Wait}).valid && m.state()==acted);
 turn(m,{1,m.state().units[1].cell,Action::Charge}); turn(m,{2,m.state().units[2].cell,Action::Charge}); assert(m.state().round==2);
 assert(m.save(path)); BattleModel loaded; assert(loaded.load(path) && loaded.state()==m.state());
 { std::ofstream out(path,std::ios::app); out<<" trailing"; } assert(!loaded.load(path) && loaded.state()==m.state());
 // Controlled collision: pushed enemy enters Shen's adjacency, once per round.
 BattleModel combo; launch(combo); auto& c=Access::state(combo);
 c.units[0].cell={2,2}; c.units[0].ki=100; c.units[1].cell={4,3}; c.units[2].cell={0,6}; c.units[3].cell={3,2}; c.units[4].cell={6,0};
 if(argc>2) assert(combo.save(argv[2])); // Optional reproducible visual-test fixture.
 auto out=turn(combo,{0,{2,2},Action::Skill,3,0});
 assert(out.chases==1 && c.units[3].cell==(Cell{4,2}) && c.units[1].chased && c.units[0].ki==90 && c.units[1].ki==10);
 assert(c.units[3].health==12); // 10 direct + 8 Shen chase, no ki awarded to chaser.
 // Collision damages both enemies; blocked landing never overlaps or moves.
 BattleModel collision; launch(collision); auto& x=Access::state(collision);
 x.units[0].cell={2,2}; x.units[0].ki=30; x.units[3].cell={3,2}; x.units[4].cell={4,2};
 out=turn(collision,{0,{2,2},Action::Skill,3,0}); assert(out.chases==0 && out.dealt==22 && x.units[3].cell==(Cell{3,2}) && x.units[4].health==24);
 // Line attack hits all enemies in line, but stops at terrain.
 BattleModel line; launch(line); auto& l=Access::state(line); l.skills|=1<<7;
 l.units[2].cell={3,5}; l.units[1].cell={5,5}; l.units[2].ki=100; l.units[3].cell={3,3}; l.units[4].cell={3,2};
 out=turn(line,{2,{3,5},Action::Skill,3,7}); assert(out.dealt==40 && l.units[2].ki==60);
 // Self-healing follows the moved actor, not their old square.
 BattleModel heal; launch(heal); auto& h=Access::state(heal); h.units[1].health=10;
 turn(heal,{1,{5,6},Action::Item,1}); assert(h.units[1].health==34 && h.medicine==1);
 // Research cannot be purchased without discovered conditions; one-time resource sink.
 BattleModel research; assert(research.chooseStory(1)); auto& r=Access::state(research); r.heroes[0].insights=7; r.heroes[0].practice=2; r.discovered=4;
 assert(research.research(1) && !research.research(1) && research.research(7));
 // Retry restores all learning and consumables, without campaign rewards.
 BattleModel retry; launch(retry); auto& q=Access::state(retry); q.units[0].health=0; q.phase=Phase::Defeat; q.heroes[0].practice=5; q.discovered=15; q.medicine=0;
 assert(retry.retry() && q.heroes==q.entryHeroes && q.medicine==q.entryMedicine && q.discovered==q.entryDiscovered && q.claimed==0);
 // Every defined skill is executable, with resource spending and distinct effects.
 for(int id=0;id<kSkills;++id) {
  BattleModel skills; launch(skills); auto& z=Access::state(skills); z.skills=1023; z.bond=2;
  for(int i=0;i<3;++i) z.units[i].cell={i,5};
  const int actor=kSkillData[id].owner; z.units[actor].cell={2,2}; z.units[actor].ki=100;
  z.units[3].cell={3,2}; z.units[4].cell={3,0};
  if(id==9) z.units[1].cell={2,3};
  if(id==8) z.units[actor].health=10;
  const auto effect=turn(skills,{actor,{2,2},Action::Skill,id==8?actor:3,id});
  assert(z.units[actor].ki==100-kSkillData[id].cost);
  if(id==8) assert(z.units[actor].health==28); else assert(effect.dealt>0);
  if(id==5) assert(z.units[3].stunned);
 }
 BattleModel exposed; launch(exposed); auto& e=Access::state(exposed);
 e.units[0].cell={2,2}; e.units[3].cell={3,2}; e.units[4].cell={6,0};
 turn(exposed,{0,{2,2},Action::Charge}); const int health=e.units[0].health;
 assert(exposed.endTurn().valid); assert(e.units[0].health==health-14 && e.units[0].ki==25 && e.units[0].chain==0);
 BattleModel last; launch(last); auto& end=Access::state(last);
 end.round=last.lesson().roundLimit; end.units[0].cell={2,2}; end.units[3].cell={3,2}; end.units[3].health=1; end.units[4].health=0;
 turn(last,{0,{2,2},Action::Strike,3}); assert(end.phase==Phase::Victory);
 BattleModel expired; launch(expired); Access::state(expired).round=expired.lesson().roundLimit;
 assert(expired.endTurn().valid && expired.state().phase==Phase::Defeat);
 assert(expired.returnToBase()); assert((expired.state().units==std::array<Unit,kUnits>{}));
 assert(expired.prepare() && expired.returnToBase());
 BattleModel invalidBase; assert(invalidBase.chooseStory(0)); Access::state(invalidBase).units[0].cell={999,999}; assert(!invalidBase.save(path));
 { std::ofstream file(path); file<<"WUTEN 2\n0 0"; } const auto beforeLoad=loaded.state(); assert(!loaded.load(path) && loaded.state()==beforeLoad);
 // A complete campaign through the real public state machine, replaying saved checkpoints.
 BattleModel campaign; int total=0;
 for(int map=0;map<kLessons;++map) {
  assert(campaign.state().phase==Phase::Story); assert(campaign.chooseStory(0));
  for(int skill=0;skill<kSkills;++skill) if(campaign.canResearch(skill)) assert(campaign.research(skill));
  assert(campaign.prepare() && campaign.startBattle());
  while(campaign.state().phase==Phase::Playing && ++total<250) turn(campaign,bestCommand(campaign));
  std::printf("MAP %d: phase=%d round=%d roshi=%d chases=%d\n",map+1,static_cast<int>(campaign.state().phase),campaign.state().round,campaign.state().units[0].health,campaign.state().chases);
  assert(campaign.state().phase==Phase::Victory);
  assert(campaign.save(path)); BattleModel copy; assert(copy.load(path)); campaign=copy;
  const int supplies=campaign.state().supplies; assert(campaign.nextLesson()); assert(!campaign.nextLesson()); assert(campaign.state().supplies==supplies+4);
  assert(campaign.save(path)); assert(copy.load(path) && copy.state()==campaign.state());
 }
 assert(campaign.state().phase==Phase::Complete && campaign.state().claimed==31);
 // Corrupt values are rejected transactionally, including incorrect reward history.
 auto corrupt=campaign; Access::state(corrupt).claimed=0; assert(!corrupt.save(path));
 BattleModel stress; launch(stress); const auto begin=std::chrono::steady_clock::now(); int checksum=0;
 for(int i=0;i<10000;++i) checksum+=stress.preview({0,{2,4},Action::Charge}).valid;
 std::printf("10000 previews %.2f ms checksum=%d\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count(),checksum);
 assert(checksum==10000); std::remove(path.c_str());
 std::puts("PASS: campaign loop, deployment, atomic turns, preview, collision, pursuit, line skill, heal, research, retry, rewards, save and five-map completion");
}
