#include "app/srpg/BattleArt.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace app::srpg {
namespace {
using Pixel=uint32_t;
constexpr Pixel outline=0x25333fff,skin=0xe5b47eff,skinLight=0xffd49bff,skinShade=0xb97957ff,hair=0x222a32ff;
struct Canvas {
 engine::Image image;
 int ox=0,oy=0,cw,ch;
 explicit Canvas(int w,int h):image{w,h,std::vector<uint8_t>(w*h*4)},cw(w),ch(h) {}
 void pixel(int x,int y,Pixel c) {
  if(x<0 || y<0 || x>=cw || y>=ch) return;
  const auto p=static_cast<size_t>(((y+oy)*image.width+x+ox)*4);
  image.pixels[p]=c>>24; image.pixels[p+1]=(c>>16)&255; image.pixels[p+2]=(c>>8)&255; image.pixels[p+3]=c&255;
 }
 void rect(int x,int y,int w,int h,Pixel c) { for(int j=y;j<y+h;++j) for(int i=x;i<x+w;++i) pixel(i,j,c); }
 void ellipse(int x,int y,int rx,int ry,Pixel c) {
  for(int j=-ry;j<=ry;++j) for(int i=-rx;i<=rx;++i) if(i*i*ry*ry+j*j*rx*rx<=rx*rx*ry*ry) pixel(x+i,y+j,c);
 }
 void line(int x,int y,int x2,int y2,Pixel c,int size=1) {
  const int steps=std::max(std::abs(x2-x),std::abs(y2-y));
  for(int i=0;i<=steps;++i) rect(x+(x2-x)*i/std::max(1,steps),y+(y2-y)*i/std::max(1,steps),size,size,c);
 }
 void triangle(int x,int y,int width,int height,Pixel c) {
  for(int row=0;row<height;++row) { const int half=width*row/std::max(1,height)/2; rect(x-half,y+row,half*2+1,1,c); }
 }
};
struct Costume { Pixel dark,main,light,belt; int body,head; };
// Body 0 boy, 1 slender, 2 broad, 3 elder, 4 female. Shared limbs and motions.
constexpr std::array<Costume,8> costumes{{
 {0x994e38ff,0xe9924fff,0xffbf75ff,0x427c7aff,0,0},
 {0x416567ff,0xe5ddbcff,0xfff0cdff,0x315659ff,0,1},
 {0x3b6484ff,0x6c9cb0ff,0xa7caccff,0xe3ce99ff,4,2},
 {0x534660ff,0x856886ff,0xb89bafff,0xddc299ff,3,3},
 {0x663d39ff,0xad654aff,0xd58b59ff,0xddd09bff,2,4},
 {0x2a454dff,0x4c6a68ff,0x849586ff,0xa17c53ff,2,5},
 {0x693e49ff,0xa26058ff,0xd0916cff,0x334651ff,1,1},
 {0x796042ff,0xbb9856ff,0xe7c87dff,0x48676cff,1,6}
}};
constexpr std::array<int,6> frameStart{0,4,10,16,19,27},frameCount{4,6,6,3,8,6};
void person(Canvas& c,int costume,int direction,Pose pose,int frame) {
 const auto& p=costumes[costume];
 const bool side=direction==1 || direction==3,back=direction==2;
 const int swing=pose==Pose::Walk?std::array<int,6>{-3,-1,2,3,1,-2}[frame]:0;
 const int bob=pose==Pose::Idle?(frame==1 || frame==2):pose==Pose::Walk?(frame%3==0?-1:0):0;
 const int lean=pose==Pose::Hit?-3:pose==Pose::Attack?(frame>=2 && frame<=4?3:0):0;
 const int x=24+lean,y=33+bob,w=p.body==2?11:p.body==4?7:8;
 // Rear arm, boots, cloth silhouette, then front arm: common rig.
 c.line(x-w-2,y+1,x-w-4,y+15,outline,5);
 c.line(x-w-1,y+2,x-w-3,y+12,p.dark,3);
 c.rect(x-10-swing/2,51+swing,9,7,outline); c.rect(x+2+swing/2,51-swing,10,7,outline);
 c.rect(x-9-swing/2,52+swing,6,2,p.belt); c.rect(x+3+swing/2,52-swing,7,2,p.belt);
 c.line(x-5,y+11,x-7-swing/2,51+swing,p.dark,7); c.line(x+3,y+11,x+4+swing/2,51-swing,p.main,7);
 c.line(x-4,y+12,x-6-swing/2,50+swing,p.main,3); c.line(x+4,y+13,x+5+swing/2,50-swing,p.light,2);
 c.rect(x-w-1,y-2,2*w+2,18,outline); c.rect(x-w,y-3,2*w,17,p.main);
 c.rect(x-w,y,3,13,p.dark); c.rect(x+w-3,y,2,12,p.light);
 if(!back) { c.line(x-6,y-3,x+3,y+6,p.light,3); c.line(x+5,y-3,x-5,y+9,p.dark,2); }
 c.rect(x-w,y+11,2*w,4,p.belt); c.rect(x+1,y+15,3,8,p.belt);
 if(costume==5) for(int row=0;row<3;++row) for(int col=0;col<4;++col) { c.rect(x-9+col*5,y+row*4,4,3,p.dark); c.pixel(x-8+col*5,y+row*4,p.light); }
 int armX=x+w,armY=y+8;
 if(pose==Pose::Attack) { const int reach=std::array<int,6>{0,3,11,11,5,0}[frame]; armX=std::min(42,x+w+reach); armY=y-1; }
 if(pose==Pose::Skill) { armX=x+w+2; armY=y-5+(frame%2); }
 c.line(x+w-2,y,armX,armY,outline,5); c.line(x+w-1,y+1,armX,armY,p.main,3);
 c.ellipse(armX+1,armY+3,3,4,skinShade); c.ellipse(armX,armY+2,2,3,skinLight);
 c.ellipse(x,y-13,10,12,outline); c.ellipse(x,y-13,9,11,skinShade); c.ellipse(x-1,y-15,8,9,skin);
 c.ellipse(x-3,y-17,5,6,skinLight); c.rect(x-2,y-3,5,3,skinShade);
 if(p.head==3 || p.head==5) {
  c.ellipse(x-3,y-21,4,2,skinLight);
 } else {
  c.ellipse(x,y-23,10,5,hair); c.rect(x-10,y-23,3,12,hair);
  if(p.head==0 || p.head==4) { for(int i=0;i<5;++i) c.triangle(x-9+i*4,y-33+(i%2)*3,8,13,hair); c.line(x-8,y-26,x-1,y-29,0x47515aff,2); }
  if(p.head==1) { c.ellipse(x+1,y-31,4,4,hair); c.rect(x-2,y-29,7,2,p.belt); c.line(x+7,y-24,x+10,y-11,hair,2); }
  if(p.head==2) { c.rect(x-11,y-23,4,20,hair); c.rect(x+7,y-23,4,20,hair); c.rect(x-13,y-21,4,6,0x8bb898ff); c.rect(x+10,y-20,4,6,0x8bb898ff); }
  if(p.head==4) c.rect(x-10,y-23,20,3,p.light);
 }
 if(back) { c.ellipse(x,y-15,9,9,p.head==3 || p.head==5?skin:hair); }
 else {
  const int eyeX=side?x+3:x-5;
  c.rect(eyeX,y-16,3,3,outline); c.pixel(eyeX,y-16,0xfff0d9ff);
  if(!side) { c.rect(x+4,y-16,3,3,outline); c.pixel(x+4,y-16,0xfff0d9ff); }
  c.rect(side?x+7:x,y-11,3,2,skinShade); c.rect(x-1,y-7,4,1,outline);
  if(p.head==3) { c.rect(x-8,y-19,6,2,0xe5ded0ff); c.rect(x+3,y-19,6,2,0xe5ded0ff); c.line(x,y-10,x-11,y-5,0xe5ded0ff,3); c.line(x,y-10,x+9,y-5,0xe5ded0ff,3); }
 }
 if(p.head==6) { c.triangle(x,y-33,28,12,outline); c.triangle(x,y-32,25,10,p.light); c.rect(x-15,y-22,31,3,p.dark); }
 if(pose==Pose::Kick && frame>=2 && frame<=4) {
  c.line(x+3,y+15,43,y+10,outline,6);c.line(x+4,y+15,42,y+10,p.main,4);c.rect(40,y+8,7,5,outline);
 }
 // Mirror the side pose to obtain west with exactly matching proportions.
 if(direction==3) for(int yy=0;yy<64;++yy) for(int xx=0;xx<24;++xx) {
  const int a=((c.oy+yy)*c.image.width+c.ox+xx)*4,b=((c.oy+yy)*c.image.width+c.ox+47-xx)*4;
  for(int channel=0;channel<4;++channel) std::swap(c.image.pixels[a+channel],c.image.pixels[b+channel]);
 }
}
engine::Image makeDojo() {
 Canvas c(192,112);
 c.ellipse(96,104,88,6,0x233d3b88);
 c.rect(22,50,148,48,0x3f4240ff); c.rect(27,53,138,40,0xd3b784ff);
 c.rect(31,56,130,28,0x977d58ff);
 for(int x=34;x<160;x+=8) { c.rect(x,57,5,26,0xd8c499ff); c.rect(x+2,57,1,26,0xf0dbacff); }
 c.rect(76,53,40,42,0x35464aff); c.rect(80,56,14,36,0x66726aff); c.rect(98,56,14,36,0x66726aff);
 for(int x:{24,66,120,162}) { c.rect(x,47,6,51,0x654838ff); c.rect(x,48,2,48,0xa77b4eff); }
 c.rect(16,96,160,5,0x877c65ff); c.rect(11,101,170,5,0xb9a885ff); c.rect(6,107,180,5,0x6f7465ff);
 for(int row=0;row<39;++row) {
  const int inset=34-row*3/4; c.rect(inset,12+row,192-2*inset,1,row%6==0?0x9aab91ff:row%6==1?0x556e68ff:0x365358ff);
  if(row%6>1) for(int x=inset+5;x<192-inset;x+=10) c.rect(x,12+row,1,1,0x628177ff);
 }
 c.rect(18,10,156,4,0x304b4fff); c.rect(13,7,11,7,0x456b64ff); c.rect(168,7,11,7,0x456b64ff);
 c.rect(4,49,184,5,0x243b43ff); c.rect(1,45,9,7,0x416660ff); c.rect(182,45,9,7,0x416660ff);
 c.rect(80,39,32,14,0x684735ff); c.rect(83,41,26,10,0xd9bd78ff);
 c.line(87,44,103,44,0x6a563eff); c.line(93,43,93,49,0x6a563eff); c.line(102,43,99,49,0x6a563eff);
 return std::move(c.image);
}
}
engine::Image BattleArt::makeUnits() {
 Canvas c(33*48,8*4*64); c.cw=48; c.ch=64;
 for(int style=0;style<8;++style) for(int direction=0;direction<4;++direction) for(int pose=0;pose<6;++pose) for(int frame=0;frame<frameCount[pose];++frame) {
  c.ox=(frameStart[pose]+frame)*48; c.oy=(style*4+direction)*64;
  person(c,style,direction,static_cast<Pose>(pose),frame);
 }
 return std::move(c.image);
}
engine::Image BattleArt::makeTiles() {
 Canvas c(48*8,48*4); c.cw=c.ch=48;
 for(int kind=0;kind<32;++kind) {
  c.ox=(kind%8)*48; c.oy=(kind/8)*48;
  const bool water=kind>=8 && kind<12,path=kind>=4 && kind<8;
  c.rect(0,0,48,48,water?0x356975ff:path?0xb2a479ff:0x537762ff);
  // Stable variation hashes avoid shimmer and repeated checkerboard patterns.
  for(int i=0;i<80;++i) {
   const int x=(i*17+kind*13)%48,y=(i*29+i*i+kind*7)%48;
   if(water) c.rect(x,y,5+(i%7),1,i%3?0x4d8992ff:0x80b4aeff);
   else if(path) c.rect(x,y,1+i%3,1,i%3?0xc1b187ff:0x938e6bff);
   else { c.rect(x,y,2,1,i%3?0x66866aff:0x416953ff); if(i%7==0)c.line(x,y,x+1,y-3,0x89a578ff); }
  }
  if(kind==12) { // Rock with a strong collision silhouette.
   c.ellipse(25,38,21,8,0x304e4588); c.ellipse(24,25,19,17,0x394e4dff);
   c.ellipse(23,22,17,14,0x728178ff); c.ellipse(18,18,11,8,0x98a28cff);
   c.line(28,11,23,29,0x57695fff,2); c.line(23,29,37,32,0x57695fff,2); c.rect(8,34,10,3,0x819567ff);
  } else if(kind==13 || kind==14) {
   c.ellipse(25,40,21,6,0x274638aa); c.rect(21,25,8,19,0x534b38ff); c.rect(22,28,3,15,0x8d7850ff);
   c.ellipse(24,23,22,18,0x2b5347ff); c.ellipse(15,19,13,14,0x376550ff); c.ellipse(31,18,15,13,0x477858ff);
   c.ellipse(22,10,15,10,0x5f8a62ff); c.ellipse(18,9,10,6,0x7a9c6aff);
   c.line(8,22,13,19,0x66875eff,2); c.line(28,12,35,17,0x83a36bff,2);
   if(kind==14) { c.triangle(24,1,34,21,0x396c51ff); c.triangle(24,1,26,15,0x7f9e6aff); c.triangle(24,13,43,21,0x2b5846ff); c.triangle(22,14,32,16,0x548363ff); }
  } else if(kind==15) {
   for(int i=0;i<8;++i) { const int x=4+i*5,y=10+(i*13)%30; c.line(x,y+5,x,y,0x315b47ff); c.rect(x-1,y,3,2,0xe4c291ff); c.pixel(x,y,0xffe9bdff); }
  } else if(kind>=16) {
   // Four-neighbour grass banks. Adjacent water bits N/E/S/W select the edges.
   const int mask=kind-16;
   if(mask&1) {c.rect(0,0,48,6,0x356975ff);c.rect(0,6,48,2,0xc1b68bff);}
   if(mask&2) {c.rect(42,0,6,48,0x356975ff);c.rect(40,0,2,48,0xc1b68bff);}
   if(mask&4) {c.rect(0,42,48,6,0x356975ff);c.rect(0,40,48,2,0xc1b68bff);}
   if(mask&8) {c.rect(0,0,6,48,0x356975ff);c.rect(6,0,2,48,0xc1b68bff);}
  }
 }
 return std::move(c.image);
}
engine::Sprite BattleArt::region(const engine::Texture& texture,int x,int y,int w,int h) {
 if(!texture.isValid()) return {};
 const float tw=static_cast<float>(texture.width()),th=static_cast<float>(texture.height());
 // Half-pixel inset also prevents linear portrait atlas neighbours bleeding.
 return {&texture,(x+0.5f)/tw,(y+0.5f)/th,(x+w-0.5f)/tw,(y+h-0.5f)/th,static_cast<float>(w),static_cast<float>(h)};
}
bool BattleArt::load(engine::AssetLoader& assets) {
 const bool portraits=portraits_.create(engine::Image::load(assets,"art/portraits.png"));
 const bool units=units_.create(makeUnits()),tiles=tiles_.create(makeTiles()),dojo=dojo_.create(makeDojo());
 if(units)units_.setNearest();
 if(tiles)tiles_.setNearest();
 if(dojo)dojo_.setNearest();
 return portraits && units && tiles && dojo;
}
void BattleArt::portrait(engine::SpriteBatch& b,int person,engine::Rect r,engine::Color tint) const {
 if(person<0 || person>8)return;
 const int w=portraits_.width()/3,h=portraits_.height()/3;
 b.draw(region(portraits_,(person%3)*w,(person/3)*h,w,h),{r.x+r.w/2,r.y+r.h/2},{r.w,r.h},tint);
}
void BattleArt::unit(engine::SpriteBatch& b,int costume,int direction,Pose pose,float time,engine::Vec2 feet,float scale,engine::Color tint) const {
 const int p=static_cast<int>(pose),frame=static_cast<int>(std::max(0.0f,time)*(pose==Pose::Idle?5:14))%frameCount[p];
 b.draw(region(units_,(frameStart[p]+frame)*48,(std::clamp(costume,0,7)*4+std::clamp(direction,0,3))*64,48,64),{feet.x,feet.y-32*scale},{48*scale,64*scale},tint);
}
void BattleArt::tile(engine::SpriteBatch& b,int kind,engine::Rect r) const {
 kind=std::clamp(kind,0,31); b.draw(region(tiles_,(kind%8)*48,(kind/8)*48,48,48),{r.x+r.w/2,r.y+r.h/2},{r.w,r.h});
}
void BattleArt::dojo(engine::SpriteBatch& b,engine::Rect r) const { b.draw(region(dojo_,0,0,192,112),{r.x+r.w/2,r.y+r.h/2},{r.w,r.h}); }
}
