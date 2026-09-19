#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include "app/srpg/BattleModel.h"
#include "engine/math/Vec2.h"
namespace app::srpg {
inline float eventDuration(EventKind kind) {
 switch(kind) {
 case EventKind::Move:return 0.20f;
 case EventKind::Push:return 0.18f;
 case EventKind::Hit:return 0.32f;
 case EventKind::Chase:case EventKind::Collision:return 0.20f;
 default:return 0.32f;
 }
}
struct VisualUnit { engine::Vec2 cell; int health=0,direction=0; bool active=false; };
struct TimelineFrame { std::array<VisualUnit,kUnits> units{}; int event=-1; float progress=0; };
inline float timelineDuration(const Outcome& out) {
 float duration=0.20f; for(int i=0;i<out.count;++i)duration+=eventDuration(out.events[i].kind); return duration;
}
inline int facing(engine::Vec2 from,engine::Vec2 to) {
 const float x=to.x-from.x,y=to.y-from.y;
 return std::abs(x)>std::abs(y)?(x>=0?1:3):(y>=0?0:2);
}
// Replay render state only. Hit is the sole damage event: Chase and Collision
// announce their cause, followed by the model's armor-adjusted Hit event.
inline TimelineFrame timelineFrame(const std::array<Unit,kUnits>& before,const Outcome& out,float elapsed) {
 TimelineFrame result;
 for(int i=0;i<kUnits;++i)result.units[i]={{static_cast<float>(before[i].cell.x),static_cast<float>(before[i].cell.y)},before[i].health,i<3?2:0,before[i].active};
 for(int i=0;i<out.count;++i) {
  const auto& e=out.events[i]; const float duration=eventDuration(e.kind),t=std::clamp(elapsed/duration,0.0f,1.0f);
  if(e.target>=0 && e.target<kUnits) {
   auto& u=result.units[e.target];
   if(e.kind==EventKind::Move || e.kind==EventKind::Push) {
    const float ease=1-(1-t)*(1-t)*(1-t);
    u.cell={e.from.x+(e.to.x-e.from.x)*ease,e.from.y+(e.to.y-e.from.y)*ease};
    if(e.kind==EventKind::Move)u.direction=facing({static_cast<float>(e.from.x),static_cast<float>(e.from.y)},{static_cast<float>(e.to.x),static_cast<float>(e.to.y)});
   }
   if(e.actor>=0 && e.actor<kUnits && e.actor!=e.target) result.units[e.actor].direction=facing(result.units[e.actor].cell,u.cell);
   if(t>=0.45f && e.kind==EventKind::Hit)u.health=std::max(0,u.health-e.amount);
   if(t>=0.45f && e.kind==EventKind::Heal)u.health+=e.amount;
  }
  if(elapsed<duration) { result.event=i;result.progress=t;break; }
  elapsed-=duration;
 }
 return result;
}
}
