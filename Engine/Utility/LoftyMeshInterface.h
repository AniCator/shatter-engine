// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/File.h>

struct LoftyMeshInterface
{
	static bool Import( const CFile& File, struct FPrimitive* Output, struct AnimationSet& Set );
};
