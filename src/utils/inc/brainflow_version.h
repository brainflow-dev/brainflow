#pragma once

#ifndef BRAINFLOW_VERSION
#define BRAINFLOW_VERSION 0.0.1
#endif

// https://stackoverflow.com/a/240361

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2 (s)
#define BRAINFLOW_VERSION_STRING STRINGIZE(BRAINFLOW_VERSION)
