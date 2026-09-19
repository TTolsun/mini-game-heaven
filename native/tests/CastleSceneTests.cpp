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
    std::remove(path.c_str());
    std::puts("PASS: queued fusion taps, modal back cancellation, next-frame input recovery");
}
