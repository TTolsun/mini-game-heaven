// Emulator-only fixtures for visual QA. Never inject these into a player's save.
#include "app/srpg/BattleModel.h"
#include <cassert>
#include <string>
namespace app::srpg {
struct BattleModelTestAccess { static BattleState& state(BattleModel& m) { return m.state_; } };
}
using namespace app::srpg;
int main(int argc,char** argv) {
 assert(argc==2);const std::string root=argv[1];BattleModel model;
 assert(model.save(root+"/visual-story.txt"));assert(model.chooseStory(0));assert(model.save(root+"/visual-base.txt"));
 assert(model.prepare());assert(model.save(root+"/visual-deployment.txt"));assert(model.startBattle());
 auto& s=BattleModelTestAccess::state(model);s.skills=(1<<kSkills)-1;s.bond=2;
 s.units[0].cell={2,2};s.units[0].ki=100;s.units[1].cell={4,3};s.units[1].ki=100;s.units[2].cell={0,6};s.units[2].ki=100;
 s.units[3].cell={3,2};s.units[4].cell={6,0};assert(model.save(root+"/visual-combo.txt"));
 s.units[2].cell={1,4};s.units[3].cell={3,4};s.units[4].cell={4,4};assert(model.save(root+"/visual-beam.txt"));
 s.units[0].health=40;assert(model.save(root+"/visual-heal.txt"));
 s.units[0].cell={1,3};s.units[3].cell={0,3};s.units[4].cell={5,2};assert(model.save(root+"/visual-collision.txt"));
}
