#pragma once

#ifndef NAPA_MODULE_EXTENSION
#define NAPA_MODULE_EXPORT __declspec(dllexport) 
#else
#define NAPA_MODULE_EXPORT __declspec(dllimport) 
#endif