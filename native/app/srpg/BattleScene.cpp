#include "app/srpg/BattleScene.h"
#include <algorithm>
#include <cstdio>
#include "engine/Engine.h"
#include "engine/asset/AssetLoader.h"
#include "engine/graphics/SpriteBatch.h"
namespace app::srpg {
namespace {
using engine::Color; using engine::Rect; using engine::TextAlign;
constexpr Color background=Color::rgb8(20,28,34),panel=Color::rgb8(34,45,52),ink=Color::rgb8(239,236,214),muted=Color::rgb8(160,176,177),jade=Color::rgb8(94,208,172),gold=Color::rgb8(239,194,104),danger=Color::rgb8(237,112,100);
constexpr float tileSize=76,boardX=94,boardY=272;
Rect tileRect(Cell cell) { return {boardX+cell.x*tileSize,boardY+cell.y*tileSize,tileSize,tileSize}; }
const char* unitName(const BattleState& s,int i) { return i<3?kHeroData[i].name:kEnemyData[s.units[i].kind].name; }
}
void BattleScene::onEnter(engine::Engine& engine) {
 engine_=&engine;
 const auto glyphs=engine.assets().readFile("fonts/srpg-glyphs.txt");
 font_.load(engine.assets(),engine.atlas(),"srpg_font","fonts/NotoSansKR.ttf",std::string_view(reinterpret_cast<const char*>(glyphs.data()),glyphs.size()));
 sfx_.build(); savePath_=engine.dataPath()+"/wuten-v2.txt"; model_.load(savePath_); refresh();
}
void BattleScene::onExit() { save(); }
void BattleScene::save() { saved_=!savePath_.empty() && model_.save(savePath_); }
void BattleScene::invalidateInput() { hitCount_=0; pointer_=-1; pressed_=false; }
void BattleScene::refresh() {
 const auto& s=model_.state();
 if(s.phase==Phase::Playing && (!s.units[actor_].active || s.units[actor_].health==0 || s.units[actor_].acted)) {
  for(int i=0;i<3;++i) if(s.units[i].active && s.units[i].health>0 && !s.units[i].acted) { actor_=i; break; }
 }
 destination_=s.units[actor_].cell; reachable_=model_.movement(actor_);
 if(target_<0 || target_>=kUnits || !s.units[target_].active || s.units[target_].health==0) {
  target_=-1; for(int i=3;i<kUnits;++i) if(s.units[i].active && s.units[i].health>0) { target_=i; break; }
 }
 preview_=model_.preview({actor_,destination_,action_,target_,skill_});
}
void BattleScene::update(float dt) { animation_=std::max(0.0f,animation_-dt); }
Rect BattleScene::box(Rect area) const { return {left_+area.x*scale_,top_+area.y*scale_,area.w*scale_,area.h*scale_}; }
void BattleScene::rect(engine::SpriteBatch& batch,Rect area,Color color) { batch.drawRect(box(area),color); }
void BattleScene::text(engine::SpriteBatch& batch,std::string_view message,float x,float y,float size,Color color,TextAlign align) {
 font_.draw(batch,message,{left_+x*scale_,top_+y*scale_},size*scale_,color,align);
}
void BattleScene::fit(engine::SpriteBatch& batch,std::string_view message,float x,float y,float width,float size,Color color) {
 text(batch,message,x,y,std::min(size,size*width/std::max(1.0f,font_.measure(message,size))),color);
}
void BattleScene::button(engine::SpriteBatch& batch,Rect area,std::string_view title,Button action,int value,bool enabled,bool selected) {
 rect(batch,area,!enabled?Color::rgb8(43,52,57):selected?jade:panel);
 if(selected && !enabled) rect(batch,{area.x,area.y,5,area.h},jade);
 const float size=std::min(22.0f,22.0f*(area.w-14)/std::max(1.0f,font_.measure(title,22)));
 text(batch,title,area.x+area.w/2,area.y+(area.h-size)/2-2,size,enabled?(selected?background:ink):muted,TextAlign::Center);
 if(enabled && animation_<=0 && hitCount_<static_cast<int>(hits_.size())) hits_[hitCount_++]={box(area),action,value};
}
void BattleScene::render(engine::SpriteBatch& batch) {
 hitCount_=0;
 const float available=engine_->worldHeight()-engine_->safeTop()-engine_->safeBottom();
 scale_=std::min(1.0f,std::max(0.1f,available/1220.0f)); left_=(720-720*scale_)/2;
 top_=engine_->safeTop()+std::max(0.0f,(available-1220*scale_)/2);
 batch.drawRect({0,0,720,engine_->worldHeight()},background);
 text(batch,"武天 / 함께 여는 길",25,12,32,ink);
 button(batch,{584,8,111,46},"안내",Button::Help);
 const auto& s=model_.state(); char line[220];
 std::snprintf(line,sizeof(line),"%d / 5  %s",s.lesson+1,model_.lesson().name); fit(batch,line,25,70,670,27,gold);
 switch(s.phase) {
 case Phase::Story: story(batch); break;
 case Phase::Base: base(batch); break;
 case Phase::Deployment: deployment(batch); break;
 case Phase::Playing: battle(batch); break;
 default: results(batch); break;
 }
 fit(batch,saved_?notice_:"저장 실패: 다음 확정 동작에서 다시 시도합니다.",25,1160,670,18,saved_?muted:danger);
 text(batch,"스토리 → 도장 → 출전 → 전투 → 성장",360,1194,17,muted,TextAlign::Center);
 if(help_ || skillsOpen_ || confirmEnd_ || confirmRestart_) overlay(batch);
}
void BattleScene::story(engine::SpriteBatch& batch) {
 const auto& s=model_.state();
 rect(batch,{25,160,670,350},panel);
 text(batch,model_.lesson().speaker,55,197,30,jade);
 fit(batch,model_.lesson().dialogue,55,263,610,27,ink);
 text(batch,"로시",55,358,27,gold);
 text(batch,"이번에는 몸으로 배워 볼게요.",55,410,25,ink);
 text(batch,"누구와 준비할까요?",25,565,29,ink);
 button(batch,{25,628,670,80},"학선인과 함께  ·  관계 +1",Button::Story,0);
 button(batch,{25,729,670,80},"무태두에게 묻기  ·  보급 +1",Button::Story,1);
 button(batch,{25,830,670,80},"혼자 기 수련  ·  이번 출전 기 +10",Button::Story,2);
 text(batch,"선택은 관계와 준비에 반영됩니다.",25,978,23,muted);
 text(batch,"엔딩과 동료를 영구 차단하지 않습니다.",25,1023,23,muted);
 if(s.lesson==0) text(batch,"란란은 이 게임의 창작 동료입니다.",25,1080,21,muted);
}
void BattleScene::base(engine::SpriteBatch& batch) {
 const auto& s=model_.state(); char line[220];
 std::snprintf(line,sizeof(line),"무태두 도장    보급 %d    약초 %d/3    관계 %d/5",s.supplies,s.medicine,s.bond);
 fit(batch,line,25,125,670,24,jade);
 const std::array<const char*,5> tabs{"수련","기술","장비","동료","휴식"};
 for(int i=0;i<5;++i) button(batch,{25+i*136.0f,178,126,55},tabs[i],Button::Tab,i,true,tab_==i);
 for(int i=0;i<3;++i) {
  std::snprintf(line,sizeof(line),"%s Lv.%d",kHeroData[i].name,s.heroes[i].level);
  button(batch,{25+i*226.0f,250,216,55},line,Button::Hero,i,true,actor_==i);
 }
 const auto& p=s.heroes[actor_];
 if(tab_==0) {
  text(batch,"수련 방향: 출전마다 한 명, 보급 2",25,343,26,gold);
  const std::array<const char*,4> details{"체력 +8","초기 기 +10 · 축적 +5","방어 피해 -2","직접 공격 피해 +2"};
  for(int i=0;i<4;++i) {
   std::snprintf(line,sizeof(line),"%s  /  %s",kTrainingNames[i],details[i]);
   button(batch,{25,410+i*105.0f,670,78},line,Button::Train,i,!s.trained && s.supplies>=2,p.training==i);
  }
  text(batch,s.trained?"이번 도장의 수련을 마쳤습니다.":"선택한 방향은 다음에 바꿀 때까지 유지됩니다.",25,895,22,muted);
 } else if(tab_==1) {
  text(batch,"발견한 기술을 연구합니다. 보급 1",25,336,24,gold);
  for(int i=0;i<kSkills;++i) {
   const bool known=(s.skills&(1<<i))!=0;
   const char* status=known?"습득":model_.canResearch(i)?"연구 가능":"조건 미달";
   std::snprintf(line,sizeof(line),"%s · %s  [%s]",kHeroData[kSkillData[i].owner].name,kSkillData[i].name,status);
   button(batch,{25,383+i*58.0f,670,49},line,Button::Research,i,model_.canResearch(i),known);
  }
  fit(batch,"붕권: 단서 3개·발경 2회 / 기공파: 기공사 관찰",25,975,670,20,muted);
  fit(batch,"비연각: 2전 / 회선각·점혈: 3전 / 쌍룡격: 4전·관계 2",25,1015,670,20,muted);
 } else if(tab_==2) {
  text(batch,"장비는 출전 전에 자유롭게 교체합니다.",25,349,25,gold);
  const std::array<const char*,3> details{"받는 피해 -2","이동 -1 · 승리 경험 +4","방어 반격 +3"};
  for(int i=0;i<3;++i) {
   std::snprintf(line,sizeof(line),"%s  /  %s",kGearNames[i],details[i]);
   button(batch,{25,420+i*125.0f,670,88},line,Button::Gear,i,true,p.gear==i);
  }
  fit(batch,"중량 보상은 맵 승리 때 한 번만 얻습니다.",25,863,670,23,muted);
 } else if(tab_==3) {
  for(int i=0;i<3;++i) {
   std::snprintf(line,sizeof(line),"%s  /  %s",kHeroData[i].name,kHeroData[i].role);
   text(batch,line,25,359+i*153.0f,25,jade);
   std::snprintf(line,sizeof(line),"체력 %d · 이동 %d · 경험 %d/20",model_.maxHealth(i),model_.moveRange(i),s.heroes[i].xp);
   text(batch,line,25,412+i*153.0f,23,ink);
  }
  text(batch,"관계 2부터 추격 피해가 3 증가합니다.",25,879,23,gold);
  text(batch,"출전 인원은 로시를 포함해 2~3명입니다.",25,931,23,muted);
 } else {
  text(batch,"전투마다 체력과 초기 기를 회복합니다.",25,365,25,gold);
  text(batch,"휴식 준비로 약초를 하나 보충합니다.",25,427,24,ink);
  button(batch,{25,531,670,85},"약초 준비  ·  보급 1",Button::Rest,0,s.supplies>=1 && s.medicine<3,true);
  text(batch,"약초: 자신이나 인접한 동료의 체력 +24",25,695,23,ink);
  text(batch,"연구·수련과 보급을 나누어 쓰세요.",25,751,23,muted);
 }
 button(batch,{25,1065,670,70},"출전 준비",Button::Prepare,0,true,true);
}
void BattleScene::board(engine::SpriteBatch& batch,bool deploymentMode) {
 const auto& s=model_.state();
 for(int i=0;i<kCells;++i) {
  const Cell cell=cellAt(i); const Rect tile=tileRect(cell);
  Color color=(cell.x+cell.y)%2?Color::rgb8(65,77,69):Color::rgb8(71,85,75);
  if(!deploymentMode && reachable_[i]>=0) color=Color::rgb8(48,99,101);
  if(!deploymentMode && model_.threatened(cell)) color=Color::rgb8(124,66,57);
  rect(batch,{tile.x+2,tile.y+2,tile.w-4,tile.h-4},color);
  if(model_.blocked(cell)) rect(batch,{tile.x+12,tile.y+10,52,55},muted);
  if(!deploymentMode && animation_<=0 && hitCount_<static_cast<int>(hits_.size())) hits_[hitCount_++]={box(tile),Button::Tile,i};
 }
 auto figure=[&](Cell cell,int id,const char* name,int health,int maximum,bool dim) {
  Rect tile=tileRect(cell);
  if(!deploymentMode && animation_>0) {
   for(int e=0;e<last_.count;++e) { const auto& event=last_.events[e];
    if((event.kind==EventKind::Move || event.kind==EventKind::Push) && event.target==id) {
     const auto start=tileRect(event.from); const float t=1-animation_/0.55f; const float ease=1-(1-t)*(1-t)*(1-t);
     tile.x=start.x+(tile.x-start.x)*ease; tile.y=start.y+(tile.y-start.y)*ease; break;
    }
   }
  }
  const Color robe=id==0?jade:id==1?gold:id==2?Color::rgb8(112,174,244):danger;
  rect(batch,{tile.x+16,tile.y+57,46,8},background);
  rect(batch,{tile.x+25,tile.y+24,27,34},dim?muted:robe);
  rect(batch,{tile.x+28,tile.y+10,22,21},Color::rgb8(226,184,141));
  rect(batch,{tile.x+26,tile.y+7,26,8},background);
  text(batch,name,tile.x+38,tile.y+34,15,background,TextAlign::Center);
  rect(batch,{tile.x+10,tile.y+67,56,5},background);
  rect(batch,{tile.x+10,tile.y+67,56.0f*health/std::max(1,maximum),5},robe);
 };
 if(deploymentMode) {
  for(int i=0;i<model_.lesson().enemyCount;++i) figure(model_.lesson().enemies[i],i+3,kEnemyData[model_.lesson().kinds[i]].name,1,1,false);
  for(int slot=0;slot<3;++slot) {
   const Cell at=model_.lesson().starts[slot];
   for(int i=0;i<3;++i) if(s.slots[i]==slot && s.deployed[i]) figure(at,i,kHeroData[i].name,1,1,i!=actor_);
   if(animation_<=0) hits_[hitCount_++]={box(tileRect(at)),Button::Slot,slot};
  }
 } else {
  for(int i=0;i<kUnits;++i) if(s.units[i].active && s.units[i].health>0) {
   const auto& u=s.units[i]; const int maximum=i<3?model_.maxHealth(i):kEnemyData[u.kind].health+s.lesson*3;
   figure(u.cell,i,unitName(s,i),u.health,maximum,i<3 && u.acted);
  }
  if(animation_>0) {
   std::array<int,kUnits> rows{};
   for(int e=0;e<last_.count;++e) { const auto& event=last_.events[e];
    if(event.target<0 || event.target>=kUnits || !inside(s.units[event.target].cell)) continue;
    if(event.kind!=EventKind::Hit && event.kind!=EventKind::Chase && event.kind!=EventKind::Collision && event.kind!=EventKind::Heal) continue;
    const auto at=tileRect(s.units[event.target].cell); char label[50];
    const char* prefix=event.kind==EventKind::Chase?"추격 ":event.kind==EventKind::Collision?"충돌 ":event.kind==EventKind::Heal?"회복 ":"-";
    std::snprintf(label,sizeof(label),"%s%d",prefix,event.amount);
    text(batch,label,at.x+38,at.y-12-rows[event.target]++*21-(1-animation_/0.55f)*20,19,event.kind==EventKind::Heal?jade:gold,TextAlign::Center);
   }
  }
  if(inside(destination_)) {
   const auto tile=tileRect(destination_);
   rect(batch,{tile.x+2,tile.y+2,72,3},jade); rect(batch,{tile.x+2,tile.y+71,72,3},jade);
   if(destination_!=s.units[actor_].cell) text(batch,"이동",tile.x+38,tile.y+23,25,jade,TextAlign::Center);
  }
  if(target_>=0 && s.units[target_].health>0) {
   const auto tile=tileRect(s.units[target_].cell); rect(batch,{tile.x+2,tile.y+2,3,72},gold); rect(batch,{tile.x+71,tile.y+2,3,72},gold);
  }
 }
}
void BattleScene::deployment(engine::SpriteBatch& batch) {
 const auto& s=model_.state();
 text(batch,"인물을 고른 뒤 시작 칸을 눌러 자리를 바꿉니다.",25,125,22,jade);
 for(int i=0;i<3;++i) button(batch,{25+i*226.0f,180,216,57},kHeroData[i].name,Button::Hero,i,true,actor_==i);
 board(batch,true);
 for(int i=0;i<3;++i) button(batch,{25+i*226.0f,838,216,62},s.deployed[i]?"출전 중":"대기 중",Button::Toggle,i,i!=0,s.deployed[i]);
 fit(batch,model_.lesson().briefing,25,951,670,24,gold);
 button(batch,{25,997,210,54},"도장으로",Button::Base);
 text(batch,"로시 포함 최소 두 명 출전",263,1010,22,muted);
 button(batch,{25,1070,670,70},"전투 시작",Button::Start,0,true,true);
}
void BattleScene::battle(engine::SpriteBatch& batch) {
 const auto& s=model_.state(); char line[240];
 std::snprintf(line,sizeof(line),"아군 차례  %d/%d 턴   ·   약초 %d",s.round,model_.lesson().roundLimit,s.medicine);
 text(batch,line,25,119,24,jade);
 for(int i=0;i<3;++i) {
  const auto& u=s.units[i]; std::snprintf(line,sizeof(line),"%s %d/%d 기%d%s",kHeroData[i].name,u.health,model_.maxHealth(i),u.ki,u.acted?" 완료":"");
  button(batch,{25+i*226.0f,166,216,63},line,Button::Hero,i,u.active && u.health>0 && !u.acted,actor_==i);
 }
 text(batch,"파랑: 이동  /  빨강: 적의 현재 사거리  /  금색: 대상",360,242,18,muted,TextAlign::Center);
 board(batch);
 if(target_>=0) {
  std::snprintf(line,sizeof(line),"대상 %s · 체력 %d%s",unitName(s,target_),s.units[target_].health,s.units[target_].stunned?" · 점혈":"");
  text(batch,line,25,822,21,gold);
 }
 if(preview_.valid) {
  std::snprintf(line,sizeof(line),"예상 피해 %d / 아군 피해 %d / 기 %+d / 추격 %d",preview_.dealt,preview_.received,preview_.kiChange,preview_.chases);
  fit(batch,line,25,866,670,21,ink);
 } else fit(batch,preview_.message,25,866,670,21,danger);
 const std::array<const char*,7> actions{"타격","기술","방어","기 모음","관찰","약초","대기"};
 for(int i=0;i<7;++i) button(batch,{25+i*97.0f,909,89,54},actions[i],Button::Action,i,true,static_cast<int>(action_)==i);
 if(action_==Action::Skill) {
  std::snprintf(line,sizeof(line),"%s  ·  기 %d  ·  사거리 %d",kSkillData[skill_].name,kSkillData[skill_].cost,kSkillData[skill_].range);
  button(batch,{25,976,465,54},line,Button::Skill,-1);
 } else fit(batch,"빈 칸은 이동, 상대 칸은 대상 선택입니다.",25,990,460,19,muted);
 button(batch,{505,976,190,54},"아군 턴 종료",Button::EndTurn);
 button(batch,{25,1050,670,77},"이동 + 행동 확정",Button::Commit,0,preview_.valid,true);
 if(animation_>0) {
  char effect[100]; std::snprintf(effect,sizeof(effect),"타격 %d  ·  추격 %d  ·  받은 피해 %d",last_.dealt,last_.chases,last_.received);
  rect(batch,{108,490,504,78},panel); text(batch,effect,360,514,24,gold,TextAlign::Center);
 }
}
void BattleScene::results(engine::SpriteBatch& batch) {
 const auto& s=model_.state(); const bool victory=s.phase==Phase::Victory,complete=s.phase==Phase::Complete;
 rect(batch,{25,170,670,840},panel);
 text(batch,complete?"함께 여는 길 · 1장 완료":victory?"전투 승리":"다시 전열을 가다듬자",360,215,31,gold,TextAlign::Center);
 if(victory) {
  text(batch,"전리품: 보급 4 · 약초 1",65,317,27,jade);
  text(batch,"출전한 동료: 경험 8 + 행동 경험",65,385,24,ink);
  text(batch,"중량 장비 경험 +4 · 추격 시 관계 +1",65,445,24,ink);
  text(batch,"성장과 발견한 기술은 다음 이야기로 이어집니다.",65,546,22,muted);
  button(batch,{65,698,590,90},"보상 받기 · 다음 이야기",Button::Next,0,true,true);
 } else if(complete) {
  text(batch,"다섯 번의 전투를 세 동료가 함께 마쳤습니다.",60,332,24,ink);
  text(batch,"무태두: 이제 서로에게도 배울 수 있겠구나.",60,401,24,jade);
  text(batch,"전체 성장기와 손오공과의 만남은 다음 이야기입니다.",60,511,21,muted);
  button(batch,{65,733,590,80},"새 여정 시작",Button::Restart,0,true,true);
 } else {
  text(batch,"로시가 쓰러졌거나 제한 턴이 끝났습니다.",60,335,24,ink);
  text(batch,"출전 당시의 배움과 상태로 다시 시작합니다.",60,405,23,muted);
  button(batch,{65,632,590,80},"이번 전투 재도전",Button::Retry,0,true,true);
  button(batch,{65,755,590,80},"도장에서 다시 정비",Button::Base);
 }
 text(batch,"이동 → 밀쳐내기 → 추격 → 기 활용",360,1055,26,jade,TextAlign::Center);
}
void BattleScene::overlay(engine::SpriteBatch& batch) {
 hitCount_=0; batch.drawRect({0,0,720,engine_->worldHeight()},background.withAlpha(0.96f));
 rect(batch,{25,150,670,940},panel);
 if(confirmRestart_ || confirmEnd_) {
  text(batch,confirmRestart_?"진행을 지우고 새 여정을 시작할까요?":"행동하지 않은 동료가 있어도 턴을 마칠까요?",360,315,24,gold,TextAlign::Center);
  button(batch,{65,540,590,82},"확정",confirmRestart_?Button::RestartConfirm:Button::ConfirmEnd,0,true,true);
  button(batch,{65,658,590,82},"돌아가기",Button::Close);
 } else if(skillsOpen_) {
  text(batch,"사용할 기술",65,190,30,gold); int row=0; char line[180];
  for(int i=0;i<kSkills;++i) if(kSkillData[i].owner==actor_) {
   const auto& skill=kSkillData[i]; const bool known=(model_.state().skills&(1<<i))!=0;
   std::snprintf(line,sizeof(line),"%s  기%d · 거리%d%s",skill.name,skill.cost,skill.range,known?"":" · 미습득");
   button(batch,{65,278+row++*128.0f,590,96},line,Button::Skill,i,known,skill_==i);
  }
  text(batch,"회복 기술은 동료 칸을 대상으로 선택합니다.",65,879,22,muted);
  button(batch,{65,965,590,70},"돌아가기",Button::Close);
 } else {
  text(batch,"위치가 연계를 만듭니다",65,189,30,gold);
  const std::array<const char*,14> lines{
   "아군은 각자 이동과 행동을 한 번 합니다.","마지막 아군이 행동하면 적 차례가 진행됩니다.",
   "로시가 쓰러지거나 제한 턴을 넘기면 패배합니다.","적을 모두 쓰러뜨리면 보상을 받고 진행합니다.",
   "발경으로 적을 동료 옆에 밀면 추격합니다.","추격은 동료마다 한 라운드에 한 번입니다.",
   "벽과 적 충돌은 추가 피해를 줍니다.","일반 타격 +10, 사거리 이탈 +15, 피격 +5 기.",
   "기 모음은 제자리 연속 20 → 30 → 50입니다.","모으다 맞으면 추가 피해 6 · 기 손실 10.",
   "관찰·사거리 이탈·방어 피격은 단서가 됩니다.","발경 2회와 단서 3개를 얻고 도장에서 붕권 연구.",
   "약초·응급처치는 동료 칸을 누르세요.","빨강은 현재 사거리이며 적은 이동 후 공격합니다."};
  for(int i=0;i<static_cast<int>(lines.size());++i) fit(batch,lines[i],55,258+i*45.0f,610,21,ink);
  button(batch,{65,954,590,80},"돌아가기",Button::Close,0,true,true);
 }
}
void BattleScene::finish(const Outcome& out) {
 if(!out.valid) { notice_=out.message; return; }
 last_=out; notice_=out.message; animation_=0.55f;
 if(sound_) engine_->mixer().play(out.chases?sfx_.fanfare:out.dealt?sfx_.tap:sfx_.click);
 engine_->haptics().light(); action_=Action::Strike; refresh(); save();
}
void BattleScene::act(Button buttonValue,int value) {
 invalidateInput();
 if(animation_>0) return;
 if(buttonValue==Button::Close) { help_=skillsOpen_=confirmEnd_=confirmRestart_=false; return; }
 if(buttonValue==Button::Help) { help_=true; return; }
 if(help_) return;
 if(confirmEnd_) { if(buttonValue==Button::ConfirmEnd) { confirmEnd_=false; finish(model_.endTurn()); } return; }
 if(confirmRestart_) { if(buttonValue==Button::RestartConfirm) { confirmRestart_=false; model_.restart(); actor_=0; refresh(); save(); } return; }
 if(skillsOpen_) { if(buttonValue==Button::Skill && value>=0 && value<kSkills && kSkillData[value].owner==actor_ && (model_.state().skills&(1<<value))) { skill_=value; action_=Action::Skill; skillsOpen_=false; if(kSkillData[value].shape==SkillShape::Heal) target_=actor_; preview_=model_.preview({actor_,destination_,action_,target_,skill_}); } return; }
 const auto phase=model_.state().phase; bool changed=false;
 if(buttonValue==Button::Hero && value>=0 && value<3) { actor_=value; action_=Action::Strike; refresh(); return; }
 switch(phase) {
 case Phase::Story: if(buttonValue==Button::Story) changed=model_.chooseStory(value); break;
 case Phase::Base:
  if(buttonValue==Button::Tab) tab_=value;
  if(buttonValue==Button::Train) changed=model_.train(actor_,value);
  if(buttonValue==Button::Gear) changed=model_.equip(actor_,value);
  if(buttonValue==Button::Research) changed=model_.research(value);
  if(buttonValue==Button::Rest) changed=model_.rest();
  if(buttonValue==Button::Prepare) changed=model_.prepare();
  break;
 case Phase::Deployment:
  if(buttonValue==Button::Base) changed=model_.returnToBase();
  if(buttonValue==Button::Toggle) changed=model_.toggleDeploy(value);
  if(buttonValue==Button::Slot) changed=model_.place(actor_,value);
  if(buttonValue==Button::Start) { changed=model_.startBattle(); action_=Action::Strike; }
  break;
 case Phase::Playing:
  if(buttonValue==Button::Tile && value>=0 && value<kCells) {
   const Cell cell=cellAt(value); const int who=model_.occupant(cell);
   if(who>=0) { if(who==actor_) destination_=cell; if(who>=3 || action_==Action::Item || (action_==Action::Skill && kSkillData[skill_].shape==SkillShape::Heal)) target_=who; }
   else if(reachable_[value]>=0) destination_=cell;
  }
  if(buttonValue==Button::Action && value>=0 && value<=6) {
   action_=static_cast<Action>(value);
   if(action_==Action::Skill) skillsOpen_=true;
   if(action_==Action::Item) target_=actor_;
  }
  if(buttonValue==Button::Skill) skillsOpen_=true;
  if(buttonValue==Button::EndTurn) confirmEnd_=true;
  if(buttonValue==Button::Commit) { finish(model_.execute({actor_,destination_,action_,target_,skill_})); return; }
  preview_=model_.preview({actor_,destination_,action_,target_,skill_}); break;
 case Phase::Victory: if(buttonValue==Button::Next) changed=model_.nextLesson(); break;
 case Phase::Defeat: if(buttonValue==Button::Retry) changed=model_.retry(); if(buttonValue==Button::Base) changed=model_.returnToBase(); break;
 case Phase::Complete: if(buttonValue==Button::Restart) confirmRestart_=true; break;
 }
 if(changed) { notice_="준비와 진행을 저장했습니다."; refresh(); save(); }
}
void BattleScene::onTouch(const engine::TouchEvent& event) {
 using Touch=engine::TouchEvent::Phase;
 if(event.phase==Touch::Down && pointer_==-1) {
  pointer_=event.pointerId; pressed_=false;
  for(int i=hitCount_-1;i>=0;--i) if(hits_[i].rect.contains(event.position)) { pressedHit_=hits_[i]; pressed_=true; break; }
 } else if(event.pointerId==pointer_ && (event.phase==Touch::Up || event.phase==Touch::Cancel)) {
  if(event.phase==Touch::Up && pressed_ && pressedHit_.rect.contains(event.position)) {
   for(int i=0;i<hitCount_;++i) if(hits_[i].button==pressedHit_.button && hits_[i].value==pressedHit_.value && hits_[i].rect.contains(event.position)) { act(pressedHit_.button,pressedHit_.value); break; }
  }
  pointer_=-1; pressed_=false;
 }
}
bool BattleScene::onBack() {
 invalidateInput();
 if(help_ || skillsOpen_ || confirmEnd_ || confirmRestart_) { help_=skillsOpen_=confirmEnd_=confirmRestart_=false; return true; }
 if(model_.state().phase==Phase::Playing && destination_!=model_.state().units[actor_].cell) { refresh(); return true; }
 if(model_.state().phase==Phase::Deployment) { model_.returnToBase(); refresh(); save(); return true; }
 save(); return false;
}
}
