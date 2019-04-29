// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <math.h>

#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/matrix_transform.hpp>
#include <ThirdParty/glm/gtx/quaternion.hpp>

const static glm::mat4 IdentityMatrix = glm::mat4( 1.0f );
static const glm::vec3 WorldRight = glm::vec3( 1.0f, 0.0f, 0.0f );
static const glm::vec3 WorldForward = glm::vec3( 0.0f, 1.0f, 0.0f );
static const glm::vec3 WorldUp = glm::vec3( 0.0f, 0.0f, 1.0f );

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
		StoredPosition = glm::vec3( 0.0f );
		StoredOrientation = glm::vec3( 0.0f, 0.0f, 0.0f );
		StoredSize = glm::vec3( 1.0f );
	}

	FTransform( const glm::vec3& Position, const glm::vec3& Orientation, const glm::vec3& Size )
	{
		SetTransform( Position, Orientation, Size );
	}

	void SetPosition( const glm::vec3& Position )
	{
		this->StoredPosition = Position;
		Update();
	}

	void SetOrientation( const glm::vec3& Orientation )
	{
		this->StoredOrientation = Orientation;
		Update();
	}

	void SetSize( const glm::vec3& Size )
	{
		this->StoredSize = Size;
		Update();
	}

	void SetTransform( const glm::vec3& Position, const glm::vec3& Orientation, const glm::vec3& Size )
	{
		this->StoredPosition = Position;
		this->StoredOrientation = Orientation;
		this->StoredSize = Size;
		Update();
	}

	void SetTransform( const glm::vec3& Position, const glm::vec3& Orientation )
	{
		this->StoredPosition = Position;
		this->StoredOrientation = Orientation;
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

	const glm::vec3& GetPosition() const
	{
		return StoredPosition;
	}

	const glm::vec3& GetOrientation() const
	{
		return StoredOrientation;
	}

	const glm::vec3& GetSize() const
	{
		return StoredSize;
	}

	glm::vec3 Position( const glm::vec3& Position ) const
	{
		return TransformMatrix * glm::vec4( Position, 1.0f );
	}

private:
	void Update()
	{
		static const glm::vec3 AxisX = glm::vec3( 1.0f, 0.0f, 0.0f );
		static const glm::vec3 AxisY = glm::vec3( 0.0f, 1.0f, 0.0f );
		static const glm::vec3 AxisZ = glm::vec3( 0.0f, 0.0f, 1.0f );

		glm::mat4 ScaleMatrix = glm::scale( IdentityMatrix, StoredSize );
		
		glm::quat Quaternion = glm::quat( glm::radians( StoredOrientation ) );
		RotationMatrix = glm::toMat4( Quaternion );

		TransformMatrix = glm::translate( IdentityMatrix, StoredPosition );

		TransformMatrix = TransformMatrix * RotationMatrix * ScaleMatrix;
	}

	glm::mat4 RotationMatrix;
	glm::mat4 TransformMatrix;

	glm::vec3 StoredPosition;
	glm::vec3 StoredOrientation;
	glm::vec3 StoredSize;
};

namespace Math
{
	static const float Pi = glm::pi<float>();
	static const float Pi2 = Pi * 2.0f;

	inline glm::vec3 EulerToDirection( const glm::vec3& Euler )
	{
		glm::vec3 Direction;
		Direction[1] = cos( glm::radians( Euler[0] ) ) * cos( glm::radians( Euler[1] ) );
		Direction[2] = sin( glm::radians( Euler[0] ) );
		Direction[0] = cos( glm::radians( Euler[0] ) ) * sin( glm::radians( Euler[1] ) );
		Direction = glm::normalize( Direction );

		return Direction;
	}

	inline float Length( const glm::vec2& Vector )
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

	inline float Length( const glm::vec3& Vector )
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

	inline void Seed( uint32_t Value )
	{
		std::srand( Value );
	}

	inline double Random()
	{
		return static_cast<double>( std::rand() ) / RAND_MAX;
	}
}
