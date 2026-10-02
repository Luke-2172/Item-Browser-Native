#pragma once
#include <array>
#include <cstddef>
// ABI adapter for FalloutNV.exe 1.4.0.525 only. Addresses/layout verified against
// xNVSE GameTiles.h/.cpp and GameUI.cpp. Calls game methods; patches no code.
namespace nativeui {
struct Tile;
struct Value {uint32_t id;Tile* owner;float number;char* string;void* action;};
struct Node {Node* next;Node* previous;Tile* tile;};
struct Tile {
 void** vtable;Node* first;Node* last;uint32_t childCount;
 void* arrayVtable;Value** values;uint32_t valueCount,capacity;
 char* name;uint16_t nameLength,nameCapacity;Tile* parent;
};
static_assert(offsetof(Tile,values)==0x14&&offsetof(Tile,name)==0x20&&offsetof(Tile,parent)==0x28,"Tile ABI");
using SetNumber=void(__thiscall*)(Tile*,uint32_t,float,bool);
using SetText=void(__thiscall*)(Tile*,uint32_t,const char*,bool);
using Trait=uint32_t(__cdecl*)(const char*);
inline SetNumber setNumber=(SetNumber)0xA012D0;
inline SetText setText=(SetText)0xA01350;
inline Trait trait=(Trait)0xA01860;
inline uint32_t id(const char* name){static std::map<std::string,uint32_t> cache;auto i=cache.find(name);if(i!=cache.end())return i->second;auto n=trait(name);cache[name]=n;return n;}
inline Value* value(Tile* t,uint32_t key){
 if(!t||!t->values||t->valueCount>4096)return nullptr;
 uint32_t lo=0,hi=t->valueCount;
 while(lo<hi){auto m=(lo+hi)/2;auto v=t->values[m];if(!v)return nullptr;if(v->id==key)return v;if(v->id<key)lo=m+1;else hi=m;}
 return nullptr;
}
inline float get(Tile* t,const char* key,float fallback=0){auto v=value(t,id(key));return v?v->number:fallback;}
inline void number(Tile* t,const char* key,float n){if(!t)return;auto keyID=id(key);auto v=value(t,keyID);if(v&&v->number!=n)setNumber(t,keyID,n,true);}
inline void string(Tile* t,const std::string& text){if(!t)return;auto key=id("string");auto v=value(t,key);if(v&&(!v->string||text!=v->string))setText(t,key,text.c_str(),true);}
inline Tile* child(Tile* t,const char* name){
 if(!t)return nullptr;unsigned count=0;
 for(auto node=t->first;node&&count++<4096;node=node->next)
  if(node->tile&&node->tile->name&&!strcmp(node->tile->name,name))return node->tile;
 return nullptr;
}
inline Tile* hud(){auto data=*(Tile***)0x11F350C;return data?data[3]:nullptr;}
constexpr size_t Pool=256;
struct Primitive {bool text=false;float x=0,y=0,w=0,h=0;COLORREF color=0;int size=22;bool heading=false,center=false;std::string label;};
inline std::vector<Primitive> commands;
inline bool overflow=false;
inline void begin(){commands.clear();overflow=false;}
inline void rect(int x,int y,int w,int h,COLORREF c){if(commands.size()>=Pool){overflow=true;return;}commands.push_back({false,(float)x,(float)y,(float)w,(float)h,c});}
inline void text(int x,int y,int w,int h,COLORREF c,int size,bool heading,bool center,std::string label){
 if(commands.size()>=Pool){overflow=true;return;}if(label.size()>512)label.resize(512);
 commands.push_back({true,(float)x,(float)y,(float)w,(float)h,c,size,heading,center,std::move(label)});
}

struct alignas(16) Dimensions {float width=0,height=0,lines=0,padding=0;};
using Measure=bool(*)(const std::string&,int,Dimensions&);
inline bool engineMeasure(const std::string& text,int font,Dimensions& out){
 // Same game method used by JIP GetStringUIDimensions; not a JIP hook.
 auto manager=*(void**)0x11F33F8;if(!manager||font<1||font>8)return false;
 using Fn=Dimensions*(__thiscall*)(void*,Dimensions*,const char*,uint32_t,float,uint32_t);
 ((Fn)0xA1B020)(manager,&out,text.c_str(),font,3.402823466e+38F,0);
 return std::isfinite(out.width)&&std::isfinite(out.height)&&out.width>=0&&out.height>0;
}
inline Measure measure=engineMeasure;
struct Fitted {std::string label;float zoom=1,width=0,height=0;};
inline bool fit(const Primitive& p,int font,float scale,Fitted& out){
 out.label=p.label;
 for(char& c:out.label)if(c=='\r'||c=='\n'||c=='\t')c=' ';
 Dimensions base{},dims{};if(!measure("Ag",font,base)||base.height<=0)return false;
 float available=std::max(0.f,(p.w-4)*scale);
 out.zoom=std::max(0.01f,std::min((float)p.size,std::max(1.f,p.h-2))*scale/base.height);
 if(out.label.empty()){out.height=base.height*out.zoom;return true;}
 if(!measure(out.label,font,dims))return false;
 if(p.center&&dims.width*out.zoom>available&&dims.width>0)
  out.zoom=std::max(out.zoom*.70f,available/dims.width);
 if(dims.width*out.zoom>available){
  std::string original=out.label;
  size_t low=0,high=original.size();
  while(low<high){size_t mid=(low+high+1)/2;Dimensions candidate{};
   if(!measure(original.substr(0,mid)+"...",font,candidate))return false;
   if(candidate.width*out.zoom<=available)low=mid;else high=mid-1;
  }
  out.label=original.substr(0,low)+"...";
  if(!measure(out.label,font,dims))return false;
  if(dims.width*out.zoom>available){out.label.clear();dims.width=0;}
 }
 out.width=dims.width*out.zoom;out.height=base.height*out.zoom;return true;
}


struct View {
 Tile* root=nullptr;Tile* background=nullptr;Tile* cursor=nullptr;
 std::array<Tile*,Pool> rects{},texts{};
 float scale=1;int bodyFont=3,headingFont=2;
 float bodyBaseline=0.25f,headingBaseline=0.33f;
 void forget(){root=background=cursor=nullptr;rects.fill(nullptr);texts.fill(nullptr);}
 bool bind(Tile* next){
  if(next==root&&root)return true;forget();if(!next)return false;root=next;
  background=child(root,"Background");cursor=child(root,"Cursor");
  for(size_t i=0;i<Pool;++i){auto n=std::to_string(i);rects[i]=child(root,("R"+n).c_str());texts[i]=child(root,("T"+n).c_str());if(!rects[i]||!texts[i]){forget();return false;}}
  if(!background||!cursor){forget();return false;}return true;
 }
 void hide(){number(root,"visible",0);}
 bool layout(){
  float w=get(root,"_sw"),h=get(root,"_sh");
  if(!std::isfinite(w)||!std::isfinite(h)||w<320||h<200||w>16384||h>16384)return false;
  scale=std::min(w/1150.f,h/730.f);
  number(root,"x",(w-1120*scale)/2);number(root,"y",(h-700*scale)/2);
  number(root,"width",1120*scale);number(root,"height",700*scale);
  number(background,"width",1120*scale);number(background,"height",700*scale);
  return true;
 }
 void tint(Tile* t,uint32_t rgb,float brightness){
  number(t,"red",((rgb>>16)&255)*brightness);number(t,"green",((rgb>>8)&255)*brightness);number(t,"blue",(rgb&255)*brightness);
 }
 bool paint(uint32_t rgb,int opacity){
  if(!root||overflow||!layout())return false;
  tint(background,rgb,1);number(background,"alpha",opacity*2.55f);
  for(size_t i=0;i<Pool;++i){
   bool used=i<commands.size();number(rects[i],"visible",used&&!commands[i].text?1:0);number(texts[i],"visible",used&&commands[i].text?1:0);
   if(!used)continue;auto& p=commands[i];auto t=p.text?texts[i]:rects[i];
   float brightness=std::max({GetRValue(p.color),GetGValue(p.color),GetBValue(p.color)})/255.f;tint(t,rgb,brightness);
   if(p.text){
    int font=p.heading?headingFont:bodyFont;Fitted fitted;if(!fit(p,font,scale,fitted))return false;
    number(t,"font",(float)font);number(t,"zoom",100*fitted.zoom);
    // Native justify=1 is left. Position explicitly using measured glyph width.
    number(t,"justify",1);number(t,"wrapwidth",100000);number(t,"wraplimit",1);
    number(t,"x",p.x*scale+(p.center?std::max(0.f,(p.w*scale-fitted.width)*.5f):0));
    // The game font line box includes leading above/below the visible glyphs.
    // Calibrated to the supplied in-game screenshot, in scaled text-height units.
    float baseline=p.center?fitted.height*(p.heading?headingBaseline:bodyBaseline):0.f;
    number(t,"y",p.y*scale+std::max(0.f,(p.h*scale-fitted.height)*.5f)+baseline);
    number(t,"depth",300.f+(float)i);string(t,fitted.label);
   }else{
    number(t,"x",p.x*scale);number(t,"y",p.y*scale);number(t,"width",p.w*scale);number(t,"height",p.h*scale);
    number(t,"depth",1.f+(float)i);
    number(t,"alpha",p.w>6&&p.h>6?110.f:255.f);
   }
  }
  number(root,"visible",1);return true;
 }
 void pointer(float x,float y,bool controller,uint32_t rgb){
  number(cursor,"visible",controller?0.f:1.f);number(cursor,"x",x*scale);number(cursor,"y",y*scale);
  number(cursor,"width",10*scale);number(cursor,"height",17*scale);tint(cursor,rgb,1);
 }
};
}

