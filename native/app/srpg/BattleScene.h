#pragma once

#include <array>
#include <string>
#include "app/Sfx.h"
#include "app/srpg/BattleModel.h"
#include "engine/Scene.h"
#include "engine/graphics/Font.h"
#include "engine/math/Rect.h"

namespace app::srpg {
class BattleScene final : public engine::Scene {
public:
    void onEnter(engine::Engine& engine) override;
    void onExit() override;
    void update(float dt) override;
    void render(engine::SpriteBatch& batch) override;
    void onTouch(const engine::TouchEvent& event) override;
    bool onBack() override;
private:
    friend struct BattleSceneTestAccess;
    enum class Button { Tile, Action, Commit, Help, Close, Retry, Next, Restart };
    struct Hit { engine::Rect rect; Button button; int value = 0; };
    engine::Engine* engine_ = nullptr;
    engine::Font font_;
    Sfx sfx_;
    BattleModel model_;
    Cell destination_{3,4};
    Action action_ = Action::Observe;
    Outcome preview_;
    std::array<int, kCells> reachable_{};
    std::array<Hit, 80> hits_{};
    int hitCount_ = 0;
    int pointer_ = -1;
    bool pressed_ = false;
    Hit pressedHit_{};
    bool help_ = false;
    bool saved_ = true;
    bool sound_ = true;
    float animation_ = 0;
    float scale_ = 1, left_ = 0, top_ = 0;
    std::string savePath_;
    const char* notice_ = "파란 칸으로 이동한 뒤 행동을 선택하고 확정하세요.";
    void refresh();
    void save();
    void invalidateInput();
    void act(Button button, int value);
    engine::Rect box(engine::Rect rect) const;
    void rect(engine::SpriteBatch& batch, engine::Rect area, engine::Color color);
    void text(engine::SpriteBatch& batch, std::string_view message, float x, float y,
              float size, engine::Color color, engine::TextAlign align = engine::TextAlign::Left);
    void button(engine::SpriteBatch& batch, engine::Rect area, std::string_view title,
                Button action, int value = 0, bool enabled = true, bool selected = false);
    void board(engine::SpriteBatch& batch);
    void overlay(engine::SpriteBatch& batch);
};
} // namespace app::srpg
