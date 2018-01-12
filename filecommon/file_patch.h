

#include <windows.h>

#define FP_SUCCESS				0
#define FP_ERROR_OPEN_PATCH		1
#define FP_ERROR_OPEN_WRITE		2
#define FP_ERROR_CHECK			3
#define FP_ERROR_PART			4
#define FP_ERROR_ALL			5

typedef void (* FP_CALLBACK)(void *user, unsigned long finished, unsigned long count);

int file_patch(LPCWSTR patchfile, LPCWSTR writefile, FP_CALLBACK callback, void *user);
