// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#define UseCompactVertex
#ifdef UseCompactVertex
#include "CompactVertex.h"
using VertexFormat = CompactVertex;
#else
#include "ComplexVertex.h"
using VertexFormat = ComplexVertex;
#endif