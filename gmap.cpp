#include "globals.h"
#include <fstream>
#include <sstream>
#include <cassert>
using namespace std;


namespace gmap {
	
	int width = 0, height = 0, layers = 0;
	SDL_Rect viewport = { 0, 0, 10*16, 9*16 };
	std::vector<std::vector<int>> tilemap;
	std::list<gmap::Sprite> spritelist;
	
	int loadmap(const std::string& fname) {
		fstream fs(fname, fstream::in);
		assert(fs.is_open() == true);
		string input, s;
		while (getline(fs, s))
			input += s;
		fs.close();
		
		// get width and height
		width = height = 0;
		{
			int pos1 = input.find("width=\"") + 7;
			int pos2 = input.find("\"", pos1);
			s = input.substr(pos1, pos2-pos1);
			stringstream ss(s);
			ss >> width;
			//printf("[%d]\n", width);
		}
		{
			int pos1 = input.find("height=\"") + 8;
			int pos2 = input.find("\"", pos1);
			s = input.substr(pos1, pos2-pos1);
			stringstream ss(s);
			ss >> height;
			//printf("[%d]\n", height);
		}
		
		// get row data 1-3
		int lpos = 0;
		vector<string> data;
		while (true) {
			string start = "<data encoding=\"csv\">",  end = "</data>";
			int pos1 = input.find(start, lpos) ;
			if (pos1 == string::npos)  break;
			pos1 += start.length();
			int pos2 = input.find(end, pos1);
			lpos = pos2;
			s = input.substr(pos1, pos2-pos1);
			data.push_back(s);
			//printf("[%s]\n", s.c_str());
		}
		
		// parse rows to number list
		tilemap = {};
		for (auto& d : data) {
			for (char& c : d)
				if (c == ',')  c = ' ';
			tilemap.push_back({});
			stringstream ss(d);
			int n = 0;
			while (ss >> n)
				tilemap.back().push_back(n);
		}
		layers = tilemap.size();
		
		printf("map load OK:  %d,%d,%d\n", width, height, layers);
		return 0;
	}
		
	int loadascii(const std::string& fname) {
		// load
		vector<string> ascmap = {
			"############",
			"#..#.......#",
			"#..#.......#",
			"#..........#",
			"#####..#####",
			"#/........|#",
			"#..........#",
			"#..........#",
			"#####DD#####",
			"T,,,,,,,,,,T",
			"T,,,,,,,,,,T",
		};
		// map size
		height = ascmap.size();
		width = ascmap[0].length();
		// parse to int list
		tilemap = { {}, {} };
		int mpos = 0;
		for (const auto& s : ascmap)
		for (char c : s) {
			int t = 0, k = 0;
			switch (c) {
			case ' ':  t =  0;  break;
			case ',':  t =  1;  break;
			case '.':  t =  3;  break;
			case '#':  t =  8;  k = 1;  break;
			case 'T':  t =  9;  k = 1;  break;
			case 'D':  t = 10;  k = 1;  npcs::npclist.push_back({ "door1", "nilcoffee", mpos%width, mpos/width });  break;
			case '/':  t = 11;  k = -1;  break;
			case '|':  t = 12;  k = -1;  break;
			}
			// k = (t >= 4);
			tilemap[0].push_back(t+1);
			tilemap[1].push_back(k);
			mpos++;
		}
		layers = tilemap.size();
		return 0;
	}
	
	int bounds(int l, int x, int y) {
		return (l < 0 || x < 0 || y < 0 || l >= layers || x >= width || y >= height);
	}
	
	int collide(int x, int y) {
		if (bounds(layers-1, x, y))  return 1;  // bounds check
		if (tilemap[layers-1][y * width + x] > 0)  return 1;  // top layer collision
		for (const auto& spr : spritelist)
			if (spr.img.sf != NULL && spr.pos.x/16 == x && spr.pos.y/16 == y)  return 1;  // sprite collision
		return 0;
	}
	
	SrcImg gettile(int l, int x, int y) {
		if (bounds(l, x, y))  return { {0}, NULL };
		const int tswidth = tileset->w / 16;
		int t = tilemap[l][y * width + x];
		if (t == 0)  return { {0}, NULL };
		t--;
		SDL_Rect r = { int16_t(t % tswidth * 16), int16_t(t / tswidth * 16), 16, 16 };
		return { r, tileset };
	}
	
	Sprite& getsprite(const std::string& id) {
		for (auto& sp : spritelist)
			if (sp.id == id)  return sp;
		fprintf(stderr, "missing sprite: %s\n", id.c_str()), exit(1);
	}
	
	void paint(SDL_Rect viewport, int posx, int posy) {
		for (int y = -1; y <= viewport.h; y++)
		for (int x = -1; x <= viewport.w; x++)
		for (int l = 0; l < layers-1; l++) {
			auto srcimg = gettile(l, viewport.x + x, viewport.y + y);
			if (srcimg.sf == NULL)  continue;
			SDL_Rect dst = { int16_t(x*16 + posx), int16_t(y*16 + posy), 0, 0 };
			SDL_BlitSurface(srcimg.sf, &srcimg.r, buf, &dst);
		}
	}
	
	void paint() {
		// calculate tile positions
		int twidth = viewport.w / 16;
		int theight = viewport.h / 16;
		int tx = viewport.x / 16;
		int px = viewport.x % 16;
		int ty = viewport.y / 16;
		int py = viewport.y % 16;
		if (viewport.x < 0)  tx--, px += 16;  // special case, negative offset
		if (viewport.y < 0)  ty--, py += 16;
		// show blocks
		// SDL_SetClipRect(buf, tt);
		for (int y = 0; y <= theight; y++)
		for (int x = 0; x <= twidth; x++)
		for (int l = 0; l < layers-1; l++) {
			auto srcimg = gettile(l, tx + x, ty + y);
			if (srcimg.sf == NULL)  continue;
			SDL_Rect dst = { int16_t(x*16 - px), int16_t(y*16 - py), 0, 0 };
			SDL_BlitSurface(srcimg.sf, &srcimg.r, buf, &dst);
		}
		// show sprites
		for (const auto& spr : spritelist) {
			if (spr.pos.x > viewport.x + viewport.w || spr.pos.x + spr.pos.w < viewport.x)  continue;
			if (spr.pos.y > viewport.y + viewport.h || spr.pos.y + spr.pos.h < viewport.y)  continue;
			SDL_Rect dst = spr.pos, src = spr.img.r;
			dst.y -= viewport.y + 6, dst.x -= viewport.x;
			//SDL_FillRect(buf, &dst, 0xff0000ff);
			SDL_BlitSurface(spr.img.sf, &src, buf, &dst);
		}
	}

} // end map
