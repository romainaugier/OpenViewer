#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

#include "cstdint"
#include "math.h"
#include <vector>

struct ImPlaybar
{
	ImVec2 playbar_range;
	uint16_t playbar_frame = 0;
	int playbar_framerate = 24;
	bool play;

	ImPlaybar(ImVec2 range) : 
		playbar_range(range)
	{
		play = false;
	}

	void draw(std::vector<char>& cached);
};

inline bool hover(ImVec2 min, ImVec2 max, ImVec2 pos)
{
	if (pos.x > min.x && pos.y > min.y && pos.x < max.x && pos.y < max.y) return true;
	else return false;
}