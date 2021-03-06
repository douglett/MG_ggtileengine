#pragma once
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <SDL/SDL.h>

// main
extern SDL_Surface *buf, *tileset, *guy, *guyshadow, *idsquare, *qbfont;
int  mainloop();
int  init();
void walk2(int dir);
void walk3(int dir);
int  action2(int dir);
void recenter();
void paint1();
void flip3x();

namespace etc {  // helpers
	struct SrcImg { SDL_Rect r; SDL_Surface* sf; };
	SDL_Surface* loadbmp(const std::string& fname);
	SDL_Surface* mksurface(int w, int h);
	SDL_Surface* clonesurface(SDL_Surface* sf);
	void scalex(SDL_Surface* sf, int sx);
	void qbprint(SDL_Surface* sf, int x, int y, const std::string& s);
	std::string join(const std::vector<std::string>& vs, const std::string& glue);
}
namespace gmap {
	struct Sprite { std::string id; SDL_Rect pos; etc::SrcImg img; };
	extern int width, height, layers;
	extern SDL_Rect viewport;
	extern std::vector<std::vector<int>> tilemap;
	extern std::list<gmap::Sprite> spritelist;
	int loadmap(const std::string& fname);
	int loadascii(const std::string& fname);
	int loadascii(const std::vector<std::string>& ascmap);
	int bounds(int l, int tx, int ty);
	int collide(int tx, int ty);
	etc::SrcImg gettile(int l, int tx, int ty);
	Sprite* getsprite(const std::string& id);
	void delsprite(const Sprite* spr);
	void paint();
}
namespace menus {
	extern int textspeed;
	extern std::vector<std::string> items;
	int init();
	void txtbox(SDL_Rect dst, const std::string& txt);
	void dialogue(const std::string& str);
	std::string showlist(SDL_Rect mbox, const std::vector<std::string>& mitems);
	void showinv();
}
namespace battle {
	struct BattleStats {
		std::string name;
		int str, intl, stm, elem;
		int hp, mp;
	};
	extern BattleStats player, enemy;
	extern int plxp;
	void rest(BattleStats& st);
	void statsbox(int x, int y);
	void begin();
	void exchange(int pdmg, int pelem);
}

namespace mygame {
	int init();
	int action(const std::string& mapname, gmap::Sprite* spr);
}
