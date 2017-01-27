#ifndef NAPA_MODULE_EXPORT_H
#define NAPA_MODULE_EXPORT_H

#ifndef NAPA_MODULE_EXTENSION
#define NAPA_MODULE_EXPORT __declspec(dllexport) 
#else
#define NAPA_MODULE_EXPORT __declspec(dllimport) 
#endif

#endif NAPA_MODULE_EXPORT_H