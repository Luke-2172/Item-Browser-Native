#include "browser.cpp"
#include <iostream>
#include <deque>
static bool dds(const std::string& path,unsigned w,unsigned h,const std::vector<uint32_t>& pixels){
 uint32_t header[32]{};header[0]=0x20534444;header[1]=124;header[2]=0x100F;header[3]=h;header[4]=w;header[5]=w*4;
 header[19]=32;header[20]=0x41;header[22]=32;header[23]=0x00FF0000;header[24]=0x0000FF00;header[25]=0x000000FF;header[26]=0xFF000000;header[27]=0x1000;
 std::ofstream out(path,std::ios::binary);out.write((char*)header,sizeof(header));out.write((char*)pixels.data(),pixels.size()*4);return (bool)out;
}
static std::map<std::string,uint32_t> traitIDs;
static uint32_t __cdecl mockTrait(const char* n){auto& id=traitIDs[n];if(!id)id=(uint32_t)traitIDs.size();return id;}
static unsigned writes=0;
static std::map<nativeui::Tile*,std::string> strings;
static void __fastcall mockNumber(nativeui::Tile* t,void*,uint32_t id,float v,bool){++writes;nativeui::value(t,id)->number=v;}
static void __fastcall mockString(nativeui::Tile* t,void*,uint32_t id,const char* v,bool){++writes;strings[t]=v;nativeui::value(t,id)->string=(char*)strings[t].c_str();}
struct MockTile {
 nativeui::Tile tile{};nativeui::Node node{};std::string name;std::vector<nativeui::Value> values;std::vector<nativeui::Value*> pointers;
 MockTile(const std::string& n):name(n){
  for(auto key:{"visible","x","y","width","height","red","green","blue","alpha","font","zoom","justify","wrapwidth","string","_sw","_sh","wraplimit","depth"})
   values.push_back({nativeui::id(key),&tile,0,nullptr,nullptr});
  std::sort(values.begin(),values.end(),[](auto& a,auto& b){return a.id<b.id;});
  for(auto& v:values)pointers.push_back(&v);
  tile.name=(char*)name.c_str();tile.values=pointers.data();tile.valueCount=(uint32_t)pointers.size();node.tile=&tile;
 }
};
static bool mockMeasure(const std::string& s,int font,nativeui::Dimensions& out){out={(float)s.size()*(font==2?16.f:12.f),28,1};return true;}
int main(){
 nativeui::measure=mockMeasure;
 plugins={L"Fixture.esm"};filteredPlugins={0};pluginIndex=0;selected=0;
 for(unsigned i=0;i<50;++i){catalog.items.push_back({i,
#ifdef ACTOR_BROWSER
 "NPC_",
#else
 "WEAP",
#endif
 "Fixture "+std::to_string(i),"Fixture",false});visible.push_back(i);}
 size_t peak=0;
 for(int page=0;page<3;++page)for(bool controller:{false,true}){
  settingsPage=page==1;
#ifdef ACTOR_BROWSER
  valuesPage=page==2;
#endif
  controllerPrompts=controller;nativeDrawing=true;drawCanvas();peak=std::max(peak,nativeui::commands.size());
  if(nativeui::overflow||nativeui::commands.empty())return 1;
  for(auto& p:nativeui::commands)if(!std::isfinite(p.x)||p.w<0||p.h<0)return 2;
 }
 // Direct engine calls are replaced only inside this test process.
 nativeui::trait=mockTrait;nativeui::setNumber=(nativeui::SetNumber)mockNumber;nativeui::setText=(nativeui::SetText)mockString;
 std::deque<MockTile> tiles;tiles.emplace_back("root");auto& rootTile=tiles.front().tile;nativeui::Node* last=nullptr;
 auto add=[&](std::string n){tiles.emplace_back(n);auto node=&tiles.back().node;if(last)last->next=node;else rootTile.first=node;node->previous=last;last=node;};
 add("Background");add("Cursor");for(size_t i=0;i<nativeui::Pool;++i){add("R"+std::to_string(i));add("T"+std::to_string(i));}
 nativeui::number(&rootTile,"_sw",1280);nativeui::number(&rootTile,"_sh",720);
 nativeui::View view;if(!view.bind(&rootTile))return 3;
 nativeui::begin();nativeui::rect(5,6,20,30,RGB(255,255,255));nativeui::text(20,30,600,25,RGB(255,255,255),22,false,true,"Quotes \" and XML <text> remain data.");
 if(!view.paint(0x20FF40,72))return 4;
  float textHeight=22*view.scale;
 float expectedY=30*view.scale+(25*view.scale-textHeight)*.5f+textHeight*.25f;
 if(std::abs(nativeui::get(view.texts[1],"y")-expectedY)>.01f)return 16;
 auto savedWrites=writes;if(!view.paint(0x20FF40,72)||savedWrites!=writes)return 5;
 if(strings[view.texts[1]]!="Quotes \" and XML <text> remain data.")return 6;
  if(nativeui::get(view.texts[1],"depth")<=nativeui::get(view.rects[0],"depth")||nativeui::get(view.texts[1],"justify")!=1)return 13;
 nativeui::Primitive longRow{true,37,190,290,28,0,22,false,false,"The Mod Configuration Menu.esp with extra words"};
 nativeui::Fitted fitted;if(!nativeui::fit(longRow,3,1,fitted)||fitted.width>286||fitted.height>26||fitted.label.find("...")==std::string::npos)return 14;
 for(float scale:{0.5f,1.f,2.f}){longRow.center=true;if(!nativeui::fit(longRow,2,scale,fitted)||fitted.width>286*scale+.01f||fitted.height>26*scale+.01f)return 15;}
 view.hide();if(nativeui::get(&rootTile,"visible")!=0)return 7;
 nativeui::number(&rootTile,"_sw",0);if(view.paint(0x20FF40,72))return 8;
 nativeui::begin();for(size_t i=0;i<=nativeui::Pool;++i)nativeui::rect(0,0,1,1,0);
 if(!nativeui::overflow||nativeui::commands.size()!=nativeui::Pool)return 9;
 // Build native DDS assets from the existing procedural background; never used
 // as a pre-rendered text/menu overlay. All controls/text are separate game tiles.
 nativeDrawing=false;renderScale=1;if(!createCanvas())return 10;buildBackdrop();
 std::vector<uint32_t> bg=backdrop;for(int y=0;y<Height;++y)for(int x=0;x<Width;++x)
  bg[y*Width+x]|=(x>=7&&x<Width-13&&y>=7&&y<Height-13)?0xFF000000:0;
#ifdef ACTOR_BROWSER
 const std::string dir="package/textures/Interface/LukesActorBrowser/";
#else
 const std::string dir="package/textures/Interface/LukesItemBrowser/";
#endif
 if(!dds(dir+"NativeBackground.dds",Width,Height,bg)||!dds(dir+"White.dds",1,1,{0xFFFFFFFF}))return 11;
 std::vector<uint32_t> cursor(32*32,0);
 for(int y=0;y<28;++y)for(int x=0;x<=y/2&&x<15;++x)cursor[y*32+x]=0xFFFFFFFF;
 if(!dds(dir+"Cursor.dds",32,32,cursor))return 12;
 std::cout<<"PASS: native tile command capture across pages; peak "<<peak<<"/"<<nativeui::Pool<<"; diff-only updates; text stays data; scaling validation; hide; bounded pool; native DDS assets\n";
 return 0;
}

