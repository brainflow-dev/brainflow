#pragma once

// matlab uses regexp to parse headers and find function declarations
// this regexp doesnt work with __attribute__ ((visibility ("default"))), solution - build all code
// using shared_export.h but copypaste this file to matlab headers parser instead shared_export.h
#ifdef _WIN32
#define SHARED_EXPORT __declspec(dllexport)
#define CALLING_CONVENTION __cdecl
#else
#define SHARED_EXPORT
#define CALLING_CONVENTION
#endif
