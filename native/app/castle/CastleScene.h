#pragma once
#include <array>
#include <string>
#include "app/castle/CastleModel.h"
#include "app/Sfx.h"
#include "engine/Scene.h"
#include "engine/graphics/Font.h"
#include "engine/graphics/Sprite.h"
#include "engine/math/Rect.h"

namespace app::castle {
class CastleScene final : public engine::Scene {
public:
    void onEnter(engine::Engine& engine) override;
    void onExit() override;
    void update(float dt) override;
    void render(engine::SpriteBatch& batch) override;
    void onTouch(const engine::TouchEvent& event) override;
    bool onBack() override;
private:
    friend struct CastleSceneTestAccess;
    struct Hit { engine::Rect rect; int action; int value; };
    engine::Engine* engine_ = nullptr;
    engine::Font font_;
    std::array<engine::Sprite, 6> monsterSprites_{};
    Sfx sfx_;
    CastleModel model_;
    std::string savePath_;
    std::array<Hit, 64> hits_{};
    int hitCount_ = 0, selectedRoom_ = 1, tab_ = 0, moving_ = -1;
    int fusionFirst_ = -1, fusionSecond_ = -1;
    int pressed_ = -1, pointer_ = -1;
    Hit pressedHit_{};
    bool paused_ = false, muted_ = false, saved_ = true;
    bool routeEditing_ = false;
    float clock_ = 0, speed_ = 1, celebration_ = 0;
    float mapTop_ = 238, roomHeight_ = 116, panelTop_ = 760, bottom_ = 1300;
    void save();
    void act(int action, int value);
    void label(engine::SpriteBatch& batch, std::string_view text, float x, float y, float size,
               engine::Color color, engine::TextAlign align = engine::TextAlign::Left);
    void button(engine::SpriteBatch& batch, engine::Rect rect, std::string_view text, int action,
                int value = 0, bool enabled = true, bool accent = false);
    void monster(engine::SpriteBatch& batch, int species, float x, float y, float scale, bool dim = false);
    engine::Rect roomRect(int index) const;
    void renderMap(engine::SpriteBatch& batch);
    void renderPanel(engine::SpriteBatch& batch);
    void renderRouteEditor(engine::SpriteBatch& batch);
    void renderOverlay(engine::SpriteBatch& batch);
};
}  // namespace app::castle
