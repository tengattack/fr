
#ifndef _TA_IMAGE_H_
#define _TA_IMAGE_H_ 1
#pragma once

#include <base/basictypes.h>

typedef struct _ImageSource {
    uint8 *data;
    uint32 size;
    uint32 offset;
} ImageSource;

typedef struct _ImageInfo {
    uint8 *pixelData;
    uint32 imageWidth;
    uint32 imageHeight;
} ImageInfo;

#endif