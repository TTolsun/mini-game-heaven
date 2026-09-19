#pragma once

#include <array>
#include <string>

namespace app::castle {

enum class Facility { Empty, Den, Treasury, Trap, Library };
enum class Species { Slime, Imp, Golem, Shade, Dragon, KingSlime };
enum class Phase { Build, Raid, Result };
struct FacilityInfo { const char* name; const char* detail; int cost; };
inline constexpr std::array<FacilityInfo, 5> kFacilities{{
    {"빈 방", "시설을 지어 성을 키워 보세요.", 0},
    {"마물 숙소", "마물 체력 +50% · 인접 마물 공격 +2", 55},
    {"보물 창고", "방어 종료 시 금화 +25", 65},
    {"가시 함정", "지나가는 모든 용사에게 피해 14", 45},
    {"마력 서고", "방어 종료 시 마력 +12 · 인접 함정 +8", 70},
}};
inline constexpr std::array<const char*, 6> kSpeciesNames{"물방울 슬라임", "불씨 임프", "이끼 골렘", "달그림자", "새벽 드래곤", "왕관 슬라임"};
inline constexpr std::array<const char*, 6> kTraits{"튼튼한 몸으로 용사를 막아요.", "빠른 불꽃 공격이 특기예요.", "단단한 몸으로 오래 버텨요.", "같은 방의 다른 용사에게도 피해를 줘요.", "불꽃 숨결로 같은 방의 용사를 함께 공격해요.", "초당 체력 3을 회복하며 오래 버텨요."};
inline constexpr std::array<int, 6> kBaseHp{65, 42, 100, 110, 180, 170};
inline constexpr std::array<int, 6> kBaseAttack{8, 13, 10, 25, 32, 18};
inline constexpr std::array<int, 3> kSummonCosts{25, 40, 60};
inline constexpr int kRooms = 12;
inline constexpr int kMaxMonsters = 8;
inline constexpr int kMaxHeroes = 12;
inline constexpr int kFinalDay = 10;
inline constexpr int kRoomColumns = 3;
inline constexpr int kEntranceRoom = 0;
constexpr int roomColumn(int room) { return (room / kRoomColumns) % 2 ? 2 - room % kRoomColumns : room % kRoomColumns; }
struct InvasionRoute { std::array<int, kRooms> rooms{}; int count = 0; };
struct Room { bool open = false; Facility facility = Facility::Empty; };
struct Monster { Species species = Species::Slime; int level = 1; int room = -1; float hp = 0; float cooldown = 0; };
struct Hero { float position = -1; float hp = 0; float maxHp = 0; float cooldown = 0; int lastRoom = -1; bool alive = false; };

// Deterministic simulation, independent of graphics and Android. A fixed-size
// actor pool keeps battle updates allocation-free; permanent state saves at
// player actions and settlement. Interrupted battles restart from preparation.
class CastleModel {
public:
    CastleModel();
    std::array<Room, kRooms> rooms{};
    std::array<Monster, kMaxMonsters> monsters{};
    std::array<Hero, kMaxHeroes> heroes{};
    int monsterCount = 2;
    int gold = 170, mana = 65, day = 1, heart = 100;
    int defeated = 0, totalHeroes = 0, spawned = 0, rewardGold = 0, rewardMana = 0;
    Phase phase = Phase::Build;
    bool won = false, completed = false;
    unsigned discovered = 3;
    float battleTime = 0, hitFlash = 0;
    const char* notice = "두 마물에게 먹이를 주고 달그림자로 합성해 보세요.";

    int resident(int room) const;
    int roomCount() const;
    int digCost() const;
    int trainingCost(int monster) const;
    int maxHp(const Monster& monster) const;
    int attack(const Monster& monster) const;
    int trapDamage(int room) const;
    int nextHeroCount() const;
    int nextHeroHp() const;
    static bool adjacentRooms(int first, int second);
    bool canDig(int room) const;
    bool connected(int first, int second) const;
    bool togglePassage(int first, int second);
    bool moveThrone(int room);
    int throneRoom() const { return throneRoom_; }
    const InvasionRoute& invasionRoute() const { return route_; }
    int heroRoom(const Hero& hero) const;
    bool build(int room, Facility facility);
    bool dig(int room);
    bool summon(Species species);
    bool assign(int monster, int room);
    bool train(int monster);
    int fusionResult(int first, int second) const;
    bool fuse(int first, int second);
    bool startRaid();
    void update(float dt);
    void continueBuilding();
    bool save(const std::string& path) const;
    bool load(const std::string& path);

private:
    std::array<unsigned, kRooms> passages_{};
    int throneRoom_ = 5;
    InvasionRoute route_{};
    float spawnClock_ = 0;
    void rebuildRoute();
    void connectRooms(int first, int second);
    int adjacentFacilities(int room, Facility facility) const;
    void finishRaid();
};
}  // namespace app::castle
