// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#define CONCAT_(x,y) x##y
#define Concatenate(x,y) CONCAT_(x,y)
#define MacroName(x) Concatenate(x, __LINE__)