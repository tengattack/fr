
#ifndef _TA_IMAGE_COLGA_H_
#define _TA_IMAGE_COLGA_H_ 1
#pragma once

#include <base/basictypes.h>
#include <windows.h>
#include <tchar.h>
#include "image.h"

namespace image {

	bool colga_set_color(LPCSTR lpszFile, uint32 new_colors = 256);

}

#endif