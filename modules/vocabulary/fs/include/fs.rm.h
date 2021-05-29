#ifdef FSRM_EXPORTS
#define FSRM_API __declspec(dllexport)
#else
#define FSRM_API __declspec(dllimport)
#endif

