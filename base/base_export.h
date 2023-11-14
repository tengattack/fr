#if defined(FR_EXPORT)
#if defined(WIN32)
#define BASE_EXPORT __declspec(dllexport)
#else
#define BASE_EXPORT __attribute__((visibility("default")))
#endif
#else
#define BASE_EXPORT
#endif
