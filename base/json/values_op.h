
#ifndef BASE_VALUES_OP_H_
#define BASE_VALUES_OP_H_
#pragma once

void ListValueAppend(ListValue* value, int count, ...);
bool ListValueGet(ListValue* value, Value** outvalue, bool lastremove, int depth, ...);

#endif