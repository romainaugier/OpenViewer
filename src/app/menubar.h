#include "settings.h"
#include "core/loader.h"
#include "ImFileDialog.h"
#include "display.h"
#include "implaybar.h"

#include "stdio.h"
#include <algorithm>

struct Menubar
{
	void draw(Settings& current_settings, Loader& loader, Display& display, ImPlaybar& playbar);
};