#include "app/castle/CastleModel.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>

namespace app::castle {
CastleModel::CastleModel() {
    for (int i = 0; i < 6; ++i) rooms[i].open = true;
    rooms[0].facility = Facility::Trap;
    rooms[1].facility = Facility::Den;
    monsters[0] = {Species::Slime, 1, 1};
    monsters[1] = {Species::Imp, 1, 3};
}
int CastleModel::resident(int room) const {
    for (int i = 0; i < monsterCount; ++i) if (monsters[i].room == room) return i;
    return -1;
}
int CastleModel::roomCount() const {
    int count = 0; for (const Room& room : rooms) if (room.open) ++count; return count;
}
int CastleModel::digCost() const { return 35 + (roomCount() - 6) * 15; }
int CastleModel::trainingCost(int m) const { return m >= 0 && m < monsterCount ? 15 + monsters[m].level * 10 : 0; }
int CastleModel::adjacentFacilities(int room, Facility facility) const {
    // Rooms follow a serpentine corridor. Visual neighbours are spatial, not
    // merely consecutive path indices.
    constexpr std::array<int, kRooms> columns{0, 1, 2, 2, 1, 0, 0, 1, 2, 2, 1, 0};
    int count = 0;
    for (int i = 0; i < kRooms; ++i)
        if (rooms[i].open && rooms[i].facility == facility &&
            std::abs(i / 3 - room / 3) + std::abs(columns[i] - columns[room]) == 1) ++count;
    return count;
}
int CastleModel::maxHp(const Monster& m) const {
    int hp = kBaseHp[static_cast<int>(m.species)] + (m.level - 1) * 20;
    if (m.room >= 0 && rooms[m.room].facility == Facility::Den) hp = hp * 3 / 2;
    return hp;
}
int CastleModel::attack(const Monster& m) const {
    return kBaseAttack[static_cast<int>(m.species)] + (m.level - 1) * 4 +
        (m.room >= 0 ? adjacentFacilities(m.room, Facility::Den) * 2 : 0);
}
int CastleModel::trapDamage(int room) const { return 14 + adjacentFacilities(room, Facility::Library) * 8; }
int CastleModel::nextHeroCount() const { return std::min(kMaxHeroes, 2 + day); }
int CastleModel::nextHeroHp() const { return 25 + day * 8 + (day % 5 == 0 ? 20 : 0); }
bool CastleModel::build(int room, Facility facility) {
    const int kind = static_cast<int>(facility);
    if (phase != Phase::Build || room < 0 || room >= kRooms || !rooms[room].open || kind < 1 || kind > 4) return false;
    if (rooms[room].facility != Facility::Empty) { notice = "시설은 빈 방에 지을 수 있어요."; return false; }
    if (gold < kFacilities[kind].cost) { notice = "금화가 부족해요. 방어 보상을 모아 주세요."; return false; }
    gold -= kFacilities[kind].cost;
    rooms[room].facility = facility;
    notice = "시설 완성! 옆방의 시설과 조합해 보세요.";
    return true;
}
bool CastleModel::dig(int room) {
    if (phase != Phase::Build || room < 0 || room >= kRooms || rooms[room].open) return false;
    if (room != roomCount()) { notice = "통로와 이어진 다음 방부터 확장해 주세요."; return false; }
    if (gold < digCost()) { notice = "확장할 금화가 부족해요."; return false; }
    gold -= digCost(); rooms[room].open = true;
    notice = "새로운 방이 열렸어요! 방어할 통로도 길어졌어요.";
    return true;
}
bool CastleModel::summon(Species species) {
    const int kind = static_cast<int>(species);
    if (phase != Phase::Build || kind < 0 || kind > 2) return false;
    if (monsterCount >= kMaxMonsters) { notice = "마물은 최대 8마리까지 함께할 수 있어요."; return false; }
    if (mana < kSummonCosts[kind]) { notice = "마력이 부족해요. 서고를 짓거나 방어에 도전하세요."; return false; }
    int room = -1;
    for (int i = 0; i < kRooms; ++i) if (rooms[i].open && resident(i) == -1) { room = i; break; }
    if (room == -1) { notice = "마물이 머물 빈자리가 필요해요. 성을 확장하세요."; return false; }
    mana -= kSummonCosts[kind]; monsters[monsterCount++] = {species, 1, room};
    discovered |= 1u << kind;
    notice = "새 식구가 왔어요! 훈련으로 더 강하게 키워 주세요.";
    return true;
}
bool CastleModel::assign(int m, int room) {
    if (phase != Phase::Build || m < 0 || m >= monsterCount || room < 0 || room >= kRooms || !rooms[room].open) return false;
    const int other = resident(room);
    if (other >= 0) monsters[other].room = monsters[m].room;
    monsters[m].room = room; notice = "배치 완료! 마물이 있던 방을 누르면 훈련할 수 있어요.";
    return true;
}
bool CastleModel::train(int m) {
    if (phase != Phase::Build || m < 0 || m >= monsterCount) return false;
    if (monsters[m].level >= 10) { notice = "최고 레벨에 도달했어요!"; return false; }
    if (gold < trainingCost(m)) { notice = "먹이를 살 금화가 부족해요."; return false; }
    gold -= trainingCost(m); ++monsters[m].level; notice = "레벨 업! 공격력과 체력이 올랐어요.";
    return true;
}
int CastleModel::fusionResult(int first, int second) const {
    if (first < 0 || second < 0 || first >= monsterCount || second >= monsterCount || first == second) return -1;
    int a = static_cast<int>(monsters[first].species), b = static_cast<int>(monsters[second].species);
    if (a > b) std::swap(a, b);
    if (a == 0 && b == 0) return 5;
    if (a == 0 && b == 1) return 3;
    if (a == 1 && b == 2) return 4;
    return -1;
}
bool CastleModel::fuse(int first, int second) {
    if (phase != Phase::Build) return false;
    const int result = fusionResult(first, second);
    if (result < 0) { notice = "이 조합은 합성할 수 없어요. 도감을 확인하세요."; return false; }
    if (monsters[first].level < 2 || monsters[second].level < 2) { notice = "두 마물 모두 레벨 2 이상이어야 해요."; return false; }
    if (mana < 40) { notice = "합성하려면 마력 40이 필요해요."; return false; }
    const int room = monsters[first].room;
    const int level = std::min(10, (monsters[first].level + monsters[second].level) / 2);
    const int keep = std::min(first, second), remove = std::max(first, second);
    monsters[keep] = {static_cast<Species>(result), level, room};
    for (int i = remove; i + 1 < monsterCount; ++i) monsters[i] = monsters[i + 1];
    --monsterCount; mana -= 40; discovered |= 1u << result;
    notice = "합성 성공! 새로운 마물이 도감에 등록되었어요.";
    return true;
}
bool CastleModel::startRaid() {
    if (phase != Phase::Build || completed) return false;
    phase = Phase::Raid; heart = 100; defeated = spawned = 0;
    totalHeroes = nextHeroCount(); battleTime = spawnClock_ = hitFlash = 0;
    heroes.fill({});
    for (int i = 0; i < monsterCount; ++i) { monsters[i].hp = static_cast<float>(maxHp(monsters[i])); monsters[i].cooldown = 0; }
    notice = "용사들이 들어옵니다! 마물과 함정이 성을 지켜요.";
    return true;
}
void CastleModel::update(float dt) {
    if (phase != Phase::Raid || !std::isfinite(dt) || dt <= 0) return;
    // Substeps preserve combat behaviour at both 30/60 Hz and fast-forward.
    dt = std::min(dt, 0.25f);
    while (dt > 0 && phase == Phase::Raid) {
        const float step = std::min(dt, 1.0f / 60.0f); dt -= step;
        battleTime += step; spawnClock_ -= step; hitFlash = std::max(0.0f, hitFlash - step);
        if (spawned < totalHeroes && spawnClock_ <= 0) {
            Hero& hero = heroes[spawned++]; hero = {};
            hero.position = -0.65f; hero.hp = hero.maxHp = static_cast<float>(nextHeroHp()); hero.alive = true;
            spawnClock_ = 2.2f;
        }
        for (int m = 0; m < monsterCount; ++m) {
            Monster& monster = monsters[m]; monster.cooldown -= step;
            if (monster.species == Species::KingSlime && monster.hp > 0)
                monster.hp = std::min(static_cast<float>(maxHp(monster)), monster.hp + step * 3);
            if (monster.hp <= 0 || monster.cooldown > 0) continue;
            for (Hero& hero : heroes) {
                if (!hero.alive || hero.position < 0 || static_cast<int>(hero.position) != monster.room) continue;
                if (monster.species == Species::Shade || monster.species == Species::Dragon) {
                    const float splash = attack(monster) * (monster.species == Species::Dragon ? 0.65f : 0.4f);
                    for (Hero& other : heroes) {
                        if (&other == &hero || !other.alive || other.position < 0 || static_cast<int>(other.position) != monster.room) continue;
                        other.hp -= splash;
                        if (other.hp <= 0) { other.alive = false; ++defeated; }
                    }
                }
                hero.hp -= static_cast<float>(attack(monster)); monster.cooldown = monster.species == Species::Golem ? 1.2f : 0.8f;
                hitFlash = 0.12f;
                if (hero.hp <= 0) { hero.alive = false; ++defeated; }
                break;
            }
        }
        for (Hero& hero : heroes) {
            if (!hero.alive) continue;
            const int room = hero.position < 0 ? -1 : static_cast<int>(hero.position);
            if (room >= roomCount()) { hero.alive = false; heart = std::max(0, heart - 25); hitFlash = 0.3f; continue; }
            if (room >= 0 && hero.lastRoom != room) {
                hero.lastRoom = room;
                if (rooms[room].facility == Facility::Trap) {
                    hero.hp -= static_cast<float>(trapDamage(room)); hitFlash = 0.12f;
                    if (hero.hp <= 0) { hero.alive = false; ++defeated; continue; }
                }
            }
            const int m = room >= 0 ? resident(room) : -1;
            if (m >= 0 && monsters[m].hp > 0) {
                hero.cooldown -= step;
                if (hero.cooldown <= 0) { monsters[m].hp -= 6.0f + day * 1.4f; hero.cooldown = 1.0f; hitFlash = 0.12f; }
            } else { hero.position += step * 0.72f; }
        }
        bool active = false; for (const Hero& hero : heroes) active |= hero.alive;
        if (heart <= 0 || (spawned == totalHeroes && !active)) finishRaid();
    }
}
void CastleModel::finishRaid() {
    won = heart > 0; phase = Phase::Result;
    rewardGold = (won ? 65 + day * 10 : 40) + defeated * 5;
    rewardMana = (won ? 20 + day * 3 : 20) + defeated * 3;
    for (const Room& room : rooms) {
        if (room.facility == Facility::Treasury) rewardGold += 25;
        if (room.facility == Facility::Library) rewardMana += 12;
    }
    gold = std::min(99999, gold + rewardGold); mana = std::min(99999, mana + rewardMana);
    completed = won && day == kFinalDay;
    notice = completed ? "작은 성이 어엿한 마왕성이 되었어요!" : won ? "성을 지켰어요! 보상으로 다음 습격을 준비하세요." : "성문이 뚫렸어요. 보급품으로 마물을 키워 다시 도전하세요.";
}
void CastleModel::continueBuilding() {
    if (phase != Phase::Result) return;
    if (won && !completed) ++day;
    phase = Phase::Build; heart = 100;
    notice = completed ? "10일 방어 성공! 성을 계속 꾸밀 수 있어요." : "시설과 마물을 살펴보고 다음 습격을 준비하세요.";
}
bool CastleModel::save(const std::string& path) const {
    // Persist result rewards with the next day, so process death cannot claim
    // the same result twice. Raid state intentionally restores preparation.
    std::ofstream out(path + ".tmp", std::ios::trunc);
    if (!out) return false;
    const int savedDay = day + (phase == Phase::Result && won && !completed ? 1 : 0);
    out << "CASTLE 1\n" << gold << ' ' << mana << ' ' << savedDay << ' ' << monsterCount << ' ' << completed << ' ' << discovered << '\n';
    for (const Room& room : rooms) out << room.open << ' ' << static_cast<int>(room.facility) << '\n';
    for (int i = 0; i < monsterCount; ++i) out << static_cast<int>(monsters[i].species) << ' ' << monsters[i].level << ' ' << monsters[i].room << '\n';
    out.flush(); if (!out) return false; out.close(); if (out.fail()) return false;
    return std::rename((path + ".tmp").c_str(), path.c_str()) == 0;
}
bool CastleModel::load(const std::string& path) {
    std::ifstream in(path); std::string magic; int version = 0;
    CastleModel candidate;
    if (!(in >> magic >> version) || magic != "CASTLE" || version != 1) return false;
    int complete = 0;
    if (!(in >> candidate.gold >> candidate.mana >> candidate.day >> candidate.monsterCount >> complete >> candidate.discovered) || candidate.discovered > 63 ||
        candidate.gold < 0 || candidate.gold > 99999 || candidate.mana < 0 || candidate.mana > 99999 ||
        candidate.day < 1 || candidate.day > kFinalDay || candidate.monsterCount < 1 || candidate.monsterCount > kMaxMonsters ||
        complete < 0 || complete > 1 || (complete && candidate.day != kFinalDay)) return false;
    candidate.completed = complete != 0;
    bool closed = false;
    for (Room& room : candidate.rooms) {
        int open, facility;
        if (!(in >> open >> facility) || open < 0 || open > 1 || facility < 0 || facility > 4 || (!open && facility != 0) || (closed && open)) return false;
        room = {open != 0, static_cast<Facility>(facility)}; closed |= !room.open;
    }
    if (candidate.roomCount() < 6) return false;
    std::array<bool, kRooms> occupied{};
    for (int i = 0; i < candidate.monsterCount; ++i) {
        int species; Monster& m = candidate.monsters[i];
        if (!(in >> species >> m.level >> m.room) || species < 0 || species > 5 || m.level < 1 || m.level > 10 ||
            m.room < 0 || m.room >= candidate.roomCount() || occupied[m.room]) return false;
        m.species = static_cast<Species>(species); occupied[m.room] = true;
        candidate.discovered |= 1u << species;
    }
    candidate.notice = candidate.completed ? "10일 방어를 마친 성에 돌아오셨군요!" : "돌아오셨군요, 마왕님! 성이 기다리고 있었어요.";
    *this = candidate; return true;
}
}  // namespace app::castle
