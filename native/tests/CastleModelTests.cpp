#include "app/castle/CastleModel.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <limits>
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
    assert(!m.dig(8)); assert(m.dig(6)); assert(m.roomCount()==7);
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
    std::printf("PASS: economy, placement, 3 fusions, persistence, corruption, replay, defeat recovery, 10-day campaign (%d attempts). 100 raid simulations %.1f ms\n",attempts,duration);
    std::remove(file.c_str());
}
