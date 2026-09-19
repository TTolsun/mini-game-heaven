#pragma once
#include <array>
namespace app::srpg {
inline constexpr int kColumns=7, kRows=7, kCells=49, kHeroes=3, kUnits=7, kLessons=5, kSkills=10, kMaxKi=100;
struct Cell { int x=-1,y=-1; friend bool operator==(Cell,Cell)=default; };
constexpr bool inside(Cell c) { return c.x>=0 && c.y>=0 && c.x<kColumns && c.y<kRows; }
constexpr int index(Cell c) { return c.y*kColumns+c.x; }
constexpr Cell cellAt(int i) { return {i%kColumns,i/kColumns}; }
inline constexpr std::array<Cell,4> kDirections{{{0,-1},{1,0},{0,1},{-1,0}}};
enum class Phase { Story, Base, Deployment, Playing, Victory, Defeat, Complete };
enum class Action { Strike, Skill, Guard, Charge, Observe, Item, Wait };
enum class SkillShape { Single, Cross, Line, Heal };
struct HeroData { const char* name; const char* role; int health,move; };
inline constexpr std::array<HeroData,3> kHeroData{{
 {"로시","밀쳐내기 · 반격",76,2},{"학선인","기동 · 추격",62,3},{"란란","기공 · 회복",60,2}
}};
struct EnemyData { const char* name; int health,damage,range,move; };
inline constexpr std::array<EnemyData,4> kEnemyData{{
 {"권사",30,10,1,2},{"철벽병",42,12,1,1},{"기공사",26,13,3,1},{"돌격수",34,15,2,2}
}};
struct SkillData { const char* name; int owner,cost,damage,range,push; SkillShape shape; bool stun; };
inline constexpr std::array<SkillData,kSkills> kSkillData{{
 {"발경",0,10,10,1,1,SkillShape::Single,false},
 {"붕권",0,20,18,1,2,SkillShape::Single,false},
 {"회선각",0,30,12,1,0,SkillShape::Cross,false},
 {"연타",1,15,18,1,0,SkillShape::Single,false},
 {"비연각",1,20,14,2,1,SkillShape::Single,false},
 {"점혈",1,25,10,1,0,SkillShape::Single,true},
 {"기탄",2,20,14,3,0,SkillShape::Single,false},
 {"기공파",2,40,20,4,0,SkillShape::Line,false},
 {"응급처치",2,15,18,2,0,SkillShape::Heal,false},
 {"쌍룡격",0,40,24,1,1,SkillShape::Single,false}
}};
inline constexpr int kInitialSkills=(1<<0)|(1<<3)|(1<<6)|(1<<8);
inline constexpr std::array<const char*,4> kTrainingNames{"산길 달리기","폭포 수련","무태두 대련","학선인 대련"};
inline constexpr std::array<const char*,3> kGearNames{"낡은 도복","중량 등껍질","손목 보호대"};
struct Lesson {
 const char* name; const char* briefing; const char* speaker; const char* dialogue;
 int roundLimit,enemyCount;
 std::array<const char*,7> tiles;
 std::array<Cell,3> starts;
 std::array<Cell,4> enemies;
 std::array<int,4> kinds;
};
inline constexpr std::array<Lesson,kLessons> kLessonData{{
 {"산길의 세 사람","도둑을 격퇴하세요. 로시가 쓰러지면 실패합니다.","무태두","힘은 누구를 위해 쓰느냐에 따라 달라진다.",14,2,
 {".......",".......",".......",".......",".......",".......","......."}, {{{2,5},{3,5},{4,5}}}, {{{3,2},{4,2},{-1,-1},{-1,-1}}}, {0,0,0,0}},
 {"도장의 입문 시험","벽으로 밀고 동료의 추격을 연결하세요.","학선인","네가 길을 열어. 마지막 한 방은 내가 맡겠다.",15,3,
 {".......",".##.##.",".......",".#.....",".......",".......","......."}, {{{2,5},{3,5},{4,5}}}, {{{3,2},{4,2},{5,3},{-1,-1}}}, {1,0,3,0}},
 {"계곡의 빛","기공사의 사선과 기둥을 활용하세요.","란란","기는 멀리 뻗지만 바위를 뚫고 가진 못해.",16,3,
 {".......",".......",".#...#.","...#...",".#...#.",".......","......."}, {{{2,5},{3,5},{4,5}}}, {{{2,1},{4,1},{3,2},{-1,-1}}}, {2,2,1,0}},
 {"엇갈리는 발걸음","네 적의 틈을 나누어 공략하세요.","로시","혼자 이기려다 모두 놓칠 뻔했어. 이번엔 같이 가자.",17,4,
 {".......","..#.#..",".......",".#...#.",".......",".......","......."}, {{{2,5},{3,5},{4,5}}}, {{{2,2},{3,2},{4,2},{5,1}}}, {3,0,1,2}},
 {"함께 여는 길","기술과 위치를 연결해 마지막 대련을 마치세요.","무태두","배운 것을 잊지 말고, 곁에 선 사람을 보아라.",18,4,
 {".......",".##.##.",".......",".......",".#...#.",".......","......."}, {{{2,5},{3,5},{4,5}}}, {{{2,2},{3,2},{4,2},{5,2}}}, {1,3,2,0}}
}};
}
