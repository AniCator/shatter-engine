// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math/Vector.h>

class CBody;

namespace Geometry
{
	struct Result
	{
		bool Hit = false;
		Vector3D Position = Vector3D::Zero;
		float Distance = 0.0f;

		// This value is only set when using the physics engine to cast rays. It will remain null if nothing was hit.
		CBody* Body = nullptr;
	};
}