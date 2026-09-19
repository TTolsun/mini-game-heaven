#include "app/castle/CastleModel.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <limits>
#include <sstream>
using namespace app::castle;
void finish(CastleModel& model, float dt=1.0f/60) {
    for(int i=0;i<60000 && model.phase==Phase::Raid;++i) model.update(dt);
    assert(model.phase==Phase::Result);
}
int main(int argc,char** argv) {
    const std::string file=std::string(argc>1?argv[1]:".")+"/castle-test-save.txt";
    CastleModel m;
    assert(m.monsterCount==2 && m.roomCount()==6);
    const int gold=m.gold; assert(!m.build(-1,Facility::Den)); assert(!m.build(0,Facility::Den)); assert(m.gold==gold);
    assert(!m.dig(9)); assert(m.dig(6)); assert(m.roomCount()==7);
    assert(m.build(2,Facility::Library)); assert(m.build(3,Facility::Trap)); assert(m.trapDamage(3)==22);
    CastleModel growth;
    assert(!growth.fuse(0,1));
    assert(growth.train(0)); assert(growth.train(1));
    assert(growth.fusionResult(0,1)==3); assert(growth.fusionResult(1,0)==3);
    const int oldRoom=growth.monsters[1].room;
    assert(growth.fuse(1,0)); assert(growth.monsterCount==1);
    assert(growth.monsters[0].species==Species::Shade && growth.monsters[0].level==2 && growth.monsters[0].room==oldRoom);
    assert(growth.discovered==11 && growth.mana==25);
    assert(!growth.fuse(0,0)); assert(growth.summon(Species::Slime));
    assert(growth.assign(1,oldRoom)); assert(growth.monsters[0].room!=growth.monsters[1].room);
    assert(growth.save(file)); CastleModel loaded; assert(loaded.load(file));
    assert(loaded.gold==growth.gold && loaded.mana==growth.mana && loaded.discovered==growth.discovered);
    assert(loaded.monsters[0].room==growth.monsters[0].room);
    // Existing CASTLE 1 saves migrate to the original full serpentine route.
    {std::ofstream out(file); out<<"CASTLE 1\n100 97 2 2 0 3\n";
     for(int i=0;i<12;++i) out<<(i<6?1:0)<<" 0\n";
     out<<"0 5 1\n1 1 3\n";}
    CastleModel legacy; assert(legacy.load(file));
    assert(legacy.gold==100 && legacy.mana==97 && legacy.monsters[0].level==5);
    assert(legacy.throneRoom()==5 && legacy.invasionRoute().count==6);
    for(int i=0;i<6;++i) assert(legacy.invasionRoute().rooms[i]==i);
    assert(legacy.save(file)); assert(loaded.load(file));
    assert(loaded.invasionRoute().count==6 && loaded.monsters[0].level==5);
    // A shortcut changes actual battle contacts: the same defenders are bypassed.
    CastleModel defended;
    defended.day=5; defended.monsterCount=1; defended.monsters[0]={Species::Dragon,10,1};
    defended.rooms[0].facility=Facility::Empty;
    CastleModel shortcut=defended;
    assert(shortcut.togglePassage(0,5));
    assert(shortcut.invasionRoute().count==2 && shortcut.invasionRoute().rooms[1]==5);
    assert(defended.startRaid()); assert(shortcut.startRaid());
    assert(!shortcut.togglePassage(0,5)); assert(!shortcut.moveThrone(1)); assert(!shortcut.dig(6));
    finish(defended); finish(shortcut);
    assert(defended.won && defended.defeated==defended.totalHeroes);
    assert(!shortcut.won && shortcut.defeated==0);
    std::printf("ROUTE BALANCE: protected path kills=%d heart=%d; shortcut kills=%d heart=%d\n",
        defended.defeated,defended.heart,shortcut.defeated,shortcut.heart);
    CastleModel paths;
    assert(!paths.togglePassage(-1,0) && !paths.togglePassage(0,2) && !paths.togglePassage(0,0));
    assert(!paths.moveThrone(0) && !paths.moveThrone(11));
    assert(paths.togglePassage(0,1)); // Editing can temporarily disconnect the throne.
    assert(paths.invasionRoute().count==0 && !paths.startRaid());
    assert(paths.phase==Phase::Build && paths.rewardGold==0);
    assert(paths.save(file)); assert(loaded.load(file)); assert(!loaded.startRaid());
    assert(paths.togglePassage(0,5));
    assert(paths.invasionRoute().count==2);
    assert(paths.togglePassage(0,1)); // Cycle must not repeat rooms or farm traps.
    assert(paths.invasionRoute().count==2);
    assert(paths.dig(8)); // Non-consecutive expansion below room 3.
    assert(paths.rooms[8].open && !paths.rooms[6].open);
    assert(paths.moveThrone(8));
    assert(paths.summon(Species::Slime)); assert(paths.assign(2,8));
    assert(paths.save(file)); assert(loaded.load(file));
    assert(loaded.throneRoom()==8 && loaded.monsters[2].room==8);
    assert(loaded.invasionRoute().count==paths.invasionRoute().count);
    for(int i=0;i<paths.invasionRoute().count;++i) {
        assert(loaded.invasionRoute().rooms[i]==paths.invasionRoute().rooms[i]);
        for(int j=0;j<i;++j) assert(paths.invasionRoute().rooms[i]!=paths.invasionRoute().rooms[j]);
    }
    // A tampered asymmetric graph must not partially replace the current model.
    std::ifstream validSave(file); std::ostringstream savedText; savedText<<validSave.rdbuf(); validSave.close();
    const auto valid=savedText.str();
    const auto corrupt=[&](int row,const char* replacement) {
        std::istringstream source(valid); std::ofstream out(file); std::string line; int index=0;
        while(std::getline(source,line)) out<<(index++==row?replacement:line)<<'\n';
    };
    // Header + state + 12 rooms + 3 monsters + throne = row 18 is mask 0.
    corrupt(18,"0"); const int savedGold=loaded.gold;
    assert(!loaded.load(file) && loaded.gold==savedGold && loaded.throneRoom()==8);
    corrupt(18,"4096"); assert(!loaded.load(file));
    corrupt(17,"0"); assert(!loaded.load(file));
    {std::ofstream out(file); out<<valid<<"unexpected\n";}
    assert(!loaded.load(file));
    {std::ofstream out(file); out<<"CASTLE 1\n10 10 1 99 0 63\n";}
    const int before=loaded.gold; assert(!loaded.load(file)); assert(loaded.gold==before);
    assert(growth.save(file)); // atomic replacement of existing file
    CastleModel smallStep,largeStep;
    assert(smallStep.startRaid()); assert(largeStep.startRaid());
    assert(!smallStep.train(0)); assert(!smallStep.summon(Species::Imp));
    smallStep.update(std::numeric_limits<float>::quiet_NaN()); assert(smallStep.battleTime==0);
    finish(smallStep); finish(largeStep,1.0f/30);
    assert(smallStep.won==largeStep.won && smallStep.defeated==largeStep.defeated && smallStep.gold==largeStep.gold);
    assert(smallStep.save(file)); assert(loaded.load(file));
    assert(loaded.day==(smallStep.won?2:1)); assert(loaded.gold==smallStep.gold);
    const int reward=smallStep.gold; smallStep.update(10); assert(smallStep.gold==reward);
    smallStep.continueBuilding(); const int day=smallStep.day; smallStep.continueBuilding(); assert(smallStep.day==day);
    CastleModel loss; loss.day=10;
    for(auto& mon:loss.monsters) mon.room=5;
    assert(loss.startRaid()); finish(loss); assert(!loss.won);
    assert(loss.rewardGold>=40 && loss.rewardMana>=20 && loss.monsterCount==2);
    loss.continueBuilding(); assert(loss.day==10 && loss.heart==100);
    CastleModel campaign; int attempts=0;
    while(!campaign.completed && attempts<50) {
        // A simple viable strategy: discover a fusion, add residents, then feed
        // the lowest-level monster. No injected currency or levels.
        if(attempts==0) {assert(campaign.train(0));assert(campaign.train(1));assert(campaign.fuse(0,1));}
        if(campaign.monsterCount<6 && campaign.mana>=60) campaign.summon(Species::Golem);
        while(true) {
            int weakest=0;
            for(int i=1;i<campaign.monsterCount;++i) if(campaign.monsters[i].level<campaign.monsters[weakest].level) weakest=i;
            if(!campaign.train(weakest)) break;
        }
        assert(campaign.startRaid()); finish(campaign); ++attempts;
        std::printf("day=%d won=%d heroes=%d kills=%d gold=%d mana=%d\n",campaign.day,campaign.won,campaign.totalHeroes,campaign.defeated,campaign.gold,campaign.mana);
        campaign.continueBuilding();
    }
    assert(campaign.completed); assert(campaign.save(file)); assert(loaded.load(file)); assert(loaded.completed);
    assert(!loaded.startRaid());
    CastleModel collection; collection.gold=9999; collection.mana=9999;
    assert(collection.summon(Species::Slime)); assert(collection.train(0)); assert(collection.train(2));
    assert(collection.fuse(0,2)); assert(collection.monsters[0].species==Species::KingSlime);
    assert(collection.summon(Species::Golem)); assert(collection.train(1)); assert(collection.train(2));
    assert(collection.fuse(1,2)); assert(collection.monsters[1].species==Species::Dragon);
    assert((collection.discovered & 0x35)==0x35);
    CastleModel splash;
    splash.monsterCount=1; splash.monsters[0]={Species::Dragon,2,0}; splash.startRaid();
    splash.spawned=splash.totalHeroes;
    splash.heroes.fill({});
    splash.heroes[0]={0.1f,100,100,1,0,true}; splash.heroes[1]={0.2f,100,100,1,0,true};
    splash.update(1.0f/60);
    assert(splash.heroes[0].hp<100 && splash.heroes[1].hp<100);
    CastleModel regen;
    regen.monsterCount=1; regen.monsters[0]={Species::KingSlime,2,4}; regen.startRaid();
    regen.monsters[0].hp=20; regen.update(0.1f); assert(regen.monsters[0].hp>20);
    regen.monsters[0].hp=0; regen.update(0.1f); assert(regen.monsters[0].hp==0);
    auto start=std::chrono::steady_clock::now();
    for(int run=0;run<100;++run) {CastleModel battle; battle.day=10; battle.startRaid(); finish(battle);}
    auto duration=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    std::printf("PASS: editable paths, shortest route combat, disconnected raid guard, v1/v2 saves, malformed graphs, economy, 3 fusions, replay and 10-day campaign (%d attempts). 100 raid simulations %.1f ms\n",attempts,duration);
    std::remove(file.c_str());
}
