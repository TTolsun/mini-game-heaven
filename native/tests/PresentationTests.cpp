#include "app/srpg/BattleTimeline.h"
#include "engine/graphics/Viewport.h"
#include <cassert>
#include <cmath>
#include <cstdio>
using namespace app::srpg;
bool near(float a,float b) { return std::abs(a-b)<0.01f; }
int main() {
 for(const auto screen:std::array<engine::Vec2,4>{{{2400,1080},{3120,1440},{1080,2400},{1280,720}}}) {
  const auto v=engine::fitViewport(static_cast<int>(screen.x),static_cast<int>(screen.y),80,16,32,20);
  assert(v.x>=80 && v.y>=16 && v.right()<=screen.x-32 && v.bottom()<=screen.y-20);
  const auto point=engine::viewportPoint(v,v.center()); assert(near(point.x,640) && near(point.y,360));
  assert(!v.contains({v.x-1,v.y+1}) && !v.contains({v.right(),v.bottom()}));
 }
 const auto wide=engine::fitViewport(2560,1440,0,0,0,0);assert(wide.x==0 && wide.y==0 && wide.w==2560 && wide.h==1440);
 std::array<Unit,kUnits> before{};before[0].cell={2,2};before[0].health=70;before[0].active=true;
 before[1].cell={4,3};before[1].health=60;before[1].active=true;
 before[3].cell={3,2};before[3].health=30;before[3].active=true;
 Outcome out;out.count=6;
 out.events[0]={EventKind::Hit,0,3,10,{},{}};
 out.events[1]={EventKind::Push,0,3,1,{3,2},{4,2}};
 out.events[2]={EventKind::Chase,1,3,8,{},{}};
 out.events[3]={EventKind::Hit,1,3,8,{},{}};
 out.events[4]={EventKind::Collision,1,3,6,{4,2},{5,2}};
 out.events[5]={EventKind::Hit,1,3,3,{},{}}; // Armor reduces collision damage to 3.
 auto f=timelineFrame(before,out,0);assert(f.event==0 && f.units[3].health==30);
 f=timelineFrame(before,out,0.17f);assert(f.units[3].health==20);
 f=timelineFrame(before,out,0.41f);assert(f.event==1 && f.units[3].cell.x>3 && f.units[3].cell.x<4);
 f=timelineFrame(before,out,0.62f);assert(f.event==2 && f.units[3].health==20); // Chase is a label, not a second hit.
 f=timelineFrame(before,out,timelineDuration(out));assert(f.event==-1 && f.units[3].health==9 && f.units[3].cell.x==4);
 assert(before[3].health==30 && before[3].cell.x==3); // Presentation is read-only.
 assert(facing({0,0},{1,0})==1 && facing({0,0},{-1,0})==3 && facing({0,0},{0,-1})==2 && facing({0,0},{0,1})==0);
 std::puts("PASS: safe viewport / letterbox mapping / sequential knockback / armor-adjusted damage / immutable visual replay");
}
