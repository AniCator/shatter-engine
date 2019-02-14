// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/matrix_transform.hpp>
#include <ThirdParty/glm/gtx/quaternion.hpp>

const static glm::mat4 IdentityMatrix = glm::mat4( 1.0f );

struct FFrustumPlane
{

};

struct FFrustum
{
	FFrustumPlane Planes[6];
};

struct FBounds
{
	glm::vec3 Minimum;
	glm::vec3 Maximum;
};

struct FTransform
{
public:
	FTransform()
	{
		TransformMatrix = IdentityMatrix;
		Position = glm::vec3( 0.0f );
		Orientation = glm::vec3( 0.0f, 0.0f, 1.0f );
		Size = glm::vec3( 1.0f );
	}

	FTransform( glm::vec3& Position, glm::vec3& Orientation, glm::vec3& Size )
	{
		SetTransform( Position, Orientation, Size );
	}

	void SetPosition( glm::vec3& Position )
	{
		this->Position = Position;
		Update();
	}

	void SetOrientation( glm::vec3& Orientation )
	{
		this->Orientation = Orientation;
		Update();
	}

	void SetSize( glm::vec3& Size )
	{
		this->Size = Size;
		Update();
	}

	void SetTransform( glm::vec3& Position, glm::vec3& Orientation, glm::vec3& Size )
	{
		this->Position = Position;
		this->Orientation = Orientation;
		this->Size = Size;
		Update();
	}

	void SetTransform( glm::vec3& Position, glm::vec3& Orientation )
	{
		this->Position = Position;
		this->Orientation = Orientation;
		Update();
	}

	glm::mat4& GetRotationMatrix()
	{
		return RotationMatrix;
	}

	glm::mat4& GetTransformMatrix()
	{
		return TransformMatrix;
	}

private:
	void Update()
	{
		TransformMatrix = IdentityMatrix;

		TransformMatrix = glm::translate( TransformMatrix, Position );

		static const glm::vec3 AxisX = glm::vec3( 1.0f, 0.0f, 0.0f );
		static const glm::vec3 AxisY = glm::vec3( 0.0f, 1.0f, 0.0f );
		static const glm::vec3 AxisZ = glm::vec3( 0.0f, 0.0f, 1.0f );

		const glm::quat ModelQuaternion = glm::quat( Orientation );
		RotationMatrix = glm::toMat4( ModelQuaternion );

		TransformMatrix *= RotationMatrix;

		TransformMatrix = glm::scale( TransformMatrix, Size );
	}

	glm::mat4 RotationMatrix;
	glm::mat4 TransformMatrix;

	glm::vec3 Position;
	glm::vec3 Orientation;
	glm::vec3 Size;
};

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

	inline float Lerp( float A, float B, float Alpha )
	{
		return A + Alpha * ( B - A );
	}

	inline bool PointInBoundingBox( const glm::vec3& Vector, const glm::vec3& Minimum, const glm::vec3& Maximum )
	{
		if( Vector[0] > Minimum[0] && Vector[1] > Minimum[1] && Vector[2] > Minimum[2] &&
			Vector[0] < Maximum[0] && Vector[1] < Maximum[1] && Vector[2] < Maximum[2] )
		{
			return true;
		}

		return false;
	}

	inline bool BoundingBoxIntersection( const glm::vec3& MinimumA, const glm::vec3& MaximumA, const glm::vec3& MinimumB, const glm::vec3& MaximumB )
	{
		if( ( MinimumA[0] < MaximumB[0] && MaximumA[0] > MinimumB[0] ) &&
			( MinimumA[1] < MaximumB[1] && MaximumA[1] > MinimumB[1] ) &&
			( MinimumA[2] < MaximumB[2] && MaximumA[2] > MinimumB[2] ) )
		{
			return true;
		}

		return false;
	}
}
