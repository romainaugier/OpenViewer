// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "settings.h"
#include "core/loader.h"
#include "ImFileDialog.h"
#include "display.h"
#include "implaybar.h"
#include "core/ocio.h"

#include "stdio.h"
#include <algorithm>

struct Menubar
{
	void draw(Settings& current_settings, Loader& loader, Display& display, ImPlaybar& playbar, Ocio& ocio);
};