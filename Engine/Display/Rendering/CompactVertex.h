// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math.h>
#include <ThirdParty/glad/include/glad/glad.h>

struct CompactVertex
{
	Vector3D Position = Vector3D::Zero;
	Vector2D TextureCoordinate = Vector2D( 0.0f, 0.0f );
	GLbyte Normal[3] = { 0, 0, 127 };
	GLbyte Tangent[3] = { 0, 127, 0 };
	Vector3D Color = Vector3D::One;

	Vector4D Bone = Vector4D( -1.0f, -1.0f, -1.0f, -1.0f );
	Vector4D Weight = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f );
};
