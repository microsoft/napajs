#ifndef NAPA_EXPORTS_H
#define NAPA_EXPORTS_H

#ifdef NAPA_EXPORTS
#define NAPA_API __declspec(dllexport)
#else
#define NAPA_API __declspec(dllimport)
#endif // NAPA_EXPORTS

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif // __cplusplus

#endif // NAPA_EXPORTS_H
