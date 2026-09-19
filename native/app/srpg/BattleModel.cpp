#include "app/srpg/BattleModel.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
namespace app::srpg {
namespace {
int distance(Cell a,Cell b) { return std::abs(a.x-b.x)+std::abs(a.y-b.y); }
Cell plus(Cell a,Cell b) { return {a.x+b.x,a.y+b.y}; }
bool battlePhase(Phase phase) { return phase==Phase::Playing || phase==Phase::Victory || phase==Phase::Defeat; }
}
void Outcome::add(EventKind kind,int actor,int target,int amount,Cell from,Cell to) {
 if(count<static_cast<int>(events.size())) events[count++]={kind,actor,target,amount,from,to};
}
bool BattleModel::alive(int unit) const { return unit>=0 && unit<kUnits && state_.units[unit].active && state_.units[unit].health>0; }
bool BattleModel::blocked(Cell cell) const { return !inside(cell) || lesson().tiles[cell.y][cell.x]=='#'; }
int BattleModel::occupant(Cell cell) const {
 for(int i=0;i<kUnits;++i) if(alive(i) && state_.units[i].cell==cell) return i;
 return -1;
}
int BattleModel::maxHealth(int hero) const {
 const auto& p=state_.heroes[hero];
 return kHeroData[hero].health+(p.level-1)*4+(p.training==0?8:0);
}
int BattleModel::moveRange(int hero) const { return std::max(1,kHeroData[hero].move-(state_.heroes[hero].gear==1?1:0)); }
bool BattleModel::visible(Cell from,Cell to) const {
 const int dx=std::abs(to.x-from.x),dy=std::abs(to.y-from.y);
 const int sx=to.x>from.x?1:-1,sy=to.y>from.y?1:-1;
 int ix=0,iy=0;
 while(ix<dx || iy<dy) {
  const int decision=(1+2*ix)*dy-(1+2*iy)*dx;
  if(decision==0) {
   if(blocked({from.x+sx,from.y}) || blocked({from.x,from.y+sy})) return false;
   from.x+=sx; from.y+=sy; ++ix; ++iy;
  } else if(decision<0) { from.x+=sx; ++ix; } else { from.y+=sy; ++iy; }
  if(blocked(from)) return false;
 }
 return true;
}
bool BattleModel::attackRange(Cell from,Cell to,int range) const {
 return distance(from,to)>0 && distance(from,to)<=range && (from.x==to.x || from.y==to.y) && visible(from,to);
}
bool BattleModel::threatened(Cell cell) const {
 for(int i=kHeroes;i<kUnits;++i) if(alive(i) && !state_.units[i].stunned && attackRange(state_.units[i].cell,cell,kEnemyData[state_.units[i].kind].range)) return true;
 return false;
}
std::array<int,kCells> BattleModel::movement(int actor) const {
 std::array<int,kCells> costs; costs.fill(-1);
 if(actor<0 || actor>=kHeroes || !alive(actor)) return costs;
 std::array<Cell,kCells> queue{}; int begin=0,end=0;
 queue[end++]=state_.units[actor].cell; costs[index(queue[0])]=0;
 while(begin<end) {
  const Cell current=queue[begin++];
  if(costs[index(current)]>=moveRange(actor)) continue;
  for(const auto direction:kDirections) {
   const Cell next=plus(current,direction);
   if(blocked(next) || occupant(next)>=0 || costs[index(next)]>=0) continue;
   costs[index(next)]=costs[index(current)]+1; queue[end++]=next;
  }
 }
 return costs;
}
bool BattleModel::chooseStory(int choice) {
 if(state_.phase!=Phase::Story || choice<0 || choice>2) return false;
 state_.choice=choice;
 if(choice==0) state_.bond=std::min(5,state_.bond+1);
 if(choice==1) state_.supplies=std::min(99,state_.supplies+1);
 state_.trained=false; state_.phase=Phase::Base; return true;
}
bool BattleModel::train(int hero,int training) {
 if(state_.phase!=Phase::Base || hero<0 || hero>=3 || training<0 || training>3 || state_.trained || state_.supplies<2) return false;
 state_.heroes[hero].training=training; state_.supplies-=2; state_.trained=true; return true;
}
bool BattleModel::equip(int hero,int gear) {
 if(state_.phase!=Phase::Base || hero<0 || hero>=3 || gear<0 || gear>2) return false;
 state_.heroes[hero].gear=gear; return true;
}
bool BattleModel::canResearch(int skill) const {
 if(state_.phase!=Phase::Base || skill<0 || skill>=kSkills || (state_.skills&(1<<skill)) || state_.supplies<1) return false;
 switch(skill) {
 case 1: return state_.heroes[0].practice>=2 && state_.heroes[0].insights==7;
 case 2: return state_.lesson>=2;
 case 4: return state_.lesson>=1;
 case 5: return state_.lesson>=2 && (state_.discovered&(1<<1));
 case 7: return (state_.discovered&(1<<2))!=0;
 case 9: return state_.lesson>=3 && state_.bond>=2;
 default: return false;
 }
}
bool BattleModel::research(int skill) {
 if(!canResearch(skill)) return false;
 state_.skills|=1<<skill; --state_.supplies; return true;
}
bool BattleModel::rest() {
 if(state_.phase!=Phase::Base || state_.medicine>=3 || state_.supplies<1) return false;
 --state_.supplies; ++state_.medicine; return true;
}
bool BattleModel::prepare() {
 if(state_.phase!=Phase::Base) return false;
 state_.phase=Phase::Deployment; return true;
}
bool BattleModel::returnToBase() {
 if(state_.phase!=Phase::Deployment && state_.phase!=Phase::Defeat) return false;
 if(state_.phase==Phase::Defeat) { state_.heroes=state_.entryHeroes; state_.discovered=state_.entryDiscovered; state_.medicine=state_.entryMedicine; }
 state_.units={}; state_.round=1; state_.phase=Phase::Base; return true;
}
bool BattleModel::toggleDeploy(int hero) {
 if(state_.phase!=Phase::Deployment || hero<1 || hero>=3) return false;
 const int count=static_cast<int>(std::count(state_.deployed.begin(),state_.deployed.end(),true));
 if(state_.deployed[hero] && count<=2) return false;
 state_.deployed[hero]=!state_.deployed[hero]; return true;
}
bool BattleModel::place(int hero,int slot) {
 if(state_.phase!=Phase::Deployment || hero<0 || hero>=3 || slot<0 || slot>=3) return false;
 for(int i=0;i<3;++i) if(state_.slots[i]==slot) { std::swap(state_.slots[i],state_.slots[hero]); return true; }
 return false;
}
bool BattleModel::startBattle() {
 if(state_.phase!=Phase::Deployment) return false;
 state_.entryHeroes=state_.heroes; state_.entryDiscovered=state_.discovered; state_.entryMedicine=state_.medicine;
 setupBattle(); return true;
}
void BattleModel::setupBattle() {
 state_.units={}; state_.round=1; state_.chases=0;
 for(int i=0;i<3;++i) if(state_.deployed[i]) {
  auto& u=state_.units[i]; u.active=true; u.kind=i; u.cell=lesson().starts[state_.slots[i]];
  u.health=maxHealth(i); u.ki=(state_.choice==2?20:10)+(state_.heroes[i].training==1?10:0);
 }
 for(int i=0;i<lesson().enemyCount;++i) {
  auto& u=state_.units[i+3]; u.active=true; u.kind=lesson().kinds[i]; u.cell=lesson().enemies[i];
  u.health=kEnemyData[u.kind].health+state_.lesson*3;
 }
 state_.phase=Phase::Playing;
}
void BattleModel::damage(int actor,int target,int value,Outcome& out,bool counter) {
 if(!alive(target)) return;
 auto& u=state_.units[target];
 if(target<3) {
  const auto& p=state_.heroes[target];
  value-=p.gear==0?2:0;
  if(u.guard) value=(value+1)/2-(p.training==2?2:0);
  if(u.chain>0) { value+=6; u.ki=std::max(0,u.ki-10); }
 } else if(u.kind==1) value-=3;
 const int hit=std::min(u.health,std::max(1,value));
 u.health-=hit; u.ki=std::min(kMaxKi,u.ki+5); u.chain=0;
 out.add(EventKind::Hit,actor,target,hit);
 if(target<3) out.received+=hit; else out.dealt+=hit;
 if(target<3 && u.guard) state_.heroes[target].insights|=4;
 if(!counter && target<3 && (target==0 || state_.heroes[target].gear==2) && u.guard && alive(target) && alive(actor) && distance(u.cell,state_.units[actor].cell)==1) {
  damage(target,actor,(target==0?6:0)+(state_.heroes[target].gear==2?3:0),out,true);
 }
}
void BattleModel::follow(int actor,int target,Outcome& out) {
 if(actor>=3 || !alive(target)) return;
 for(int i=0;i<3 && alive(target);++i) {
  auto& partner=state_.units[i];
  if(i==actor || !alive(i) || partner.chased || distance(partner.cell,state_.units[target].cell)!=1) continue;
  partner.chased=true; ++out.chases; state_.chases=std::min(99,state_.chases+1);
  const int amount=5+(i==1?3:0)+(state_.bond>=2?3:0);
  out.add(EventKind::Chase,i,target,amount);
  damage(i,target,amount,out,true); // No recursive push, follow-up or ki generation.
 }
}
void BattleModel::push(int actor,int target,int steps,Outcome& out) {
 if(!alive(target)) return;
 const Cell a=state_.units[actor].cell,b=state_.units[target].cell;
 const Cell direction{(b.x>a.x)-(b.x<a.x),(b.y>a.y)-(b.y<a.y)};
 bool displaced=false;
 for(int step=0;step<steps && alive(target);++step) {
  auto& u=state_.units[target]; const Cell next=plus(u.cell,direction);
  const int other=occupant(next);
  if(blocked(next) || other>=0) {
   out.add(EventKind::Collision,actor,target,6,u.cell,next);
   damage(actor,target,6,out,true);
   if(other>=3) damage(actor,other,6,out,true);
   break;
  }
  const Cell old=u.cell; u.cell=next; displaced=true;
  out.add(EventKind::Push,actor,target,step+1,old,next);
 }
 if(displaced) follow(actor,target,out);
}
void BattleModel::resolve() {
 if(!alive(0)) { state_.phase=Phase::Defeat; return; }
 bool remaining=false; for(int i=3;i<kUnits;++i) remaining|=alive(i);
 if(!remaining) state_.phase=Phase::Victory;
}
Outcome BattleModel::preview(Command command) const { BattleModel copy=*this; return copy.execute(command); }
Outcome BattleModel::execute(Command command) {
 Outcome out; out.message="이동·대상·기력을 확인하세요.";
 const int actor=command.actor,target=command.target,action=static_cast<int>(command.action);
 if(state_.phase!=Phase::Playing || actor<0 || actor>=3 || !alive(actor) || state_.units[actor].acted || action<0 || action>6 || !inside(command.destination)) return out;
 const auto moveCosts=movement(actor);
 if(moveCosts[index(command.destination)]<0) return out;
 const SkillData* skill=nullptr;
 if(command.action==Action::Strike || command.action==Action::Skill) {
  if(command.action==Action::Skill) {
   if(command.skill<0 || command.skill>=kSkills || !(state_.skills&(1<<command.skill))) return out;
   skill=&kSkillData[command.skill];
   if(skill->owner!=actor || state_.units[actor].ki<skill->cost) return out;
   if(command.skill==9 && (state_.bond<2 || !alive(1) || distance(command.destination,state_.units[1].cell)>2)) { out.message="쌍룡격은 관계 2와 두 칸 안의 학선인이 필요합니다."; return out; }
  }
  if(!alive(target)) return out;
  if(skill && skill->shape==SkillShape::Heal) {
   if(target>=3 || distance(command.destination,target==actor?command.destination:state_.units[target].cell)>skill->range || !visible(command.destination,target==actor?command.destination:state_.units[target].cell)) return out;
  } else if(target<3 || !attackRange(command.destination,state_.units[target].cell,skill?skill->range:1)) return out;
 }
 if(command.action==Action::Item && (state_.medicine==0 || !alive(target) || target>=3 || distance(command.destination,target==actor?command.destination:state_.units[target].cell)>1)) return out;
 if(command.action==Action::Observe && (!alive(target) || target<3 || distance(command.destination,state_.units[target].cell)>3 || !visible(command.destination,state_.units[target].cell))) return out;
 auto& u=state_.units[actor]; const Cell previous=u.cell; const int oldKi=u.ki;
 const bool evaded=threatened(previous) && !threatened(command.destination) && previous!=command.destination;
 out.valid=true; out.message="동료의 차례를 이어가세요.";
 u.cell=command.destination; u.acted=true; u.guard=false; u.effort=std::min(6,u.effort+1);
 if(previous!=u.cell) {
  std::array<Cell,kCells> path{}; int count=0; Cell at=u.cell;
  while(at!=previous) {
   path[count++]=at;
   for(const auto direction:kDirections) {
    const Cell next=plus(at,direction);
    if(inside(next) && moveCosts[index(next)]==moveCosts[index(at)]-1) { at=next;break; }
   }
  }
  for(int step=count-1;step>=0;--step) { out.add(EventKind::Move,actor,actor,0,at,path[step]);at=path[step]; }
  u.chain=0;
 }
 if(evaded) {
  u.ki=std::min(kMaxKi,u.ki+15); state_.heroes[actor].insights|=2;
  out.add(EventKind::Ki,actor,actor,15);
 }
 if(command.action!=Action::Charge) u.chain=0;
 if(command.action==Action::Strike || skill) {
  if(skill) u.ki-=skill->cost;
  if(skill && skill->shape==SkillShape::Heal) {
   auto& ally=state_.units[target]; const int heal=std::min(skill->damage,maxHealth(target)-ally.health);
   ally.health+=heal; out.add(EventKind::Heal,actor,target,heal);
  } else {
   const int power=(skill?skill->damage:8)+(state_.heroes[actor].level-1)+(state_.heroes[actor].training==3?2:0);
   if(skill && skill->shape==SkillShape::Cross) {
    for(int i=3;i<kUnits;++i) if(alive(i) && distance(u.cell,state_.units[i].cell)==1) damage(actor,i,power,out);
   } else if(skill && skill->shape==SkillShape::Line) {
    const Cell end=state_.units[target].cell;
    const Cell direction{(end.x>u.cell.x)-(end.x<u.cell.x),(end.y>u.cell.y)-(end.y<u.cell.y)};
    Cell at=u.cell;
    for(int n=0;n<skill->range;++n) { at=plus(at,direction); if(blocked(at)) break; const int victim=occupant(at); if(victim>=3) damage(actor,victim,power,out); }
   } else {
    damage(actor,target,power,out);
    if(skill && skill->stun && alive(target)) state_.units[target].stunned=true;
    if(skill && skill->push) push(actor,target,skill->push,out);
   }
   if(!skill) u.ki=std::min(kMaxKi,u.ki+10);
   if(skill && command.skill==0) state_.heroes[actor].practice=std::min(99,state_.heroes[actor].practice+1);
  }
 } else if(command.action==Action::Charge) {
  constexpr std::array<int,3> gains{20,30,50};
  const int previousKi=u.ki;
  u.ki=std::min(kMaxKi,u.ki+gains[u.chain]+(state_.heroes[actor].training==1?5:0));
  out.add(EventKind::Ki,actor,actor,u.ki-previousKi);
  u.chain=std::min(2,u.chain+1); out.message="기 축적 중입니다. 적에게 맞으면 집중이 끊깁니다.";
 } else if(command.action==Action::Guard) u.guard=true;
 else if(command.action==Action::Observe) {
  const int before=state_.heroes[actor].insights;
  state_.heroes[actor].insights|=1; state_.discovered|=1<<state_.units[target].kind;
  out.learned=before!=state_.heroes[actor].insights;
  out.add(EventKind::Insight,actor,target,1); out.message="동작을 발견했습니다. 도장에서 연구할 수 있습니다.";
 } else if(command.action==Action::Item) {
  --state_.medicine; auto& ally=state_.units[target]; const int heal=std::min(24,maxHealth(target)-ally.health);
  ally.health+=heal; out.add(EventKind::Heal,actor,target,heal);
 }
 resolve();
 if(state_.phase==Phase::Playing) {
  bool finished=true; for(int i=0;i<3;++i) if(alive(i) && !state_.units[i].acted) finished=false;
  if(finished) enemyTurn(out);
 }
 out.kiChange=state_.units[actor].ki-oldKi;
 if(out.chases>0) out.message="밀쳐낸 상대를 동료가 추격했습니다!";
 if(state_.phase==Phase::Victory) out.message="전투 승리! 전리품과 배움을 도장으로 가져갑니다.";
 if(state_.phase==Phase::Defeat) out.message="전열이 무너졌습니다. 출전 당시 상태로 재도전하세요.";
 return out;
}
void BattleModel::enemyTurn(Outcome& out) {
 for(int enemy=3;enemy<kUnits && state_.phase==Phase::Playing;++enemy) if(alive(enemy)) {
  auto& u=state_.units[enemy]; const auto& data=kEnemyData[u.kind];
  if(u.stunned) { u.stunned=false; continue; }
  int target=-1,best=1000;
  for(int i=0;i<3;++i) if(alive(i)) { const int score=distance(u.cell,state_.units[i].cell); if(score<best) { best=score; target=i; } }
  if(target<0) break;
  // Terrain BFS from the selected target. Allies block travel; targets cannot be entered.
  std::array<int,kCells> costs; costs.fill(-1); std::array<Cell,kCells> queue{}; int begin=0,end=0;
  queue[end++]=state_.units[target].cell; costs[index(queue[0])]=0;
  while(begin<end) { const Cell at=queue[begin++]; for(const auto d:kDirections) { const Cell next=plus(at,d); if(blocked(next) || costs[index(next)]>=0) continue; costs[index(next)]=costs[index(at)]+1; queue[end++]=next; } }
  for(int step=0;step<data.move && !attackRange(u.cell,state_.units[target].cell,data.range);++step) {
   Cell next=u.cell; int cost=costs[index(u.cell)]<0?1000:costs[index(u.cell)];
   for(const auto d:kDirections) { const Cell at=plus(u.cell,d); if(blocked(at) || occupant(at)>=0) continue; const int value=costs[index(at)]; if(value>=0 && value<cost) { cost=value; next=at; } }
   if(next==u.cell) break;
   out.add(EventKind::Move,enemy,enemy,0,u.cell,next); u.cell=next;
  }
  // Re-evaluate range after moving so a reachable ally is never ignored.
  target=-1; for(int i=0;i<3;++i) if(alive(i) && attackRange(u.cell,state_.units[i].cell,data.range)) { target=i; break; }
  if(target>=0) {
   if(u.kind==3 && u.ki<30) { u.ki+=30; out.add(EventKind::Ki,enemy,enemy,30); }
   else { damage(enemy,target,data.damage+(u.kind==3?4:0),out); if(u.kind==3 && alive(enemy)) { u.ki=0; push(enemy,target,1,out); } }
  }
  resolve();
 }
 if(state_.phase!=Phase::Playing) return;
 if(state_.round>=lesson().roundLimit) { state_.phase=Phase::Defeat; return; }
 ++state_.round;
 for(int i=0;i<3;++i) { state_.units[i].acted=false; state_.units[i].chased=false; state_.units[i].guard=false; }
}
Outcome BattleModel::endTurn() {
 Outcome out; out.message="적의 차례를 처리했습니다.";
 if(state_.phase!=Phase::Playing) return out;
 out.valid=true; enemyTurn(out); return out;
}
bool BattleModel::nextLesson() {
 if(state_.phase!=Phase::Victory || (state_.claimed&(1<<state_.lesson))) return false;
 state_.claimed|=1<<state_.lesson; state_.supplies=std::min(99,state_.supplies+4);
 state_.medicine=std::min(3,state_.medicine+1);
 if(state_.chases>0) state_.bond=std::min(5,state_.bond+1);
 for(int i=0;i<3;++i) if(state_.deployed[i]) {
  auto& p=state_.heroes[i]; const int earned=8+state_.units[i].effort+(p.gear==1?4:0);
  p.xp+=earned; while(p.xp>=20 && p.level<10) { p.xp-=20; ++p.level; }
  p.xp=std::min(19,p.xp);
 }
 state_.units={}; state_.round=1;
 if(state_.lesson+1==kLessons) state_.phase=Phase::Complete;
 else { ++state_.lesson; state_.choice=-1; state_.phase=Phase::Story; }
 return true;
}
bool BattleModel::retry() {
 if(state_.phase!=Phase::Defeat) return false;
 state_.heroes=state_.entryHeroes; state_.discovered=state_.entryDiscovered;
 state_.medicine=state_.entryMedicine; // Retry restores the entire deployment checkpoint.
 setupBattle(); return true;
}
void BattleModel::restart() { state_={}; }
bool BattleModel::validState(const BattleState& s) {
 const int phase=static_cast<int>(s.phase);
 if(phase<0 || phase>6 || s.lesson<0 || s.lesson>=kLessons || s.round<1 || s.round>kLessonData[s.lesson].roundLimit || s.choice<-1 || s.choice>2 || s.bond<0 || s.bond>5 || s.supplies<0 || s.supplies>99 || s.medicine<0 || s.medicine>3 || s.entryMedicine<0 || s.entryMedicine>3 || s.skills<0 || s.skills>1023 || (s.skills&kInitialSkills)!=kInitialSkills || s.discovered<0 || s.discovered>15 || s.entryDiscovered<0 || (s.entryDiscovered&s.discovered)!=s.entryDiscovered || s.chases<0 || s.chases>99) return false;
 const int past=(1<<s.lesson)-1;
 if(s.claimed!=(s.phase==Phase::Complete?31:past) || (s.phase==Phase::Complete && s.lesson!=4)) return false;
 if((s.phase==Phase::Story && s.choice!=-1) || (s.phase!=Phase::Story && s.choice<0)) return false;
 if(!s.deployed[0] || std::count(s.deployed.begin(),s.deployed.end(),true)<2) return false;
 int slots=0; for(int slot:s.slots) { if(slot<0 || slot>2 || (slots&(1<<slot))) return false; slots|=1<<slot; }
 for(const auto& heroes:{s.heroes,s.entryHeroes}) for(const auto& p:heroes) if(p.level<1 || p.level>10 || p.xp<0 || p.xp>19 || p.training<0 || p.training>3 || p.gear<0 || p.gear>2 || p.insights<0 || p.insights>7 || p.practice<0 || p.practice>99) return false;
 if(!battlePhase(s.phase)) return std::all_of(s.units.begin(),s.units.end(),[](const Unit& unit){ return unit==Unit{}; });
 BattleModel model; model.state_=s;
 for(int i=0;i<kUnits;++i) {
  const auto& u=s.units[i]; const bool expected=i<3?s.deployed[i]:i-3<kLessonData[s.lesson].enemyCount;
  if(u.active!=expected) return false;
  if(!u.active) { if(u!=Unit{}) return false; continue; }
  if(u.kind!=(i<3?i:kLessonData[s.lesson].kinds[i-3]) || model.blocked(u.cell) || u.health<0 || u.ki<0 || u.ki>100 || u.chain<0 || u.chain>2 || u.effort<0 || u.effort>6) return false;
  const int maximum=i<3?model.maxHealth(i):kEnemyData[u.kind].health+s.lesson*3;
  if(u.health>maximum) return false;
  for(int j=0;j<i;++j) if(u.health>0 && s.units[j].active && s.units[j].health>0 && s.units[j].cell==u.cell) return false;
 }
 bool enemies=false; for(int i=3;i<kUnits;++i) enemies|=model.alive(i);
 if(s.phase==Phase::Playing) return model.alive(0) && enemies;
 if(s.phase==Phase::Victory) return model.alive(0) && !enemies;
 return !model.alive(0) || (enemies && s.round==kLessonData[s.lesson].roundLimit);
}
bool BattleModel::save(const std::string& path) const {
 if(!validState(state_)) return false;
 const auto& s=state_; const std::string temporary=path+".tmp";
 std::ofstream out(temporary,std::ios::trunc);
 out<<"WUTEN 2\n"<<static_cast<int>(s.phase)<<' '<<s.lesson<<' '<<s.round<<' '<<s.choice<<' '<<s.bond<<' '<<s.supplies<<' '<<s.medicine<<' '<<s.entryMedicine<<' '<<s.skills<<' '<<s.discovered<<' '<<s.entryDiscovered<<' '<<s.claimed<<' '<<s.chases<<' '<<s.trained<<'\n';
 for(bool deployed:s.deployed) out<<deployed<<' ';
 for(int slot:s.slots) out<<slot<<' ';
 out<<'\n';
 for(const auto& heroes:{s.heroes,s.entryHeroes}) for(const auto& p:heroes) out<<p.level<<' '<<p.xp<<' '<<p.training<<' '<<p.gear<<' '<<p.insights<<' '<<p.practice<<'\n';
 for(const auto& u:s.units) out<<u.cell.x<<' '<<u.cell.y<<' '<<u.kind<<' '<<u.health<<' '<<u.ki<<' '<<u.chain<<' '<<u.effort<<' '<<u.active<<' '<<u.acted<<' '<<u.guard<<' '<<u.chased<<' '<<u.stunned<<'\n';
 out.flush(); if(!out) return false; out.close(); if(!out) return false;
 return std::rename(temporary.c_str(),path.c_str())==0;
}
bool BattleModel::load(const std::string& path) {
 std::ifstream in(path); std::string magic; int version=0,phase=0; BattleState candidate; auto& s=candidate;
 if(!(in>>magic>>version) || magic!="WUTEN" || version!=2) return false;
 if(!(in>>phase>>s.lesson>>s.round>>s.choice>>s.bond>>s.supplies>>s.medicine>>s.entryMedicine>>s.skills>>s.discovered>>s.entryDiscovered>>s.claimed>>s.chases>>s.trained) || phase<0 || phase>6) return false;
 for(auto& deployed:s.deployed) if(!(in>>deployed)) return false;
 for(auto& slot:s.slots) if(!(in>>slot)) return false;
 for(auto* heroes:{&s.heroes,&s.entryHeroes}) for(auto& p:*heroes) if(!(in>>p.level>>p.xp>>p.training>>p.gear>>p.insights>>p.practice)) return false;
 for(auto& u:s.units) if(!(in>>u.cell.x>>u.cell.y>>u.kind>>u.health>>u.ki>>u.chain>>u.effort>>u.active>>u.acted>>u.guard>>u.chased>>u.stunned)) return false;
 in>>std::ws; if(!in.eof()) return false;
 s.phase=static_cast<Phase>(phase); if(!validState(s)) return false; state_=candidate; return true;
}
}
