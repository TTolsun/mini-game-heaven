#include "app/srpg/BattleScene.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include "engine/Engine.h"
#include "engine/asset/AssetLoader.h"
#include "engine/graphics/SpriteBatch.h"
namespace app::srpg {
namespace {
using engine::Color; using engine::Rect; using engine::TextAlign; using engine::Vec2;
constexpr Color background=Color::rgb8(21,33,38),panel=Color::rgb8(35,49,52),ink=Color::rgb8(244,233,203),muted=Color::rgb8(162,177,168),jade=Color::rgb8(118,197,167),gold=Color::rgb8(236,193,118),danger=Color::rgb8(226,127,108);
constexpr float tileSize=64,boardX=144,boardY=208;
Rect tileRect(Cell cell) { return {boardX+cell.x*tileSize,boardY+cell.y*tileSize,tileSize,tileSize}; }
Vec2 feet(Vec2 cell) { return {boardX+cell.x*tileSize+32,boardY+cell.y*tileSize+55}; }
int heroPortrait(int id) { return id==2?3:id; }
const char* unitName(const BattleState& s,int i) { return i<3?kHeroData[i].name:kEnemyData[s.units[i].kind].name; }
}
void BattleScene::onEnter(engine::Engine& engine) {
 engine_=&engine;
 const auto glyphs=engine.assets().readFile("fonts/srpg-glyphs.txt");
 font_.load(engine.assets(),engine.atlas(),"srpg_font","fonts/NotoSansKR.ttf",std::string_view(reinterpret_cast<const char*>(glyphs.data()),glyphs.size()));
 if(!art_.load(engine.assets()))notice_="일부 그림을 불러오지 못했습니다.";
 sfx_.build(); savePath_=engine.dataPath()+"/wuten-v2.txt"; model_.load(savePath_);
 std::ifstream settings(engine.dataPath()+"/visual-settings.txt"); settings>>reducedEffects_;
 refresh();
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
void BattleScene::update(float dt) {
 clock_+=dt; animation_=std::max(0.0f,animation_-dt);
 if(animation_<=0)return;
 frame_=timelineFrame(before_,last_,duration_-animation_);
 if(frame_.event>=0 && frame_.progress>=0.45f && impactEvent_!=frame_.event) {
  const auto kind=last_.events[frame_.event].kind;
  if(kind==EventKind::Hit) {
   impactEvent_=frame_.event;
   if(!reducedEffects_) { engine_->hitStop(0.05f); engine_->addTrauma(last_.events[frame_.event].amount>=18?0.36f:0.23f); engine_->haptics().light(); }
   if(sound_)engine_->mixer().play(sfx_.tap);
  }
 }
}
Rect BattleScene::box(Rect area) const { return {left_+area.x*scale_,top_+area.y*scale_,area.w*scale_,area.h*scale_}; }
void BattleScene::rect(engine::SpriteBatch& batch,Rect area,Color color) { batch.drawRect(box(area),color); }
void BattleScene::text(engine::SpriteBatch& batch,std::string_view message,float x,float y,float size,Color color,TextAlign align) {
 font_.draw(batch,message,{left_+x*scale_,top_+y*scale_},size*scale_,color,align);
}
void BattleScene::fit(engine::SpriteBatch& batch,std::string_view message,float x,float y,float width,float size,Color color) {
 text(batch,message,x,y,std::min(size,size*width/std::max(1.0f,font_.measure(message,size))),color);
}
void BattleScene::portrait(engine::SpriteBatch& batch,int person,Rect r) {
 rect(batch,{r.x-2,r.y-2,r.w+4,r.h+4},Color::rgb8(147,128,85)); art_.portrait(batch,person,box(r));
}
void BattleScene::button(engine::SpriteBatch& batch,Rect area,std::string_view title,Button action,int value,bool enabled,bool selected) {
 enabled=enabled && animation_<=0;
 rect(batch,area,selected?Color::rgb8(85,113,96):panel);
 rect(batch,{area.x,area.y,area.w,1},selected?gold:Color::rgb8(68,83,78));
 rect(batch,{area.x,area.y+area.h-1,area.w,1},Color::rgb8(17,30,34));
 if(selected)rect(batch,{area.x,area.y,3,area.h},gold);
 const float size=std::min(21.0f,21.0f*(area.w-16)/std::max(1.0f,font_.measure(title,21)));
 text(batch,title,area.x+area.w/2,area.y+(area.h-size)/2-2,size,enabled?ink:muted.withAlpha(0.55f),TextAlign::Center);
 if(enabled && animation_<=0 && hitCount_<static_cast<int>(hits_.size()))hits_[hitCount_++]={box(area),action,value};
}
void BattleScene::render(engine::SpriteBatch& batch) {
 hitCount_=0; scale_=1;left_=top_=0;
 batch.drawRect({0,0,1280,720},background);
 rect(batch,{16,14,4,47},gold); text(batch,"武天",32,12,38,ink); text(batch,"함께 여는 길",127,28,20,muted);
 const auto& s=model_.state(); char line[180];
 std::snprintf(line,sizeof(line),"제%d전  /  %s",s.lesson+1,model_.lesson().name); fit(batch,line,324,28,575,23,gold);
 button(batch,{1032,16,142,44},reducedEffects_?"연출: 절제":"연출: 기본",Button::Effects);
 button(batch,{1186,16,78,44},"안내",Button::Help);
 rect(batch,{16,71,1248,1},Color::rgb8(86,85,67));
 if(animation_>0) battle(batch);
 else switch(s.phase) {
 case Phase::Story: story(batch);break;
 case Phase::Base: base(batch);break;
 case Phase::Deployment: deployment(batch);break;
 case Phase::Playing: battle(batch);break;
 default: results(batch);break;
 }
 rect(batch,{0,688,1280,32},background);
 fit(batch,saved_?notice_:"저장 실패: 다음 확정 동작에서 다시 시도합니다.",24,696,1000,16,saved_?muted:danger);
 text(batch,"스토리 · 도장 · 출전 · 성장",1255,696,15,muted,TextAlign::Right);
 if(help_ || skillsOpen_ || confirmEnd_ || confirmRestart_)overlay(batch);
}
void BattleScene::scenery(engine::SpriteBatch& batch) {
 for(int y=0;y<9;++y)for(int x=0;x<12;++x) {
  int kind=(x*3+y*7)%4;
  if(x==10 || x==11)kind=8+(y%4);
  else if(x==9)kind=18;
  else if((x==0 || x==1) && y%3!=1)kind=13+(y%2);
  else if(y==0 || (x==8 && y%3==2))kind=15;
  art_.tile(batch,kind,box({16+x*64.0f,96+y*64.0f,64,64}));
 }
 // Crossing and dojo sit outside the playable courtyard.
 for(int i=0;i<11;++i) {
  rect(batch,{650+i*12.0f,350,10,68},Color::rgb8(151+i%2*17,124+i%2*12,81));
  rect(batch,{650+i*12.0f,348,2,73},Color::rgb8(81,79,60));
 }
 rect(batch,{650,357,134,4},gold.withAlpha(0.55f)); rect(batch,{650,405,134,4},gold.withAlpha(0.55f));
 art_.dojo(batch,box({229,78,288,126}));
 art_.unit(batch,3,0,Pose::Idle,clock_,{620,181},1.1f);
 rect(batch,{boardX-5,boardY-5,458,458},Color::rgb8(62,75,58));
 for(int y=0;y<7;++y)for(int x=0;x<7;++x)art_.tile(batch,4+(x*3+y*7)%4,box(tileRect({x,y})));
}
void BattleScene::story(engine::SpriteBatch& batch) {
 scenery(batch);
 for(int i=0;i<3;++i)art_.unit(batch,i,0,Pose::Idle,clock_+i,{300+i*76.0f,395},1.65f);
 rect(batch,{803,96,461,570},panel);
 text(batch,"어린 무도가들의 첫걸음",831,117,25,gold);
 const int speaker=model_.state().lesson==1?1:model_.state().lesson==2?3:model_.state().lesson==3?0:2;
 portrait(batch,speaker,{833,166,160,160}); portrait(batch,0,{1058,166,160,160});
 text(batch,model_.lesson().speaker,831,351,22,jade);
 fit(batch,model_.lesson().dialogue,831,387,401,23,ink);
 text(batch,"로시  “이번에는 몸으로 배워 볼게요.”",831,427,20,muted);
 button(batch,{827,479,413,49},"학선인과 함께  ·  관계 +1",Button::Story,0);
 button(batch,{827,542,413,49},"무태두에게 묻기  ·  보급 +1",Button::Story,1);
 button(batch,{827,605,413,49},"혼자 수련  ·  출전 기 +10",Button::Story,2);
 rect(batch,{41,488,716,151},background.withAlpha(0.88f));
 text(batch,"무태두 도장",65,508,32,gold);
 text(batch,"곁에 선 사람에게 배우는 무술.",65,557,27,ink);
 text(batch,"소년 로시가 스승이 되어 가는 여정의 시작입니다.",65,606,21,muted);
}
void BattleScene::base(engine::SpriteBatch& batch) {
 const auto& s=model_.state();const auto& p=s.heroes[actor_];char line[180];
 portrait(batch,heroPortrait(actor_),{32,105,240,240});
 std::snprintf(line,sizeof(line),"%s  Lv.%d",kHeroData[actor_].name,p.level);text(batch,line,32,368,31,gold);
 text(batch,kHeroData[actor_].role,32,413,22,jade);
 std::snprintf(line,sizeof(line),"체력 %d · 이동 %d",model_.maxHealth(actor_),model_.moveRange(actor_));text(batch,line,32,459,22,ink);
 std::snprintf(line,sizeof(line),"보급 %d  ·  약초 %d/3",s.supplies,s.medicine);text(batch,line,32,520,22,muted);
 std::snprintf(line,sizeof(line),"관계 %d/5  ·  경험 %d/20",s.bond,p.xp);text(batch,line,32,557,21,muted);
 for(int i=0;i<3;++i)button(batch,{32+i*88.0f,610,80,51},kHeroData[i].name,Button::Hero,i,true,actor_==i);
 rect(batch,{307,96,957,575},panel.withAlpha(0.55f));
 const std::array<const char*,5> tabs{"수련","기술","장비","동료","휴식"};
 for(int i=0;i<5;++i)button(batch,{328+i*181.0f,111,168,50},tabs[i],Button::Tab,i,true,tab_==i);
 if(tab_==0) {
  text(batch,"수련 방향  /  출전마다 한 명 · 보급 2",337,190,25,gold);
  const std::array<const char*,4> details{"체력 +8","초기 기 +10 · 축적 +5","방어 피해 -2","직접 공격 피해 +2"};
  for(int i=0;i<4;++i) { std::snprintf(line,sizeof(line),"%s  /  %s",kTrainingNames[i],details[i]);button(batch,{336,245+i*76.0f,894,60},line,Button::Train,i,!s.trained && s.supplies>=2,p.training==i); }
  text(batch,s.trained?"이번 도장의 수련을 마쳤습니다.":"선택한 방향은 다음에 바꿀 때까지 유지됩니다.",337,566,21,muted);
 } else if(tab_==1) {
  text(batch,"발견한 기술 연구  /  보급 1",337,183,24,gold);
  for(int i=0;i<kSkills;++i) {
   const bool known=(s.skills&(1<<i))!=0;
   std::snprintf(line,sizeof(line),"%s · %s  [%s]",kHeroData[kSkillData[i].owner].name,kSkillData[i].name,known?"습득":model_.canResearch(i)?"연구 가능":"조건 미달");
   button(batch,{336+(i/5)*453.0f,229+(i%5)*59.0f,440,49},line,Button::Research,i,model_.canResearch(i),known);
  }
  text(batch,"붕권: 단서 3개·발경 2회 / 기공파: 기공사 관찰",337,542,21,muted);
  text(batch,"비연각: 2전 / 회선각·점혈: 3전 / 쌍룡격: 4전·관계 2",337,576,20,muted);
 } else if(tab_==2) {
  text(batch,"장비  /  출전 전에 자유롭게 교체",337,196,25,gold);
  const std::array<const char*,3> details{"받는 피해 -2","이동 -1 · 승리 경험 +4","방어 반격 +3"};
  for(int i=0;i<3;++i) { std::snprintf(line,sizeof(line),"%s  /  %s",kGearNames[i],details[i]);button(batch,{336,270+i*88.0f,894,67},line,Button::Gear,i,true,p.gear==i); }
  text(batch,"중량 보상은 맵 승리 때 한 번만 얻습니다.",337,570,22,muted);
 } else if(tab_==3) {
  for(int i=0;i<3;++i) {
   portrait(batch,heroPortrait(i),{340,196+i*124.0f,98,98});
   std::snprintf(line,sizeof(line),"%s / %s",kHeroData[i].name,kHeroData[i].role);text(batch,line,468,205+i*124.0f,24,jade);
   std::snprintf(line,sizeof(line),"체력 %d · 이동 %d · 경험 %d/20",model_.maxHealth(i),model_.moveRange(i),s.heroes[i].xp);text(batch,line,468,249+i*124.0f,22,ink);
  }
  text(batch,"관계 2부터 추격 피해 +3. 로시를 포함해 2~3명이 출전합니다.",337,581,20,muted);
 } else {
  text(batch,"다음 전투를 위한 준비",337,198,28,gold);
  text(batch,"전투마다 체력과 초기 기를 회복합니다.",337,270,25,ink);
  text(batch,"휴식 준비로 약초를 하나 보충합니다.",337,321,23,muted);
  button(batch,{337,392,880,68},"약초 준비 · 보급 1",Button::Rest,0,s.supplies>=1 && s.medicine<3,true);
  text(batch,"약초: 자신이나 인접한 동료의 체력 +24",337,511,23,jade);
  text(batch,"연구·수련과 보급을 나누어 쓰세요.",337,557,22,muted);
 }
 button(batch,{995,609,236,48},"출전 준비 →",Button::Prepare,0,true,true);
}
void BattleScene::board(engine::SpriteBatch& batch,bool deploymentMode) {
 const auto& s=model_.state();scenery(batch);
 for(int i=0;i<kCells;++i) {
  const Cell cell=cellAt(i);const Rect r=tileRect(cell);
  art_.tile(batch,4+(cell.x*3+cell.y*7)%4,box(r));
  rect(batch,{r.x,r.y,64,1},Color::rgb8(102,104,79).withAlpha(0.65f));rect(batch,{r.x,r.y,1,64},Color::rgb8(102,104,79).withAlpha(0.65f));
  if(!deploymentMode && animation_<=0) {
   if(reachable_[i]>=0)rect(batch,{r.x+2,r.y+2,60,60},Color::rgb8(60,157,174).withAlpha(0.27f));
   if(model_.threatened(cell)) { rect(batch,{r.x+4,r.y+53,56,5},danger.withAlpha(0.75f));rect(batch,{r.x+4,r.y+8,3,44},danger.withAlpha(0.48f)); }
   if(hitCount_<static_cast<int>(hits_.size()))hits_[hitCount_++]={box(r),Button::Tile,i};
  }
 }
 if(!deploymentMode && animation_<=0) {
  if(inside(destination_)) { const auto r=tileRect(destination_);rect(batch,{r.x+1,r.y+1,62,3},jade);rect(batch,{r.x+1,r.y+61,62,3},jade); }
  if(target_>=0 && s.units[target_].health>0) { const auto r=tileRect(s.units[target_].cell);rect(batch,{r.x+1,r.y+1,3,62},gold);rect(batch,{r.x+61,r.y+1,3,62},gold); }
 }
 struct DrawUnit { Vec2 cell;int id,kind,health,maximum,direction;bool dim; };
 std::array<DrawUnit,kUnits> draw{};int count=0;
 if(deploymentMode) {
  for(int i=0;i<model_.lesson().enemyCount;++i) { const auto at=model_.lesson().enemies[i];draw[count++]={{static_cast<float>(at.x),static_cast<float>(at.y)},i+3,model_.lesson().kinds[i],1,1,0,false}; }
  for(int slot=0;slot<3;++slot) {
   const auto at=model_.lesson().starts[slot];
   for(int i=0;i<3;++i)if(s.slots[i]==slot && s.deployed[i])draw[count++]={{static_cast<float>(at.x),static_cast<float>(at.y)},i,0,1,1,0,i!=actor_};
   hits_[hitCount_++]={box(tileRect(at)),Button::Slot,slot};
  }
 } else for(int i=0;i<kUnits;++i) {
  const auto& u=s.units[i];const auto v=animation_>0?frame_.units[i]:VisualUnit{{static_cast<float>(u.cell.x),static_cast<float>(u.cell.y)},u.health,i<3?2:0,u.active};
  const bool dying=animation_>0 && frame_.event>=0 && last_.events[frame_.event].kind==EventKind::Hit && last_.events[frame_.event].target==i && frame_.progress<0.95f;
  if(v.active && (v.health>0 || dying))draw[count++]={v.cell,i,u.kind,v.health,i<3?model_.maxHealth(i):kEnemyData[u.kind].health+s.lesson*3,v.direction,i<3 && u.acted && animation_<=0};
 }
 std::sort(draw.begin(),draw.begin()+count,[](const DrawUnit& a,const DrawUnit& b){return a.cell.y<b.cell.y;});
 // Row sorting lets tall unit heads overlap the tile above without covering nearer units.
 for(int row=0;row<kRows;++row) {
  for(int x=0;x<kColumns;++x)if(model_.blocked({x,row}))art_.tile(batch,12,box(tileRect({x,row})));
  for(int n=0;n<count;++n) {
   const auto& u=draw[n];if(std::clamp(static_cast<int>(std::floor(u.cell.y)),0,6)!=row)continue;
   Vec2 at=feet(u.cell);Pose pose=Pose::Idle;float time=clock_;Color tint=u.dim?Color::rgb8(150,162,155):Color::white();
   if(animation_>0 && frame_.event>=0) {
    const auto& e=last_.events[frame_.event];time=frame_.progress*0.40f;
    if(e.target==u.id && e.kind==EventKind::Move)pose=Pose::Walk;
    if(e.actor==u.id && (e.kind==EventKind::Hit || e.kind==EventKind::Chase))pose=e.actor==visualCommand_.actor && visualCommand_.action==Action::Skill?Pose::Skill:Pose::Attack;
    if(e.actor==u.id && (e.kind==EventKind::Ki || e.kind==EventKind::Heal))pose=Pose::Skill;
    if(e.actor==u.id && e.actor==visualCommand_.actor && visualCommand_.action==Action::Skill && (visualCommand_.skill==2 || visualCommand_.skill==4) && e.kind==EventKind::Hit)pose=Pose::Kick;
    if(e.target==u.id && (e.kind==EventKind::Push || (e.kind==EventKind::Hit && frame_.progress>=0.45f)))pose=Pose::Hit;
    if(!reducedEffects_ && e.target==u.id && e.kind==EventKind::Hit && frame_.progress>=0.45f && frame_.progress<0.61f)tint=Color::rgb8(255,180,144);
    if(u.health==0)tint.a=std::clamp((0.95f-frame_.progress)/0.5f,0.0f,1.0f);
    if(e.actor==u.id && e.kind==EventKind::Hit && !reducedEffects_) { const auto to=feet(frame_.units[e.target].cell);const float lunge=std::sin(frame_.progress*3.14159f)*0.18f;at.x+=(to.x-at.x)*lunge;at.y+=(to.y-at.y)*lunge; }
   }
   rect(batch,{at.x-19,at.y-6,38,8},background.withAlpha(0.35f));
   rect(batch,{at.x-22,at.y-4,44,2},u.id<3?jade:danger);
   if(animation_>0 && pose!=Pose::Idle)time=frame_.progress*(pose==Pose::Skill?7.99f:pose==Pose::Hit?2.99f:5.99f)/14.0f;
   art_.unit(batch,u.id<3?u.id:4+u.kind,u.direction,pose,time,at,1.33333f,tint);
   rect(batch,{at.x-23,at.y+3,46,4},background);rect(batch,{at.x-23,at.y+3,46.0f*u.health/std::max(1,u.maximum),4},u.id<3?jade:danger);
  }
 }
 if(!deploymentMode)effects(batch);
}
void BattleScene::effects(engine::SpriteBatch& batch) {
 if(animation_<=0 || frame_.event<0)return;
 const auto& e=last_.events[frame_.event];if(e.target<0 || e.target>=kUnits)return;
 const float t=frame_.progress;const auto at=feet(frame_.units[e.target].cell);char label[60];
 const auto center=Vec2{at.x,at.y-37};
 if(e.kind==EventKind::Hit) {
  const auto source=feet(frame_.units[e.actor].cell);
  const bool ki=(e.actor==visualCommand_.actor && visualCommand_.action==Action::Skill && (visualCommand_.skill==6 || visualCommand_.skill==7)) || (e.actor>=3 && model_.state().units[e.actor].kind==2);
  if(ki) {
   const float dx=center.x-source.x,dy=center.y-(source.y-37),length=std::sqrt(dx*dx+dy*dy),angle=std::atan2(dy,dx);
   const bool beam=visualCommand_.action==Action::Skill && visualCommand_.skill==7 && e.actor==visualCommand_.actor;
   if(beam && t>0.25f) {batch.drawQuad({(source.x+center.x)/2,(source.y-37+center.y)/2},{length,30*(1-t)+8},Color::rgb8(99,197,219).withAlpha(0.8f),angle);batch.drawQuad({(source.x+center.x)/2,(source.y-37+center.y)/2},{length,9},Color::rgb8(233,247,208),angle);}
   else { const float p=std::min(1.0f,t/0.48f);rect(batch,{source.x+dx*p-8,source.y-37+dy*p-8,16,16},jade);rect(batch,{source.x+dx*p-4,source.y-37+dy*p-4,8,8},ink); }
  }
  if(e.actor==visualCommand_.actor && visualCommand_.action==Action::Skill && (visualCommand_.skill==2 || visualCommand_.skill==4) && t<0.65f) {
   for(int i=0;i<6;++i) {const float angle=-1.8f+i*0.3f+t;batch.drawQuad({center.x+std::cos(angle)*27,center.y+std::sin(angle)*27},{12,3},gold.withAlpha(0.7f),angle+1.57f);}
  }
  if(t>=0.45f) {
   const float progress=(t-0.45f)/0.55f;
   for(int i=0;i<8;++i) {const float angle=i*0.785398f,radius=9+progress*(reducedEffects_?10:35);const float size=std::max(2.0f,7*(1-progress));batch.drawQuad({center.x+std::cos(angle)*radius,center.y+std::sin(angle)*radius},{size*2,size},i%2?gold:ink,angle);}
   std::snprintf(label,sizeof(label),"-%d",e.amount);text(batch,label,center.x,center.y-39-progress*22,29,gold,TextAlign::Center);
  }
 } else if(e.kind==EventKind::Heal || e.kind==EventKind::Ki || e.kind==EventKind::Insight) {
  for(int i=0;i<7;++i) { const float angle=i*0.9f+t*2,r=20+9*std::sin(t*3.14f);const float x=center.x+std::cos(angle)*r,y=center.y+std::sin(angle)*r-t*14;rect(batch,{x-2,y-5,4,10},jade.withAlpha(1-t*0.6f));if(e.kind==EventKind::Heal)rect(batch,{x-5,y-2,10,4},jade); }
  std::snprintf(label,sizeof(label),e.kind==EventKind::Heal?"회복 +%d":e.kind==EventKind::Ki?"기 +%d":"간파 +%d",e.amount);text(batch,label,center.x,center.y-48,22,jade,TextAlign::Center);
 } else if(e.kind==EventKind::Collision || e.kind==EventKind::Chase) {
  text(batch,e.kind==EventKind::Collision?"벽 충돌!":"추격!",center.x,center.y-50,26,gold,TextAlign::Center);
  if(e.kind==EventKind::Collision)for(int i=0;i<7;++i)rect(batch,{center.x+(i-3)*9*t,center.y+8-std::sin(t*3.14f)*25+i%3*9,4,4},muted);
 }
 if(!reducedEffects_ && e.actor==visualCommand_.actor && visualCommand_.action==Action::Skill && (visualCommand_.skill==7 || visualCommand_.skill==9) && e.kind==EventKind::Hit && t<0.35f) {
  rect(batch,{18,97,752,100},background.withAlpha(0.9f));art_.portrait(batch,heroPortrait(e.actor),{33,97,100,100});text(batch,kSkillData[visualCommand_.skill].name,155,124,37,gold);
 }
}
void BattleScene::deployment(engine::SpriteBatch& batch) {
 const auto& s=model_.state();board(batch,true);
 text(batch,"출전 배치",819,106,30,gold);
 text(batch,"인물을 고른 뒤 시작 칸을 누르세요.",819,155,21,muted);
 for(int i=0;i<3;++i) {
  portrait(batch,heroPortrait(i),{819,210+i*107.0f,80,80});
  button(batch,{916,210+i*107.0f,157,80},kHeroData[i].name,Button::Hero,i,true,actor_==i);
  button(batch,{1086,210+i*107.0f,161,80},s.deployed[i]?"출전 중":"대기 중",Button::Toggle,i,i!=0,s.deployed[i]);
 }
 fit(batch,model_.lesson().briefing,819,550,424,22,jade);
 text(batch,"로시 포함 최소 두 명 출전",819,588,21,muted);
 button(batch,{819,625,140,46},"도장으로",Button::Base);
 button(batch,{973,625,275,46},"전투 시작 →",Button::Start,0,true,true);
}
void BattleScene::battle(engine::SpriteBatch& batch) {
 const auto& s=model_.state();char line[220];board(batch);
 rect(batch,{18,98,197,72},background.withAlpha(0.88f));
 std::snprintf(line,sizeof(line),"%d / %d 턴",s.round,model_.lesson().roundLimit);text(batch,line,32,110,26,gold);
 text(batch,animation_>0?"행동 진행 중":"아군 차례",32,147,17,jade);
 rect(batch,{803,94,461,582},panel.withAlpha(0.5f));
 for(int i=0;i<3;++i) {
  const auto& u=s.units[i];std::snprintf(line,sizeof(line),"%s%s",kHeroData[i].name,u.acted?" ✓":"");
  button(batch,{819+i*146.0f,102,136,44},line,Button::Hero,i,u.active && u.health>0 && !u.acted,actor_==i);
 }
 const int shownActor=animation_>0 && frame_.event>=0 && last_.events[frame_.event].actor>=0 && last_.events[frame_.event].actor<3?last_.events[frame_.event].actor:actor_;
 portrait(batch,heroPortrait(shownActor),{819,165,112,112});
 auto unit=s.units[shownActor]; if(animation_>0)unit.health=frame_.units[shownActor].health; text(batch,kHeroData[shownActor].name,951,163,28,ink);
 std::snprintf(line,sizeof(line),"체력  %d / %d",unit.health,model_.maxHealth(shownActor));text(batch,line,951,206,21,jade);
 rect(batch,{951,236,277,8},background);rect(batch,{951,236,277.0f*unit.health/model_.maxHealth(shownActor),8},jade);
 std::snprintf(line,sizeof(line),"기  %d / 100",unit.ki);text(batch,line,951,253,21,gold);
 rect(batch,{951,281,277,6},background);rect(batch,{951,281,277.0f*unit.ki/100,6},gold);
 const int shownTarget=animation_>0 && frame_.event>=0?last_.events[frame_.event].target:target_;
 if(shownTarget>=0 && shownTarget<kUnits) {std::snprintf(line,sizeof(line),"대상  %s · 체력 %d",unitName(s,shownTarget),animation_>0?frame_.units[shownTarget].health:s.units[shownTarget].health);text(batch,line,819,308,21,gold);}
 if(animation_>0)text(batch,"이동 · 타격 · 밀쳐내기 · 추격",819,347,21,jade);
 else if(preview_.valid) {std::snprintf(line,sizeof(line),"피해 %d  /  받은 피해 %d  /  추격 %d",preview_.dealt,preview_.received,preview_.chases);fit(batch,line,819,347,425,21,ink);}
 else fit(batch,preview_.message,819,347,425,21,danger);
 const std::array<const char*,7> actions{"타격","기술","방어","기 모음","관찰","약초","대기"};
 for(int i=0;i<7;++i)button(batch,{819+(i%4)*109.0f,391+(i/4)*57.0f,99,48},actions[i],Button::Action,i,true,static_cast<int>(action_)==i);
 if(action_==Action::Skill) { std::snprintf(line,sizeof(line),"%s · 기 %d · 거리 %d",kSkillData[skill_].name,kSkillData[skill_].cost,kSkillData[skill_].range);button(batch,{819,511,426,44},line,Button::Skill,-1); }
 else text(batch,"파랑: 이동 · 빨간 선: 적의 현재 사거리",819,522,19,muted);
 button(batch,{819,573,426,49},"이동 + 행동 확정",Button::Commit,0,preview_.valid,true);
 button(batch,{1080,634,165,34},"아군 턴 종료",Button::EndTurn);
 std::snprintf(line,sizeof(line),"약초 %d · 금색 테두리: 대상",s.medicine);text(batch,line,819,643,17,muted);
}
void BattleScene::results(engine::SpriteBatch& batch) {
 const auto& s=model_.state();const bool victory=s.phase==Phase::Victory,complete=s.phase==Phase::Complete;
 scenery(batch);rect(batch,{52,265,666,266},background.withAlpha(0.88f));
 portrait(batch,complete?2:0,{83,290,210,210});text(batch,complete?"스승에게 한 걸음":"오늘도 한 걸음",323,323,31,gold);
 text(batch,"배움은 다음 여정으로",323,384,27,ink);text(batch,"함께 여는 길 · 제1장",323,450,23,jade);
 rect(batch,{803,96,461,572},panel);
 text(batch,complete?"다섯 전투의 끝":victory?"전투 승리":"다시 전열을 가다듬자",827,128,30,gold);
 if(victory) {
  text(batch,"전리품: 보급 4 · 약초 1",827,223,25,jade);text(batch,"출전 동료: 경험 8 + 행동 경험",827,278,22,ink);
  text(batch,"중량 경험 +4 · 추격 시 관계 +1",827,327,22,ink);text(batch,"성장과 기술 발견이 다음 이야기로 이어집니다.",827,401,20,muted);
  button(batch,{827,557,413,73},"보상 받기 · 다음 이야기 →",Button::Next,0,true,true);
 } else if(complete) {
  text(batch,"무태두",827,225,25,jade);text(batch,"이제 서로에게도 배울 수 있겠구나.",827,275,23,ink);
  text(batch,"전체 성장기와 손오공과의 만남은",827,380,22,muted);text(batch,"다음 이야기입니다.",827,420,22,muted);
  button(batch,{827,557,413,73},"새 여정 시작",Button::Restart,0,true,true);
 } else {
  text(batch,"로시가 쓰러졌거나 제한 턴이 끝났습니다.",827,225,21,ink);text(batch,"출전 당시의 상태로 다시 시작합니다.",827,282,22,muted);
  button(batch,{827,456,413,70},"이번 전투 재도전",Button::Retry,0,true,true);button(batch,{827,557,413,70},"도장에서 다시 정비",Button::Base);
 }
}
void BattleScene::overlay(engine::SpriteBatch& batch) {
 hitCount_=0;rect(batch,{0,0,1280,720},background.withAlpha(0.94f));rect(batch,{209,56,862,608},panel);
 if(confirmRestart_ || confirmEnd_) {
  text(batch,confirmRestart_?"진행을 지우고 새 여정을 시작할까요?":"행동하지 않은 동료가 있어도 턴을 마칠까요?",640,210,28,gold,TextAlign::Center);
  button(batch,{284,399,712,74},"확정",confirmRestart_?Button::RestartConfirm:Button::ConfirmEnd,0,true,true);button(batch,{284,499,712,74},"돌아가기",Button::Close);
 } else if(skillsOpen_) {
  text(batch,"사용할 기술",245,86,30,gold);int row=0;char line[160];
  for(int i=0;i<kSkills;++i)if(kSkillData[i].owner==actor_) {
   const auto& skill=kSkillData[i];const bool known=(model_.state().skills&(1<<i))!=0;
   std::snprintf(line,sizeof(line),"%s  기%d · 거리%d%s",skill.name,skill.cost,skill.range,known?"":" · 미습득");
   button(batch,{245,151+row++*86.0f,790,70},line,Button::Skill,i,known,skill_==i);
  }
  text(batch,"회복 기술은 동료 칸을 대상으로 선택합니다.",245,536,22,muted);button(batch,{245,584,790,58},"돌아가기",Button::Close);
 } else {
  text(batch,"위치가 연계를 만듭니다",245,82,30,gold);
  const std::array<const char*,12> lines{
   "아군마다 이동과 행동을 한 번, 마지막 행동 뒤 적 차례입니다.","적을 모두 쓰러뜨리면 승리. 로시가 쓰러지거나 제한 턴을 넘기면 패배.",
   "발경으로 적을 동료 옆에 밀면 추격. 동료마다 라운드당 한 번입니다.","바위·전장 가장자리·다른 적과 충돌하면 추가 피해를 줍니다.",
   "타격 +10, 사거리 이탈 +15, 피격 +5 기를 얻습니다.","제자리 기 모음은 20 → 30 → 50. 맞으면 추가 피해 6 · 기 손실 10.",
   "관찰·사거리 이탈·방어 피격은 기술 연구의 단서가 됩니다.","발경 2회와 단서 3개를 얻은 뒤 도장에서 붕권을 연구하세요.",
   "약초와 응급처치는 동료 칸을 대상으로 선택합니다.","빨간 선은 현재 사거리입니다. 적은 이동한 뒤 공격할 수 있습니다.",
   "테두리 안의 7×7칸이 전장입니다. 주변 강·숲·다리는 배경입니다.","상단의 연출 설정으로 화면 흔들림·섬광·컷인을 끌 수 있습니다."};
  for(int i=0;i<12;++i)fit(batch,lines[i],245,141+i*35.0f,790,21,ink);
  button(batch,{245,586,790,55},"돌아가기",Button::Close,0,true,true);
 }
}
void BattleScene::finish(const Outcome& out) {
 if(!out.valid) { notice_=out.message;return; }
 last_=out;notice_=out.message;duration_=timelineDuration(out);animation_=duration_;impactEvent_=-1;
 frame_=timelineFrame(before_,last_,0);
 if(sound_ && out.dealt==0)engine_->mixer().play(sfx_.click);
 action_=Action::Strike;refresh();save();
}
void BattleScene::act(Button buttonValue,int value) {
 invalidateInput();
 if(animation_>0) return;
 if(buttonValue==Button::Effects) { reducedEffects_=!reducedEffects_; if(!engine_->dataPath().empty()) { std::ofstream settings(engine_->dataPath()+"/visual-settings.txt"); settings<<reducedEffects_; } return; }
 if(buttonValue==Button::Close) { help_=skillsOpen_=confirmEnd_=confirmRestart_=false; return; }
 if(buttonValue==Button::Help) { help_=true; return; }
 if(help_) return;
 if(confirmEnd_) { if(buttonValue==Button::ConfirmEnd) { confirmEnd_=false; before_=model_.state().units; visualCommand_={-1,{},Action::Wait,-1,-1}; finish(model_.endTurn()); } return; }
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
  if(buttonValue==Button::Commit) { before_=model_.state().units; visualCommand_={actor_,destination_,action_,target_,skill_}; finish(model_.execute(visualCommand_)); return; }
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
