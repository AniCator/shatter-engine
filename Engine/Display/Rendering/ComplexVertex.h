// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math.h>

struct ComplexVertex
{
	ComplexVertex() = default;
	ComplexVertex( const Vector3D& InPosition )
	{
		Position = InPosition;
	}

	ComplexVertex( const Vector3D& InPosition, const Vector3D& InNormal )
	{
		Position = InPosition;
		Normal = InNormal;
	}

	ComplexVertex( const Vector3D& InPosition, const Vector3D& InNormal, const Vector2D& InTextureCoordinate )
	{
		Position = InPosition;
		Normal = InNormal;
		TextureCoordinate = InTextureCoordinate;
	}

	Vector3D Position = Vector3D::Zero;
	Vector2D TextureCoordinate = Vector2D( 0.0f, 0.0f );
	Vector3D Normal = Vector3D( 0.0f, 0.0f, 1.0f );
	Vector3D Tangent = Vector3D( 0.0f, 1.0f, 0.0f );
	Vector3D Color = Vector3D::One;

	Vector4D Bone = Vector4D( -1.0f, -1.0f, -1.0f, -1.0f );
	Vector4D Weight = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f );

	bool operator<( const ComplexVertex& B ) const
	{
		return memcmp( (void*)this, (void*)&B, sizeof( ComplexVertex ) ) > 0;
	};
};
