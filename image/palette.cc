
#include "palette.h"

namespace palette {

int find_color(tacolor color, tacolor *palette, uint32 palette_color_count)
{
	for (uint32 i = 0; i < palette_color_count; i++) {
		if (palette[i] == color) {
			return i;
		}
	}
	return -1;
}


}