#include "app/srpg/BattleScene.h"
#include "engine/Engine.h"
#include "engine/asset/AssetLoader.h"
#include <cassert>
#include <cstdio>
namespace app::srpg {
struct BattleSceneTestAccess {
 static void attach(BattleScene& s,engine::Engine& e,const std::string& path) { s.engine_=&e; s.savePath_=path; s.sound_=false; s.refresh(); }
 static engine::Vec2 target(const BattleScene& s,int kind,int value=0) {
  for(int i=0;i<s.hitCount_;++i) { const auto& h=s.hits_[i]; if(static_cast<int>(h.button)==kind && h.value==value) return {h.rect.x+h.rect.w/2,h.rect.y+h.rect.h/2}; }
  std::printf("Missing target kind=%d value=%d phase=%d\n",kind,value,static_cast<int>(s.model_.state().phase)); assert(false); return {};
 }
 static int targets(const BattleScene& s) { return s.hitCount_; }
 static const BattleState& state(const BattleScene& s) { return s.model_.state(); }
 static Cell destination(const BattleScene& s) { return s.destination_; }
 static int actor(const BattleScene& s) { return s.actor_; }
};
}
class EmptyAssets final : public engine::AssetLoader { std::vector<uint8_t> readFile(const std::string&) override { return {}; } };
using namespace app::srpg; using Access=BattleSceneTestAccess;
void tap(BattleScene& scene,engine::Vec2 at) { scene.onTouch({0,engine::TouchEvent::Phase::Down,at}); scene.onTouch({0,engine::TouchEvent::Phase::Up,at}); }
int main(int argc,char** argv) {
 const std::string file=std::string(argc>1?argv[1]:".")+"/srpg-scene-test-save.txt";
 EmptyAssets assets; engine::Engine engine(assets); engine::SpriteBatch batch; BattleScene scene; Access::attach(scene,engine,file);
 auto press=[&](int kind,int value=0) { scene.render(batch); tap(scene,Access::target(scene,kind,value)); };
 scene.render(batch); const auto story=Access::target(scene,6); tap(scene,story); tap(scene,story);
 assert(Access::state(scene).phase==Phase::Base && Access::state(scene).bond==1);
 press(8,2); assert(Access::state(scene).trained);
 press(12); assert(Access::state(scene).phase==Phase::Deployment);
 press(14,1); assert(Access::state(scene).slots[0]==1);
 press(15); assert(Access::state(scene).phase==Phase::Playing);
 press(2,3); scene.render(batch); const auto commit=Access::target(scene,3); tap(scene,commit); tap(scene,commit);
 assert(Access::state(scene).round==1 && Access::state(scene).units[0].acted && Access::state(scene).units[0].ki==30);
 scene.render(batch); assert(Access::targets(scene)==0);
 scene.update(1); press(0,index({2,4})); assert(Access::destination(scene)==(Cell{2,4}));
 press(2,3); scene.render(batch); const auto pending=Access::target(scene,3);
 scene.onTouch({0,engine::TouchEvent::Phase::Down,pending}); assert(scene.onBack()); scene.onTouch({0,engine::TouchEvent::Phase::Up,pending});
 assert(Access::destination(scene)==Access::state(scene).units[Access::actor(scene)].cell && !Access::state(scene).units[1].acted);
 press(4); scene.render(batch); assert(Access::targets(scene)==1); assert(scene.onBack());
 press(16); scene.render(batch); assert(Access::targets(scene)==2); assert(scene.onBack()); assert(Access::state(scene).round==1);
 press(16); press(17); assert(Access::state(scene).round==2); scene.update(1);
 BattleModel restored; assert(restored.load(file) && restored.state()==Access::state(scene));
 std::remove(file.c_str()); std::puts("PASS: story/base/deployment input, slot swap, queued confirms, animation lock, BACK, help/end-turn modals and save parity");
}
