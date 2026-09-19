#include "app/castle/CastleScene.h"
#include "app/castle/MonsterArt.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "engine/Engine.h"
#include "engine/asset/AssetLoader.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/core/Log.h"

namespace app::castle {
using engine::Color;
using engine::Rect;
using engine::TextAlign;
namespace {
constexpr Color ink = Color::rgb8(46, 39, 62);
constexpr Color muted = Color::rgb8(125, 119, 133);
constexpr Color cream = Color::rgb8(248, 243, 228);
constexpr Color paper = Color::rgb8(255, 252, 242);
constexpr Color teal = Color::rgb8(43, 126, 109);
constexpr Color gold = Color::rgb8(242, 190, 91);
constexpr Color purple = Color::rgb8(143, 111, 197);
constexpr Color colors[] = {Color::rgb8(107, 194, 160), Color::rgb8(239, 133, 95), Color::rgb8(159, 179, 131),
    Color::rgb8(164, 136, 218), Color::rgb8(116, 183, 213), Color::rgb8(239, 189, 91)};
enum Action { RoomSelect, Tab, Summon, Feed, Move, FuseOpen, FusePartner, FuseConfirm, Close,
    Build, Dig, Raid, Continue, Speed, Pause, Sound };
int discoveredCount(unsigned mask) { int n = 0; while (mask) { n += mask & 1u; mask >>= 1; } return n; }
}
void CastleScene::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    const auto chars = engine.assets().readFile("fonts/castle-glyphs.txt");
    font_.load(engine.assets(), engine.atlas(), "castle_font", "fonts/NotoSansKR.ttf",
        std::string_view(reinterpret_cast<const char*>(chars.data()), chars.size()));
    auto& atlas = engine.atlas();
    if (!atlas.has("castle_monsters") && !atlas.add(engine.assets(), "castle_monsters", art::kAtlasPath)) {
        LOGE("Monster artwork failed to load");
    }
    const auto sheet = atlas.get("castle_monsters");
    for (int i = 0; i < static_cast<int>(monsterSprites_.size()); ++i) {
        monsterSprites_[i] = art::frame(sheet, i);
    }
    sfx_.build();
    savePath_ = engine.dataPath() + "/castle-v1.txt";
    model_.load(savePath_);
}
void CastleScene::save() { saved_ = model_.save(savePath_); }
void CastleScene::onExit() { save(); }
void CastleScene::update(float dt) {
    clock_ += dt; celebration_ = std::max(0.0f, celebration_ - dt);
    const Phase previous = model_.phase;
    const int kills = model_.defeated;
    if (!paused_) model_.update(dt * speed_);
    if (model_.defeated > kills && !muted_) engine_->mixer().play(sfx_.tap);
    if (previous == Phase::Raid && model_.phase == Phase::Result) {
        celebration_ = 2; save();
        if (!muted_) engine_->mixer().play(model_.won ? sfx_.fanfare : sfx_.thud);
        engine_->haptics().light();
    }
}
void CastleScene::label(engine::SpriteBatch& b, std::string_view text, float x, float y, float size, Color color, TextAlign align) {
    font_.draw(b, text, {x,y}, size, color, align);
}
void CastleScene::button(engine::SpriteBatch& b, Rect r, std::string_view text, int action, int value, bool enabled, bool accent) {
    b.drawRect({r.x, r.y + 4, r.w, r.h}, enabled ? Color::rgb8(208, 201, 184) : cream);
    b.drawRect(r, enabled ? (accent ? teal : paper) : Color::rgb8(227, 224, 216));
    const float size = std::min(24.0f, 24.0f * (r.w - 16) / std::max(1.0f, font_.measure(text, 24)));
    label(b, text, r.x + r.w / 2, r.y + (r.h - size) / 2 - 2, size, enabled ? (accent ? paper : ink) : muted, TextAlign::Center);
    if (enabled && hitCount_ < static_cast<int>(hits_.size())) hits_[hitCount_++] = {r, action, value};
}
Rect CastleScene::roomRect(int i) const {
    const int col = (i / 3) % 2 ? 2 - i % 3 : i % 3;
    return {40.0f + col * 218.0f, mapTop_ + (i / 3) * (roomHeight_ + 16), 204, roomHeight_};
}
void CastleScene::monster(engine::SpriteBatch& b, int species, float x, float y, float scale, bool dim) {
    if (species < 0 || species >= static_cast<int>(monsterSprites_.size())) return;
    const auto& sprite = monsterSprites_[species];
    if (!sprite.isValid()) return;
    const float hop = dim ? 0 : std::sin(clock_ * 3 + species) * 2;
    b.drawRect({x - scale * 4, y + scale * 7, scale * 8, scale}, ink.withAlpha(0.14f));
    const Color tint = dim ? Color::rgb8(115, 112, 123).withAlpha(0.55f) : Color::white();
    b.draw(sprite, {x, y + hop}, {scale * 18, scale * 18}, tint);
}
void CastleScene::renderMap(engine::SpriteBatch& b) {
    b.drawRect({20, mapTop_ - 18, 680, 4 * (roomHeight_ + 16) + 12}, Color::rgb8(61, 54, 77));
    for (int i = 0; i < kRooms; ++i) {
        const Rect r = roomRect(i);
        if (i + 1 < kRooms) {
            const Rect next = roomRect(i + 1);
            b.drawRect({std::min(r.x, next.x) + 88, std::min(r.y,next.y) + roomHeight_ / 2 - 8,
                std::abs(next.x - r.x) + 30, std::abs(next.y - r.y) + 16}, Color::rgb8(133, 120, 133));
        }
    }
    for (int i = 0; i < kRooms; ++i) {
        const Rect r = roomRect(i);
        const Room& room = model_.rooms[i];
        b.drawRect({r.x - 3, r.y - 3, r.w + 6, r.h + 6}, i == selectedRoom_ ? gold : Color::rgb8(89, 77, 96));
        b.drawRect(r, room.open ? Color::rgb8(224, 213, 194) : Color::rgb8(79, 69, 88));
        if (!room.open) {
            label(b, i == model_.roomCount() ? "+ 확장" : "미개척", r.x + r.w / 2, r.y + r.h / 2 - 13, 24,
                  i == model_.roomCount() ? gold : Color::rgb8(148, 133, 151), TextAlign::Center);
        } else {
            for (int tile = 1; tile < 4; ++tile) b.drawRect({r.x + tile * 51, r.y + 28, 1, r.h - 31}, Color::rgb8(211, 200, 183));
            char title[100]; std::snprintf(title, sizeof(title), "%02d  %s", i + 1, kFacilities[static_cast<int>(room.facility)].name);
            label(b, title, r.x + 10, r.y + 3, 19, ink);
            b.drawRect({r.x + 8, r.y + 28, r.w - 16, 2}, Color::rgb8(199, 187, 170));
            const int kind = static_cast<int>(room.facility);
            if (kind > 0) {
                const Color decor = kind == 1 ? teal : kind == 2 ? gold : kind == 3 ? Color::rgb8(170,113,109) : purple;
                b.drawRect({r.x + r.w - 35, r.y + r.h - 34, 22, 20}, decor);
                b.drawRect({r.x + r.w - 31, r.y + r.h - 38, 14, 5}, decor);
            }
            const int m = model_.resident(i);
            if (m >= 0) {
                const Monster& mon = model_.monsters[m];
                const bool down = model_.phase == Phase::Raid && mon.hp <= 0;
                monster(b, static_cast<int>(mon.species), r.x + r.w * 0.48f, r.y + r.h * 0.63f, std::min(5.0f, (r.h - 38) / 16), down);
                if (model_.phase == Phase::Raid) {
                    b.drawRect({r.x + 48, r.y + r.h - 9, 84, 5}, Color::rgb8(135,121,125));
                    b.drawRect({r.x + 48, r.y + r.h - 9, 84 * std::max(0.0f, mon.hp / model_.maxHp(mon)), 5}, teal);
                } else {
                    char lv[16]; std::snprintf(lv,sizeof(lv),"Lv.%d",mon.level); label(b,lv,r.x+10,r.y+r.h-27,17,teal);
                }
            }
        }
        if (model_.phase == Phase::Build && fusionFirst_ < 0) hits_[hitCount_++] = {r, RoomSelect, i};
    }
    if (model_.phase == Phase::Raid) for (int i = 0; i < model_.spawned; ++i) {
        const Hero& h = model_.heroes[i]; if (!h.alive) continue;
        const int idx = std::clamp(static_cast<int>(std::max(0.0f,h.position)),0,kRooms-1);
        const Rect r = roomRect(idx);
        const float x = r.x + 32 + (i % 3) * 24, y = r.y + r.h * 0.57f + (i % 2) * 6;
        b.drawRect({x-9,y-15,18,13}, Color::rgb8(229,237,232));
        b.drawRect({x-7,y-2,14,20}, Color::rgb8(99,128,172));
        b.drawRect({x+9,y-9,4,27}, paper);
        b.drawRect({x-10,y-23,24,4}, ink);
        b.drawRect({x-10,y-23,24*std::max(0.0f,h.hp/h.maxHp),4}, Color::rgb8(219,102,94));
    }
}
void CastleScene::renderPanel(engine::SpriteBatch& b) {
    const float y = panelTop_;
    char text[180];
    if (model_.phase == Phase::Raid) {
        label(b, "우리 마물들이 성을 지키고 있어요", 40,y,30,ink);
        std::snprintf(text,sizeof(text),"물리친 용사 %d / %d     성 체력 %d",model_.defeated,model_.totalHeroes,model_.heart);
        label(b,text,40,y+48,25,teal);
        b.drawRect({40,y+94,640,14},Color::rgb8(222,210,196));
        b.drawRect({40,y+94,640*model_.heart/100.0f,14},teal);
        label(b,"쓰러진 마물은 방어가 끝나면 모두 회복해요.",40,y+135,22,muted);
        button(b,{40,y+190,305,62},speed_==1 ? "속도 1배" : "속도 2배",Speed);
        button(b,{365,y+190,315,62},paused_ ? "계속하기" : "잠시 쉬기",Pause);
        return;
    }
    if (tab_ == 2) {
        std::snprintf(text,sizeof(text),"마물 도감   %d / 6종 발견",discoveredCount(model_.discovered)); label(b,text,40,y,29,ink);
        for (int i = 0; i < 6; ++i) {
            const float x = 40.0f + (i%2)*328, cy = y+48+(i/2)*96;
            const bool known = (model_.discovered & (1u<<i)) != 0;
            b.drawRect({x,cy,312,84},paper); monster(b,i,x+41,cy+42,4,!known);
            label(b,known ? kSpeciesNames[i] : "미발견 마물",x+83,cy+9,22,known?ink:muted);
            const char* recipes[] = {"마력 25로 소환", "마력 40으로 소환", "마력 60으로 소환", "슬라임 + 임프", "임프 + 골렘", "슬라임 + 슬라임"};
            label(b,recipes[i],x+83,cy+45,18,teal);
        }
        label(b,"합성 조건: 두 마물 Lv.2 이상 + 마력 40",40,y+345,21,muted);
        return;
    }
    if (tab_ == 1) {
        const Room& room = model_.rooms[selectedRoom_];
        std::snprintf(text,sizeof(text),"%02d번 방  /  %s",selectedRoom_+1,room.open?kFacilities[static_cast<int>(room.facility)].name:"미개척 지역");
        label(b,text,40,y,29,ink);
        if (!room.open) {
            label(b,"성을 넓히면 더 많은 마물이 함께 살 수 있어요.",40,y+55,23,muted);
            std::snprintf(text,sizeof(text),"방 확장  ·  금화 %d",model_.digCost());
            button(b,{40,y+110,640,66},text,Dig,selectedRoom_,selectedRoom_==model_.roomCount() && model_.gold>=model_.digCost(),true);
        } else if (room.facility != Facility::Empty) {
            label(b,kFacilities[static_cast<int>(room.facility)].detail,40,y+54,24,teal);
            label(b,"시설을 더 지으려면 위 지도에서 빈 방을 선택하세요.",40,y+108,22,muted);
            label(b,"숙소는 체력을, 서고는 소환과 합성을 도와줘요.",40,y+155,22,muted);
        } else {
            for (int i=1;i<=4;++i) {
                float cy=y+47+(i-1)*77;
                std::snprintf(text,sizeof(text),"%s · %d G",kFacilities[i].name,kFacilities[i].cost);
                button(b,{40,cy,246,58},text,Build,i,model_.gold>=kFacilities[i].cost);
                label(b,kFacilities[i].detail,305,cy+18,18,muted);
            }
        }
        return;
    }
    const int m = model_.resident(selectedRoom_);
    if (moving_ >= 0) {
        label(b,"이사할 방을 눌러 주세요",40,y,31,teal);
        label(b,"다른 마물이 있으면 서로 자리를 바꿔요.",40,y+52,23,muted);
        button(b,{40,y+110,640,62},"배치 취소",Close); return;
    }
    if (m >= 0) {
        const Monster& mon = model_.monsters[m];
        monster(b,static_cast<int>(mon.species),85,y+50,6);
        std::snprintf(text,sizeof(text),"%s  Lv.%d",kSpeciesNames[static_cast<int>(mon.species)],mon.level);
        label(b,text,144,y+3,30,ink);
        std::snprintf(text,sizeof(text),"체력 %d    공격 %d    %02d번 방",model_.maxHp(mon),model_.attack(mon),selectedRoom_+1);
        label(b,text,144,y+49,23,teal);
        label(b,kTraits[static_cast<int>(mon.species)],40,y+102,22,muted);
        std::snprintf(text,sizeof(text),mon.level<10?"먹이 주기 · %d G":"최고 레벨",model_.trainingCost(m));
        button(b,{40,y+144,254,62},text,Feed,m,mon.level<10 && model_.gold>=model_.trainingCost(m),true);
        button(b,{310,y+144,169,62},"방 옮기기",Move,m);
        button(b,{495,y+144,185,62},"합성하기",FuseOpen,m,model_.monsterCount>1);
    } else {
        label(b,"새 친구를 맞이해 보세요",40,y,30,ink);
        label(b,"마물이 있는 방을 누르면 먹이를 주거나 합성할 수 있어요.",40,y+55,22,muted);
    }
    label(b,"마물 소환",40,y+234,25,ink);
    std::snprintf(text,sizeof(text),"함께 사는 마물 %d / 8",model_.monsterCount); label(b,text,680,y+239,21,muted,TextAlign::Right);
    for (int i=0;i<3;++i) {
        const float x=40.0f+i*218;
        std::snprintf(text,sizeof(text),"%s · %d M",i==0?"슬라임":i==1?"임프":"골렘",kSummonCosts[i]);
        button(b,{x,y+283,204,66},text,Summon,i,model_.mana>=kSummonCosts[i] && model_.monsterCount<kMaxMonsters && model_.monsterCount<model_.roomCount());
    }
    label(b,"같은 조합은 같은 종으로! 합성법은 도감에서 확인하세요.",40,y+367,21,muted);
    if (bottom_ - (y+410) > 200) {
        b.drawRect({40,y+415,640,95},Color::rgb8(231,225,237));
        label(b,"오늘의 수집 목표",58,y+427,22,ink);
        const char* goal = !(model_.discovered & 8) ? "슬라임 + 임프를 Lv.2로 키워 달그림자 발견하기" :
            !(model_.discovered & 16) ? "임프 + 골렘을 합성해 새벽 드래곤 발견하기" :
            !(model_.discovered & 32) ? "슬라임 두 마리를 합성해 왕관 슬라임 발견하기" : "6종 도감 완성! 좋아하는 마물을 Lv.10까지 키워 보세요.";
        label(b,goal,58,y+468,21,teal);
    }
}
void CastleScene::renderOverlay(engine::SpriteBatch& b) {
    if (model_.phase != Phase::Result && fusionFirst_ < 0) return;
    hitCount_ = 0;
    b.drawRect({0,0,720,engine_->worldHeight()},ink.withAlpha(0.8f));
    const float y = std::max(engine_->safeTop()+30,(bottom_-760)/2);
    b.drawRect({30,y,660,760},cream);
    char text[200];
    if (model_.phase == Phase::Result) {
        label(b,model_.completed?"마왕성의 새 주인!":model_.won?"우리 마물들이 해냈어요!":"다음에는 더 강해질 거예요",360,y+42,34,ink,TextAlign::Center);
        for (int i=0;i<std::min(5,model_.monsterCount);++i) monster(b,static_cast<int>(model_.monsters[i].species),360+(i-(std::min(5,model_.monsterCount)-1)*0.5f)*118.0f,y+170,7);
        label(b,model_.completed?"10일 방어 성공 · 성장한 마물들과 이룬 승리":model_.won?"방어 성공! 모두 건강하게 돌아왔어요.":"마물은 사라지지 않아요. 먹이를 주고 다시 도전하세요.",360,y+267,23,teal,TextAlign::Center);
        std::snprintf(text,sizeof(text),"금화 +%d       마력 +%d",model_.rewardGold,model_.rewardMana);
        label(b,text,360,y+345,34,ink,TextAlign::Center);
        label(b,"보상과 마물 도감은 자동으로 저장됩니다.",360,y+413,23,muted,TextAlign::Center);
        button(b,{75,y+550,570,76},model_.completed?"내 마물들과 계속 지내기":model_.won?"먹이 주고 다음 날 준비하기":"보급품으로 성장하고 다시 도전",Continue,0,true,true);
        return;
    }
    label(b,"새로운 마물로 합성",65,y+30,32,ink);
    label(b,"두 마물이 한 마리로 바뀝니다. 도감 기록은 남아요.",65,y+83,22,muted);
    if (fusionSecond_ >= 0) {
        const int result=model_.fusionResult(fusionFirst_,fusionSecond_);
        const Monster& a=model_.monsters[fusionFirst_]; const Monster& c=model_.monsters[fusionSecond_];
        std::snprintf(text,sizeof(text),"%s Lv.%d + %s Lv.%d",kSpeciesNames[static_cast<int>(a.species)],a.level,kSpeciesNames[static_cast<int>(c.species)],c.level);
        label(b,text,360,y+153,24,ink,TextAlign::Center);
        monster(b,result,360,y+290,12);
        label(b,kSpeciesNames[result],360,y+390,35,teal,TextAlign::Center);
        std::snprintf(text,sizeof(text),"Lv.%d · 두 마물의 평균 레벨을 이어받아요",(a.level+c.level)/2);
        label(b,text,360,y+449,24,muted,TextAlign::Center);
        button(b,{65,y+536,590,70},"두 마물로 합성 · 마력 40",FuseConfirm,0,a.level>=2 && c.level>=2 && model_.mana>=40,true);
        label(b,"두 마물 모두 Lv.2 이상이어야 합니다.",360,y+626,22,muted,TextAlign::Center);
    } else {
        label(b,"함께 합성할 마물을 선택하세요",65,y+134,25,teal);
        int row=0;
        for (int i=0;i<model_.monsterCount;++i) {
            if (i==fusionFirst_) continue;
            const Monster& m=model_.monsters[i]; const int result=model_.fusionResult(fusionFirst_,i);
            std::snprintf(text,sizeof(text),"%s Lv.%d  >  %s",kSpeciesNames[static_cast<int>(m.species)],m.level,result<0?"조합 없음":kSpeciesNames[result]);
            button(b,{65,y+186+row*62.0f,590,52},text,FusePartner,i,result>=0); ++row;
        }
        label(b,"슬라임+임프 / 임프+골렘 / 슬라임+슬라임",65,y+637,20,muted);
    }
    button(b,{65,y+682,590,52},"취소하고 돌아가기",Close);
}
void CastleScene::render(engine::SpriteBatch& b) {
    hitCount_ = 0;
    const float top=engine_->safeTop(); bottom_=engine_->worldHeight()-engine_->safeBottom()-18;
    mapTop_=top+245;
    // Fit the board and nursery to short portrait screens without scaling text.
    roomHeight_=std::clamp((bottom_-mapTop_-564)/4.0f-16,67.0f,126.0f);
    panelTop_=mapTop_+4*(roomHeight_+16)+36;
    b.drawRect({0,0,720,engine_->worldHeight()},cream);
    b.drawRect({0,0,720,top+151},ink);
    label(b,"마물 정원",35,top+15,43,paper);
    label(b,"꼬마 마왕의 수집 일기",38,top+70,21,Color::rgb8(189,175,203));
    char text[160]; std::snprintf(text,sizeof(text),"%d일 / 10일",model_.day); label(b,text,492,top+25,28,gold,TextAlign::Right);
    button(b,{536,top+21,149,53},muted_?"소리 꺼짐":"소리 켜짐",Sound);
    std::snprintf(text,sizeof(text),"금화 %d",model_.gold); label(b,text,38,top+111,24,gold);
    std::snprintf(text,sizeof(text),"마력 %d",model_.mana); label(b,text,261,top+111,24,Color::rgb8(199,175,248));
    std::snprintf(text,sizeof(text),"도감 %d / 6",discoveredCount(model_.discovered)); label(b,text,678,top+111,24,paper,TextAlign::Right);
    if (model_.phase == Phase::Build) {
        std::snprintf(text,sizeof(text),model_.completed?"10일 방어 완료!":model_.day%5==0?"정예 용사 %d명 · 체력 %d":"다음 습격 %d명 · 체력 %d",model_.nextHeroCount(),model_.nextHeroHp());
        label(b,text,36,top+175,21,ink);
        button(b,{435,top+164,250,53},"방어 시작",Raid,0,!model_.completed,true);
    } else label(b,paused_?"잠시 쉬고 있어요":model_.phase==Phase::Raid?"용사들의 습격! 우리 마물들을 응원해 주세요.":"방어가 끝났어요",36,top+173,25,ink);
    renderMap(b);
    renderPanel(b);
    const float tabsY=bottom_-67;
    b.drawRect({20,tabsY-51,680,38},Color::rgb8(236,228,211));
    const char* notice=!saved_?"저장하지 못했어요. 다음 행동 때 다시 저장합니다.":model_.notice;
    const float ns=std::min(20.0f,20.0f*650/std::max(1.0f,font_.measure(notice,20)));
    label(b,notice,360,tabsY-44,ns,teal,TextAlign::Center);
    const char* tabs[]={"마물 돌보기","성 꾸미기","마물 도감"};
    for (int i=0;i<3;++i) button(b,{25+i*230.0f,tabsY,210,62},tabs[i],Tab,i,model_.phase==Phase::Build,tab_==i);
    if (celebration_>0 && model_.phase!=Phase::Result) {
        for(int i=0;i<20;++i) b.drawRect({30.0f+i*35,std::fmod(clock_*90+i*71.0f,engine_->worldHeight()),7,12},colors[i%6]);
    }
    renderOverlay(b);
}
void CastleScene::act(int action,int value) {
    // Android may deliver several gestures before the next render. Never reuse
    // targets whose modal, inventory indices or enabled state just changed.
    hitCount_ = 0;
    bool changed=false;
    switch(action) {
    case RoomSelect:
        selectedRoom_=value;
        if(moving_>=0) { changed=model_.assign(moving_,value); if(changed) moving_=-1; }
        else if(!model_.rooms[value].open) tab_=1;
        break;
    case Tab: tab_=value; moving_=-1; break;
    case Summon: changed=model_.summon(static_cast<Species>(value)); if(changed) selectedRoom_=model_.monsters[model_.monsterCount-1].room; break;
    case Feed: changed=model_.train(value); if(changed) celebration_=1.3f; break;
    case Move: moving_=value; model_.notice="지도에서 이사할 방을 선택하세요."; break;
    case FuseOpen: fusionFirst_=value; fusionSecond_=-1; break;
    case FusePartner: fusionSecond_=value; break;
    case FuseConfirm:
        { const int room=model_.monsters[fusionFirst_].room; changed=model_.fuse(fusionFirst_,fusionSecond_);
          if(changed) { selectedRoom_=room; fusionFirst_=fusionSecond_=-1; celebration_=2.4f; if(!muted_) engine_->mixer().play(sfx_.newBest); } }
        break;
    case Close: fusionFirst_=fusionSecond_=-1; moving_=-1; break;
    case Build: changed=model_.build(selectedRoom_,static_cast<Facility>(value)); break;
    case Dig: changed=model_.dig(value); break;
    case Raid: moving_=-1; changed=model_.startRaid(); paused_=false; break;
    case Continue: model_.continueBuilding(); changed=true; tab_=0; break;
    case Speed: speed_=speed_==1?2:1; break;
    case Pause: paused_=!paused_; break;
    case Sound: muted_=!muted_; break;
    }
    if(changed) { save(); engine_->haptics().light(); }
    if(!muted_) engine_->mixer().play(changed?sfx_.ding:sfx_.click);
}
void CastleScene::onTouch(const engine::TouchEvent& event) {
    using Phase=engine::TouchEvent::Phase;
    if(event.phase==Phase::Down && pointer_==-1) {
        pointer_=event.pointerId; pressed_=-1;
        for(int i=hitCount_-1;i>=0;--i) if(hits_[i].rect.contains(event.position)) { pressed_=i; pressedHit_=hits_[i]; break; }
    } else if(event.pointerId==pointer_ && (event.phase==Phase::Up || event.phase==Phase::Cancel)) {
        if(event.phase==Phase::Up && pressed_>=0 && pressedHit_.rect.contains(event.position)) {
            // A modal or phase change between down/up must never redirect a tap.
            for(int i=0;i<hitCount_;++i) {
                if(hits_[i].action==pressedHit_.action && hits_[i].value==pressedHit_.value && hits_[i].rect.contains(event.position)) {
                    act(pressedHit_.action,pressedHit_.value); break;
                }
            }
        }
        pointer_=-1; pressed_=-1;
    }
}
bool CastleScene::onBack() {
    hitCount_=0; pointer_=-1; pressed_=-1;
    if(fusionFirst_>=0 || moving_>=0) { fusionFirst_=fusionSecond_=-1; moving_=-1; return true; }
    if(model_.phase==Phase::Raid) { paused_=!paused_; save(); return true; }
    save(); return false;
}
}  // namespace app::castle
