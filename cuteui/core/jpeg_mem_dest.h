//
// Copyright 2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

extern "C" {

#include "jpeglib.h"
#include "jerror.h"

EXTERN(void) jpeg_mem_dest(j_compress_ptr cinfo, JOCTET* buf, size_t bufsize);

}