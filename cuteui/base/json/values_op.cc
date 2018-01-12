
#include "values.h"
#include "values_op.h"

void ListValueAppend(ListValue* value, int count, ...) {
	va_list ap;
	va_start(ap, count);

	Value *v = NULL;
	for (int i = 0; i < count; i++) {
		v = va_arg(ap, Value *);
		if (v == NULL) {
			value->Append(Value::CreateNullValue());
		} else {
			value->Append(v);
		}
	}

	va_end(ap);
}

bool ListValueGet(ListValue* value, Value** outvalue, bool lastremove, int depth, ...) {
	va_list ap;
	va_start(ap, depth);

	int index = 0;
	ListValue* ov = value;
	bool bget = true;
	for (int i = 0; i < depth; i++) {
		index = va_arg(ap, int);
		if (bget) {
			if (depth == i + 1) {
				if (lastremove) {
					bget = ov->Remove(index, (Value **)&ov);
				} else {
					bget = ov->Get(index, (Value **)&ov);
				}
			} else {
				bget = ov->GetList(index, &ov);
			}
			if (!ov) bget = false;	// ov may NULL
		}
	}

	va_end(ap);

	if (bget) {
		if (ov->GetType() == Value::TYPE_NULL) {
			//no null
			bget = false;
		} else if (outvalue) {
			*outvalue = ov;
		} else if (lastremove) {
			delete ov;
		}
	}
	return bget;
}
