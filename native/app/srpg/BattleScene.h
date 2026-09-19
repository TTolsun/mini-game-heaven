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
 enum class Button { Tile, Hero, Action, Commit, Help, Close, Story, Tab, Train, Gear, Research, Rest, Prepare, Toggle, Slot, Start, EndTurn, ConfirmEnd, Next, Retry, Restart, Skill, RestartConfirm, Base };
 struct Hit { engine::Rect rect; Button button; int value=0; };
 engine::Engine* engine_=nullptr;
 engine::Font font_; Sfx sfx_; BattleModel model_;
 Cell destination_; int actor_=0,target_=-1,skill_=0,tab_=0;
 Action action_=Action::Strike;
 Outcome preview_,last_;
 std::array<int,kCells> reachable_{};
 std::array<Hit,96> hits_{}; int hitCount_=0,pointer_=-1;
 bool pressed_=false,help_=false,skillsOpen_=false,confirmEnd_=false,confirmRestart_=false,saved_=true,sound_=true;
 Hit pressedHit_{};
 float animation_=0,scale_=1,left_=0,top_=0;
 std::string savePath_;
 const char* notice_="인물과 목적지, 행동과 대상을 고르세요.";
 void refresh(); void save(); void invalidateInput();
 void act(Button button,int value);
 void finish(const Outcome& out);
 engine::Rect box(engine::Rect area) const;
 void rect(engine::SpriteBatch& batch,engine::Rect area,engine::Color color);
 void text(engine::SpriteBatch& batch,std::string_view message,float x,float y,float size,engine::Color color,engine::TextAlign align=engine::TextAlign::Left);
 void fit(engine::SpriteBatch& batch,std::string_view message,float x,float y,float width,float size,engine::Color color);
 void button(engine::SpriteBatch& batch,engine::Rect area,std::string_view title,Button action,int value=0,bool enabled=true,bool selected=false);
 void board(engine::SpriteBatch& batch,bool deployment=false);
 void story(engine::SpriteBatch& batch); void base(engine::SpriteBatch& batch);
 void deployment(engine::SpriteBatch& batch); void battle(engine::SpriteBatch& batch);
 void results(engine::SpriteBatch& batch); void overlay(engine::SpriteBatch& batch);
};
}
