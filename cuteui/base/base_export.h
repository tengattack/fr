
#define BASE_EXPORTS
#if defined(_WIN32) || defined(WIN32)
#ifdef  BASE_EXPORTS
#define BASE_EXPORT __declspec(dllexport)
#else
#define BASE_EXPORT __declspec(dllimport)
#endif
#else
#define BASE_EXPORT
#endif  //  BASE_EXPORTS
