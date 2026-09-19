#pragma once
#include "engine/asset/Image.h"
#include "engine/graphics/SpriteBatch.h"
namespace app::srpg {
enum class Pose { Idle, Walk, Attack, Hit, Skill, Kick };
// Modular pixel source: palette, body, hair and accessories share pose frames.
class BattleArt {
public:
 bool load(engine::AssetLoader& assets);
 void portrait(engine::SpriteBatch& batch,int person,engine::Rect area,engine::Color tint=engine::Color::white()) const;
 void unit(engine::SpriteBatch& batch,int costume,int direction,Pose pose,float time,engine::Vec2 feet,float scale,engine::Color tint=engine::Color::white()) const;
 void tile(engine::SpriteBatch& batch,int kind,engine::Rect area) const;
 void dojo(engine::SpriteBatch& batch,engine::Rect area) const;
 static engine::Image makeUnits();
 static engine::Image makeTiles();
private:
 engine::Texture portraits_,units_,tiles_,dojo_;
 static engine::Sprite region(const engine::Texture& texture,int x,int y,int width,int height);
};
}
