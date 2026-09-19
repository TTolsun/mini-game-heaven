#include "app/srpg/BattleScene.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include "engine/Engine.h"
#include "engine/asset/AssetLoader.h"
#include "engine/graphics/SpriteBatch.h"

namespace app::srpg {
namespace {
using engine::Color;
using engine::Rect;
using engine::TextAlign;
constexpr Color background = Color::rgb8(20,28,34);
constexpr Color panel = Color::rgb8(34,45,52);
constexpr Color ink = Color::rgb8(239,236,214);
constexpr Color muted = Color::rgb8(160,176,177);
constexpr Color jade = Color::rgb8(94,208,172);
constexpr Color gold = Color::rgb8(239,194,104);
constexpr Color danger = Color::rgb8(237,112,100);
constexpr float tileSize = 80;
constexpr float boardX = 80, boardY = 252;
constexpr std::array<const char*, 5> actionNames{"관찰", "방어", "타격", "기 모으기", "철산격 모방"};
Rect tileRect(Cell cell) { return {boardX + cell.x * tileSize, boardY + cell.y * tileSize, tileSize, tileSize}; }
}
void BattleScene::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    const auto glyphs = engine.assets().readFile("fonts/srpg-glyphs.txt");
    font_.load(engine.assets(), engine.atlas(), "srpg_font", "fonts/NotoSansKR.ttf",
        std::string_view(reinterpret_cast<const char*>(glyphs.data()), glyphs.size()));
    sfx_.build();
    savePath_ = engine.dataPath() + "/wuten-v1.txt";
    model_.load(savePath_);
    refresh();
}
void BattleScene::onExit() { save(); }
void BattleScene::save() { saved_ = !savePath_.empty() && model_.save(savePath_); }
void BattleScene::refresh() {
    destination_ = model_.state().roshi;
    reachable_ = model_.movement();
    preview_ = model_.preview({destination_, action_});
}
void BattleScene::invalidateInput() { hitCount_ = 0; pointer_ = -1; pressed_ = false; }
void BattleScene::update(float dt) { animation_ = std::max(0.0f, animation_ - dt); }
Rect BattleScene::box(Rect area) const {
    return {left_ + area.x * scale_, top_ + area.y * scale_, area.w * scale_, area.h * scale_};
}
void BattleScene::rect(engine::SpriteBatch& batch, Rect area, Color color) { batch.drawRect(box(area), color); }
void BattleScene::text(engine::SpriteBatch& batch, std::string_view message, float x, float y, float size, Color color, TextAlign align) {
    font_.draw(batch, message, {left_ + x * scale_, top_ + y * scale_}, size * scale_, color, align);
}
void BattleScene::button(engine::SpriteBatch& batch, Rect area, std::string_view title, Button action, int value, bool enabled, bool selected) {
    rect(batch, area, selected ? jade : panel);
    const float size = std::min(23.0f, 23.0f * (area.w - 14) / std::max(1.0f, font_.measure(title, 23)));
    text(batch, title, area.x + area.w / 2, area.y + (area.h - size) / 2 - 2, size,
         enabled ? (selected ? background : ink) : muted.withAlpha(0.45f), TextAlign::Center);
    if (enabled && animation_ <= 0 && hitCount_ < static_cast<int>(hits_.size())) {
        hits_[hitCount_++] = {box(area), action, value};
    }
}
void BattleScene::board(engine::SpriteBatch& batch) {
    const auto& state = model_.state();
    rect(batch, {boardX-5,boardY-5,570,570}, Color::rgb8(11,19,24));
    for (int i = 0; i < kCells; ++i) {
        const Cell cell = cellAt(i);
        const Rect tile = tileRect(cell);
        Color color = (cell.x + cell.y) % 2 ? Color::rgb8(65,77,69) : Color::rgb8(71,85,75);
        if (model_.blocked(cell)) color = Color::rgb8(37,47,44);
        else if (reachable_[i] >= 0) color = Color::rgb8(55,104,99);
        if (model_.threatened(cell)) color = Color::rgb8(134,67,58);
        rect(batch, {tile.x+2,tile.y+2,tile.w-4,tile.h-4}, color);
        if (model_.blocked(cell)) {
            rect(batch, {tile.x+14,tile.y+12,52,54}, Color::rgb8(107,111,93));
            rect(batch, {tile.x+20,tile.y+16,40,7}, Color::rgb8(149,147,120));
        }
        if (model_.threatened(cell)) text(batch, "!", tile.x+10,tile.y+5,22,gold);
        if (state.phase == Phase::Playing && !help_ && !model_.blocked(cell) && cell != state.enemy && reachable_[i] >= 0 && animation_ <= 0) {
            hits_[hitCount_++] = {box(tile),Button::Tile,i};
        }
    }
    const Rect selected = tileRect(destination_);
    rect(batch,{selected.x+3,selected.y+3,74,3},jade);
    rect(batch,{selected.x+3,selected.y+74,74,3},jade);
    rect(batch,{selected.x+3,selected.y+3,3,74},jade);
    rect(batch,{selected.x+74,selected.y+3,3,74},jade);
    // Readable prototype figures; final age-specific character art is separate.
    auto figure = [&](Cell cell, Color robe, const char* name, int health, int maximum) {
        const Rect tile = tileRect(cell);
        rect(batch,{tile.x+17,tile.y+57,48,10},background.withAlpha(0.6f));
        rect(batch,{tile.x+27,tile.y+23,27,32},robe);
        rect(batch,{tile.x+29,tile.y+12,22,19},Color::rgb8(226,184,141));
        rect(batch,{tile.x+27,tile.y+9,26,8},Color::rgb8(33,32,29));
        rect(batch,{tile.x+25,tile.y+48,12,15},robe);
        rect(batch,{tile.x+44,tile.y+48,12,15},robe);
        rect(batch,{tile.x+12,tile.y+69,56,5},background);
        rect(batch,{tile.x+12,tile.y+69,56.0f*health/maximum,5},robe);
        text(batch,name,tile.x+40,tile.y+33,16,background,TextAlign::Center);
    };
    figure(state.roshi,jade,"로시",state.health,kHealth);
    figure(state.enemy,danger,"상대",state.enemyHealth,model_.lesson().enemyHealth);
    const Rect enemy = tileRect(state.enemy);
    const auto direction = kDirections[state.facing];
    rect(batch,{enemy.x+37+direction.x*30.0f,enemy.y+37+direction.y*30.0f,8,8},gold);
    if (destination_ != state.roshi) text(batch,"이동",selected.x+40,selected.y+29,21,jade,TextAlign::Center);
}
void BattleScene::render(engine::SpriteBatch& batch) {
    hitCount_ = 0;
    const float available = engine_->worldHeight() - engine_->safeTop() - engine_->safeBottom();
    scale_ = std::min(1.0f, std::max(0.1f, available / 1160.0f));
    left_ = (720.0f - 720.0f*scale_) / 2;
    top_ = engine_->safeTop() + std::max(0.0f,(available - 1160.0f*scale_)/2);
    batch.drawRect({0,0,720,engine_->worldHeight()},background);
    const auto& state = model_.state();
    char line[220];
    text(batch,"武天  /  무술의 시작",30,12,34,ink);
    button(batch,{562,10,128,46},"수련 안내",Button::Help);
    text(batch,model_.lesson().name,30,65,26,gold);
    std::snprintf(line,sizeof(line),"%d / %d 수련  ·  %d / %d 턴",state.lesson+1,kLessons,state.round,model_.lesson().roundLimit);
    text(batch,line,690,74,19,muted,TextAlign::Right);
    rect(batch,{25,108,670,48},panel);
    const float hintSize = std::min(20.0f,20.0f*636/std::max(1.0f,font_.measure(model_.lesson().hint,20)));
    text(batch,model_.lesson().hint,360,120,hintSize,ink,TextAlign::Center);
    std::snprintf(line,sizeof(line),"로시  체력 %d/%d    기 %d/%d",state.health,kHealth,state.ki,kMaxKi);
    text(batch,line,30,171,22,jade);
    std::snprintf(line,sizeof(line),"%s  체력 %d",model_.lesson().teacher,state.enemyHealth);
    text(batch,line,690,171,22,danger,TextAlign::Right);
    const std::array<const char*,3> clues{"동작 관찰","공격선 회피","방어로 버티기"};
    for (int i = 0; i < 3; ++i) {
        const bool known = (state.insights & (1 << i)) != 0;
        rect(batch,{35+i*225.0f,213,10,10},known?gold:muted.withAlpha(0.35f));
        text(batch,clues[i],55+i*225.0f,204,20,known?gold:muted);
    }
    board(batch);
    text(batch,"파랑: 이동 가능   빨강 !: 다음 공격선   금색 점: 상대 방향",360,821,18,muted,TextAlign::Center);
    const char* stage = state.successes == kMasteryUses ? "체득 · 로시류 붕권" : state.insights == kFullInsight ? "모방 · 철산격" : "간파 중 · 철산격";
    std::snprintf(line,sizeof(line),"%s    실전 성공 %d/%d",stage,state.successes,kMasteryUses);
    text(batch,line,360,858,25,gold,TextAlign::Center);
    if (state.phase == Phase::Playing) {
        if (preview_.valid) {
            std::snprintf(line,sizeof(line),"예상: 준 피해 %d  /  받은 피해 %d  /  기 %+d%s",preview_.dealt,preview_.received,preview_.kiChange,
                          preview_.newInsights ? "  · 새 단서" : "");
            text(batch,line,360,899,20,ink,TextAlign::Center);
        } else text(batch,preview_.message,360,899,18,danger,TextAlign::Center);
        for (int i = 0; i < 5; ++i) {
            const char* title = i == 4 && state.successes == kMasteryUses ? "로시류 붕권" : actionNames[i];
            button(batch,{25+i*136.0f,940,126,58},title,Button::Action,i,true,static_cast<int>(action_)==i);
        }
        button(batch,{25,1013,670,65},"이동 + 행동 확정",Button::Commit,0,preview_.valid,true);
    }
    const char* message = saved_ ? notice_ : "저장에 실패했습니다. 다음 행동에서 다시 저장합니다.";
    const float size = std::min(20.0f,20.0f*660/std::max(1.0f,font_.measure(message,20)));
    text(batch,message,360,1097,size,saved_?muted:danger,TextAlign::Center);
    text(batch,"전투 중 간파 → 모방 → 체득  ·  같은 단서는 중복 성장하지 않습니다",360,1133,16,muted,TextAlign::Center);
    if (help_ || state.phase != Phase::Playing) overlay(batch);
}
void BattleScene::overlay(engine::SpriteBatch& batch) {
    hitCount_ = 0;
    batch.drawRect({0,0,720,engine_->worldHeight()},background.withAlpha(0.92f));
    rect(batch,{35,220,650,710},panel);
    if (help_) {
        text(batch,"무태두의 수련장",360,250,34,gold,TextAlign::Center);
        const std::array<const char*,11> lines{
            "목표: 제한 턴 안에 대련 상대를 쓰러뜨리세요.",
            "칸을 고르고 행동을 선택한 뒤 확정합니다.",
            "관찰: 상대 3칸 안에서 공격 동작을 봅니다.",
            "회피: 빨간 공격선에서 안전한 칸으로 이동합니다.",
            "방어: 빨간 칸에서 방어하며 충격을 배웁니다.",
            "세 단서를 얻으면 철산격 모방이 열립니다.",
            "모방은 기 20을 쓰고 맞으면 더 아픕니다.",
            "2회 성공하면 기 15의 밀쳐내기 붕권이 됩니다.",
            "기 모으기: 같은 자리에서 20 → 30 → 50입니다.",
            "모으다 맞으면 추가 피해와 기 손실이 있습니다.",
            "재도전은 이번 수련 입장 때의 배움으로 돌아갑니다."};
        for (int i = 0; i < static_cast<int>(lines.size()); ++i) text(batch,lines[i],64,315+i*43.0f,21,ink);
        button(batch,{100,827,520,65},"수련으로 돌아가기",Button::Close,0,true,true);
        return;
    }
    const auto& state = model_.state();
    const bool victory = state.phase == Phase::Victory;
    const bool last = state.lesson == kLessons-1;
    text(batch,victory?(last?"세 번의 수련을 마쳤습니다":"대련 승리"):"다시 배울 기회",360,278,34,victory?gold:danger,TextAlign::Center);
    text(batch,victory?"배운 무술은 다음 수련으로 이어집니다.":"쓰러졌거나 수련 시간이 끝났습니다.",360,346,23,ink,TextAlign::Center);
    char line[128];
    const int count = ((state.insights & 1)!=0) + ((state.insights & 2)!=0) + ((state.insights & 4)!=0);
    std::snprintf(line,sizeof(line),"단서 %d/3    실전 모방 %d/2",count,state.successes);
    text(batch,line,360,416,27,jade,TextAlign::Center);
    text(batch,state.successes==kMasteryUses?"로시류 붕권을 체득했습니다.":"다른 행동으로 새로운 단서를 찾아보세요.",360,472,22,ink,TextAlign::Center);
    text(batch,"기술을 배운 방식과 써 본 자리를 기억해 보세요.",360,535,20,muted,TextAlign::Center);
    if (victory && !last) button(batch,{100,630,520,70},"다음 수련",Button::Next,0,true,true);
    else if (victory) button(batch,{100,630,520,70},"처음부터 새 수련",Button::Restart,0,true,true);
    button(batch,{100,737,520,65},"이번 수련 다시 도전",Button::Retry,0,true,!victory);
    text(batch,"새 수련과 재도전은 해당 시작 상태로 돌아갑니다.",360,840,19,muted,TextAlign::Center);
}
void BattleScene::act(Button buttonValue, int value) {
    invalidateInput();
    if (buttonValue == Button::Help) { help_ = true; return; }
    if (buttonValue == Button::Close) { help_ = false; return; }
    if (help_ || animation_ > 0) return;
    bool changed = false;
    if (model_.state().phase != Phase::Playing) {
        if (buttonValue == Button::Next) changed = model_.nextLesson();
        else if (buttonValue == Button::Retry) { model_.retry(); changed = true; }
        else if (buttonValue == Button::Restart && model_.state().phase == Phase::Victory && model_.state().lesson == kLessons-1) {
            model_.restart(); changed = true;
        }
        if (changed) { refresh(); save(); notice_ = "새 수련입니다. 다음 공격선과 단서를 살펴보세요."; }
        return;
    }
    if (buttonValue == Button::Tile && value >= 0 && value < kCells && reachable_[value] >= 0) destination_ = cellAt(value);
    else if (buttonValue == Button::Action && value >= 0 && value < 5) action_ = static_cast<Action>(value);
    else if (buttonValue == Button::Commit) {
        const Outcome result = model_.execute({destination_,action_});
        if (result.valid) {
            changed = true; notice_ = result.message; animation_ = 0.3f;
            if (sound_) engine_->mixer().play(result.mastered || result.learned ? sfx_.fanfare : result.dealt ? sfx_.tap : sfx_.click);
            engine_->haptics().light();
            refresh(); save();
        }
    }
    if (!changed) preview_ = model_.preview({destination_,action_});
}
void BattleScene::onTouch(const engine::TouchEvent& event) {
    using Touch = engine::TouchEvent::Phase;
    if (event.phase == Touch::Down && pointer_ == -1) {
        pointer_ = event.pointerId; pressed_ = false;
        for (int i = hitCount_-1; i >= 0; --i) if (hits_[i].rect.contains(event.position)) {
            pressedHit_ = hits_[i]; pressed_ = true; break;
        }
    } else if (event.pointerId == pointer_ && (event.phase == Touch::Up || event.phase == Touch::Cancel)) {
        if (event.phase == Touch::Up && pressed_ && pressedHit_.rect.contains(event.position)) {
            for (int i = 0; i < hitCount_; ++i) if (hits_[i].button == pressedHit_.button && hits_[i].value == pressedHit_.value && hits_[i].rect.contains(event.position)) {
                act(pressedHit_.button,pressedHit_.value); break;
            }
        }
        pointer_ = -1; pressed_ = false;
    }
}
bool BattleScene::onBack() {
    invalidateInput();
    if (help_) { help_ = false; return true; }
    if (destination_ != model_.state().roshi) { refresh(); return true; }
    save(); return false;
}
} // namespace app::srpg
