// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <glm/glm.hpp>

namespace Math
{
	inline float Length( glm::vec2& Vector )
	{
		return sqrtf( Vector[0] * Vector[0] + Vector[1] * Vector[1] );
	}

	inline float Normalize( glm::vec2& Vector )
	{
		const float LengthBiased = 1.f / ( Length( Vector ) + FLT_EPSILON );

		Vector[0] *= LengthBiased;
		Vector[1] *= LengthBiased;

		return LengthBiased;
	}

	inline float Normalize( glm::vec2& Vector, float Length )
	{
		const float LengthBiased = 1.f / ( Length + FLT_EPSILON );

		Vector[0] *= LengthBiased;
		Vector[1] *= LengthBiased;

		return LengthBiased;
	}

	inline float Length( glm::vec3& Vector )
	{
		return sqrtf( Vector[0] * Vector[0] + Vector[1] * Vector[1] + Vector[2] * Vector[2] );
	}

	inline float Normalize( glm::vec3& Vector )
	{
		const float LengthBiased = 1.f / ( Length( Vector ) + FLT_EPSILON );

		Vector[0] *= LengthBiased;
		Vector[1] *= LengthBiased;
		Vector[2] *= LengthBiased;

		return LengthBiased;
	}

	inline float Normalize( glm::vec3& Vector, float Length )
	{
		const float LengthBiased = 1.f / ( Length + FLT_EPSILON );

		Vector[0] *= LengthBiased;
		Vector[1] *= LengthBiased;
		Vector[2] *= LengthBiased;

		return LengthBiased;
	}

	struct FFrustumPlane
	{

	};

	struct FFrustum
	{

	};
}
