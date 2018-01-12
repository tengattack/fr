
#ifndef _SNOW_CUTE_CORE_MS_USER_H_
#define _SNOW_CUTE_CORE_MS_USER_H_ 1
#pragma once

#include "../base/common.h"
#include <common/Buffer.h>

#include "users.h"

bool ms_check_flags(const uint8 *data, int len);
bool ms_decompress(const uint8 *data, int len, CBuffer& buf);
bool ms_decode(uint8 *raw_data, int raw_len);
int ms_text_parse(CBuffer& text_buf, core::TAMJSTRUCT** userlist);

#endif