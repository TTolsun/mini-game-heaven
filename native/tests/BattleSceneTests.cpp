#include "app/srpg/BattleScene.h"
#include "engine/Engine.h"
#include "engine/asset/AssetLoader.h"

#include <cassert>
#include <cstdio>

namespace app::srpg {
struct BattleSceneTestAccess {
    static void attach(BattleScene& scene, engine::Engine& engine, const std::string& path) {
        scene.engine_=&engine; scene.savePath_=path; scene.sound_=false; scene.refresh();
    }
    static engine::Vec2 target(const BattleScene& scene, int kind, int value=0) {
        for (int i=0;i<scene.hitCount_;++i) {
            const auto& hit=scene.hits_[i];
            if (static_cast<int>(hit.button)==kind && hit.value==value) return {hit.rect.x+hit.rect.w/2,hit.rect.y+hit.rect.h/2};
        }
        assert(false && "Expected an interactive target"); return {};
    }
    static bool has(const BattleScene& scene, int kind) {
        for (int i=0;i<scene.hitCount_;++i) if (static_cast<int>(scene.hits_[i].button)==kind) return true;
        return false;
    }
    static const BattleState& state(const BattleScene& scene) { return scene.model_.state(); }
    static Cell destination(const BattleScene& scene) { return scene.destination_; }
    static int targets(const BattleScene& scene) { return scene.hitCount_; }
    static bool help(const BattleScene& scene) { return scene.help_; }
};
}
class EmptyAssets final : public engine::AssetLoader {
    std::vector<uint8_t> readFile(const std::string&) override { return {}; }
};
using namespace app::srpg;
using Access=BattleSceneTestAccess;
void tap(BattleScene& scene, engine::Vec2 point) {
    scene.onTouch({0,engine::TouchEvent::Phase::Down,point});
    scene.onTouch({0,engine::TouchEvent::Phase::Up,point});
}
int main(int argc, char** argv) {
    const std::string file=std::string(argc>1?argv[1]:".")+"/srpg-scene-test-save.txt";
    EmptyAssets assets;
    engine::Engine engine(assets);
    engine::SpriteBatch batch;
    BattleScene scene;
    Access::attach(scene,engine,file);
    scene.render(batch);
    tap(scene,Access::target(scene,1,static_cast<int>(Action::Guard)));
    assert(Access::targets(scene)==0);
    scene.render(batch);
    const auto commit=Access::target(scene,2);
    tap(scene,commit); tap(scene,commit);
    assert(Access::state(scene).round==2 && Access::state(scene).health==67);
    assert(Access::state(scene).insights==4 && Access::targets(scene)==0);
    scene.render(batch); assert(!Access::has(scene,2)); // Animation lock.
    scene.update(1); scene.render(batch);
    tap(scene,Access::target(scene,0,index({4,3})));
    assert(Access::destination(scene)==(Cell{4,3}));
    // BACK rolls back the uncommitted move and cancels a queued release.
    scene.render(batch);
    const auto pending=Access::target(scene,2);
    scene.onTouch({0,engine::TouchEvent::Phase::Down,pending});
    assert(scene.onBack());
    scene.onTouch({0,engine::TouchEvent::Phase::Up,pending});
    assert(Access::state(scene).round==2 && Access::destination(scene)==Access::state(scene).roshi);
    scene.render(batch);
    tap(scene,Access::target(scene,1,static_cast<int>(Action::Technique)));
    scene.render(batch); assert(!Access::has(scene,2)); // Locked technique cannot be confirmed.
    tap(scene,Access::target(scene,3));
    scene.render(batch);
    assert(Access::help(scene) && Access::targets(scene)==1);
    assert(scene.onBack() && !Access::help(scene));
    scene.render(batch);
    tap(scene,Access::target(scene,0,index({4,3})));
    scene.render(batch);
    tap(scene,Access::target(scene,1,static_cast<int>(Action::Observe)));
    scene.render(batch); tap(scene,Access::target(scene,2));
    assert(Access::state(scene).insights==kFullInsight);
    BattleModel saved;
    assert(saved.load(file) && saved.state()==Access::state(scene));
    std::remove(file.c_str());
    std::puts("PASS: real Scene targets, queued confirms, animation lock, BACK rollback, help modal, locked skill and saved learning");
}
