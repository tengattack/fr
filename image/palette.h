
#ifndef _TA_IMAGE_PALETTE_H_
#define _TA_IMAGE_PALETTE_H_ 1
#pragma once

#include <base/basictypes.h>

#define tacolor uint32

namespace palette {

	int find_color(tacolor color, tacolor *palette, uint32 palette_color_count);

}

#endif