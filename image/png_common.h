
#ifndef _TA_IMAGE_PNG_COMMON_H_
#define _TA_IMAGE_PNG_COMMON_H_ 1
#pragma once

#include <base/basictypes.h>
#include <windows.h>
#include <tchar.h>
#include "image.h"

namespace image {

	ImageInfo* FromPngFile(LPCTSTR lpszPngFile);

}

#endif