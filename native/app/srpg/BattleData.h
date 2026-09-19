#pragma once

#include <array>

namespace app::srpg {
inline constexpr int kColumns = 7;
inline constexpr int kRows = 7;
inline constexpr int kCells = kColumns * kRows;
inline constexpr int kLessons = 3;
inline constexpr int kHealth = 72;
inline constexpr int kMaxKi = 100;
inline constexpr int kMove = 2;
inline constexpr int kObserveRange = 3;
inline constexpr int kStrikeRange = 2;
inline constexpr int kBasicDamage = 6;
inline constexpr int kCopyDamage = 10;
inline constexpr int kMasterDamage = 18;
inline constexpr int kCopyCost = 20;
inline constexpr int kMasterCost = 15;
inline constexpr int kWallDamage = 6;
inline constexpr int kExposedDamage = 6;
inline constexpr int kInterruptedKi = 10;
inline constexpr int kMasteryUses = 2;
inline constexpr int kFullInsight = 7;
inline constexpr std::array<int, 3> kChargeGains{20, 30, 50};

struct Cell {
    int x = 0;
    int y = 0;
    friend bool operator==(Cell, Cell) = default;
};
constexpr bool inside(Cell cell) {
    return cell.x >= 0 && cell.y >= 0 && cell.x < kColumns && cell.y < kRows;
}
constexpr int index(Cell cell) { return cell.y * kColumns + cell.x; }
constexpr Cell cellAt(int value) { return {value % kColumns, value / kColumns}; }
inline constexpr std::array<Cell, 4> kDirections{{{0,-1},{1,0},{0,1},{-1,0}}};

enum class Action { Observe, Guard, Strike, Charge, Technique };
enum class Phase { Playing, Victory, Defeat };
enum class Insight { Motion = 1, Footwork = 2, Impact = 4 };

struct Lesson {
    const char* name;
    const char* teacher;
    const char* hint;
    int enemyHealth;
    int enemyDamage;
    int roundLimit;
    Cell start;
    Cell opponent;
    std::array<const char*, kRows> tiles;
};
inline constexpr std::array<Lesson, kLessons> kLessonData{{
    {"첫 수련 · 주먹보다 먼저", "무태두", "상대의 주먹만 보지 마라. 발과 중심을 살펴라.",
     30, 10, 18, {3,4}, {3,2}, {".......",".......",".......",".......",".......",".......","......."}},
    {"두 번째 · 흉내에서 무술로", "학선인", "익힌 동작도 틈이 있다. 성공 뒤의 자세까지 보아라.",
     44, 12, 20, {2,5}, {3,3}, {".......",".#...#.",".......",".......",".#.....",".......","......."}},
    {"마지막 · 나의 붕권", "도장 사범", "강한 주먹보다 좋은 자리를 먼저 찾아라.",
     62, 14, 22, {3,5}, {3,3}, {".......",".##.##.",".......",".......",".#...#.",".......","......."}},
}};
} // namespace app::srpg
