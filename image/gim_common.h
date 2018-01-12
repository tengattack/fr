
#ifndef _TA_IMAGE_GIM_COMMON_H_
#define _TA_IMAGE_GIM_COMMON_H_ 1
#pragma once

#include <base/basictypes.h>
#include <windows.h>
#include <tchar.h>
#include "image.h"
#include "gim.h"

namespace image {

	void Png2Gim(LPCSTR strPngFile, LPCSTR strGimFile, GIM_IMAGE &oriimg);

}

#endif