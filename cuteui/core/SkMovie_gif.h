
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <images/SkMovie.h>

#include "gif_lib.h"

class SkGIFMovie : public SkMovie {
public:
    SkGIFMovie(SkStream* stream);
    virtual ~SkGIFMovie();

	inline GifFileType* GetGif() {
		return fGIF;
	}

protected:
    virtual bool onGetInfo(Info*);
    virtual bool onSetTime(SkMSec);
    virtual bool onGetBitmap(SkBitmap*);

private:
    GifFileType* fGIF;
    int fCurrIndex;
    int fLastDrawIndex;
    SkBitmap fBackup;
};
