#pragma once

// Static Library
#ifdef _DEBUG
#pragma comment(lib, "Engine\\Debug\\Engine.lib")
#else
#pragma comment(lib, "Engine\\Release\\Engine.lib")
#endif

#include "Headers\EnginePch.h"

