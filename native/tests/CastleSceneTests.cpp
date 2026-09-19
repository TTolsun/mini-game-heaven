#include "app/castle/CastleScene.h"
#include "engine/Engine.h"
#include "engine/asset/AssetLoader.h"
#include <cassert>
#include <cstdio>

namespace app::castle {
// Attach services without loading GL resources. An uninitialized SpriteBatch
// ignores drawing, while the real scene still builds its interactive targets.
struct CastleSceneTestAccess {
    static void attach(CastleScene& scene, engine::Engine& engine, const std::string& path) {
        scene.engine_ = &engine;
        scene.savePath_ = path;
        scene.muted_ = true;
    }
    static int targets(const CastleScene& scene) { return scene.hitCount_; }
    static const CastleModel& model(const CastleScene& scene) { return scene.model_; }
    static bool editing(const CastleScene& scene) { return scene.routeEditing_; }
};
}
class EmptyAssets final : public engine::AssetLoader {
    std::vector<uint8_t> readFile(const std::string&) override { return {}; }
};
using namespace app::castle;
using Touch = engine::TouchEvent;
void tap(CastleScene& scene, float x, float y) {
    scene.onTouch({0, Touch::Phase::Down, {x,y}});
    scene.onTouch({0, Touch::Phase::Up, {x,y}});
}
int main(int argc, char** argv) {
    const std::string path = std::string(argc > 1 ? argv[1] : ".") + "/castle-scene-test-save.txt";
    EmptyAssets assets;
    engine::Engine engine(assets);
    engine::SpriteBatch batch;
    CastleScene scene;
    CastleSceneTestAccess::attach(scene, engine, path);
    CastleModel saved;

    // Real 720x1280 layout: feed both starters, select the first, open fusion.
    scene.render(batch); tap(scene, 150, 910);
    scene.render(batch); tap(scene, 570, 395);
    scene.render(batch); tap(scene, 150, 910);
    scene.render(batch); tap(scene, 350, 280);
    scene.render(batch); tap(scene, 570, 910);
    scene.render(batch); tap(scene, 350, 460);
    scene.render(batch);
    assert(CastleSceneTestAccess::targets(scene) == 2);
    tap(scene, 350, 820);
    assert(CastleSceneTestAccess::targets(scene) == 0);
    // A second queued tap must not execute FuseConfirm with consumed indices.
    tap(scene, 350, 820);
    assert(saved.load(path));
    assert(saved.monsterCount == 1 && saved.monsters[0].species == Species::Shade);
    assert(saved.mana == 25);

    // Back closes a modal and cancels an in-flight gesture before rendering.
    CastleScene cancel;
    CastleSceneTestAccess::attach(cancel, engine, path);
    cancel.render(batch); tap(cancel, 570, 910);
    cancel.render(batch);
    cancel.onTouch({0, Touch::Phase::Down, {350,460}});
    assert(cancel.onBack());
    assert(CastleSceneTestAccess::targets(cancel) == 0);
    cancel.onTouch({0, Touch::Phase::Up, {350,460}});
    assert(!cancel.onBack()); // The released finger did not reopen the modal.
    cancel.render(batch);
    tap(cancel, 150, 910);
    assert(saved.load(path) && saved.monsters[0].level == 2);
    // Exercise the real facility-tab editor, including disabled raid input and
    // two queued toggles before the next frame (must not silently reopen a wall).
    CastleScene routes;
    CastleSceneTestAccess::attach(routes, engine, path);
    routes.render(batch); tap(routes,360,1220); // Facilities.
    routes.render(batch); tap(routes,140,280); // Entrance room.
    routes.render(batch); tap(routes,570,755); // Route editor.
    assert(CastleSceneTestAccess::editing(routes));
    routes.render(batch); tap(routes,150,840); // Close 01-02.
    tap(routes,150,840); // Stale second tap must be ignored.
    assert(CastleSceneTestAccess::model(routes).invasionRoute().count==0);
    assert(saved.load(path) && saved.invasionRoute().count==0);
    routes.render(batch); tap(routes,550,185); // Disabled raid.
    assert(CastleSceneTestAccess::model(routes).phase==Phase::Build);
    routes.render(batch); tap(routes,150,840); // Repair passage.
    assert(CastleSceneTestAccess::model(routes).invasionRoute().count==6);
    assert(routes.onBack() && !CastleSceneTestAccess::editing(routes));
    routes.render(batch); tap(routes,550,185);
    assert(CastleSceneTestAccess::model(routes).phase==Phase::Raid);
    std::remove(path.c_str());
    std::puts("PASS: queued fusion taps, route editor, disconnected raid button, queued passage edits, BACK and input recovery");
}
