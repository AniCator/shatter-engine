// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <math.h>
#include <algorithm>

#include <Engine/Utility/Math/Vector.h>
#include <Engine/Utility/Math/Matrix.h>
#include <Engine/Utility/Math/Transform.h>

#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/matrix_transform.hpp>
#include <ThirdParty/glm/gtx/quaternion.hpp>

#define Vector3DToInitializerList(Vector) { Vector[0],Vector[1],Vector[2] }
#define Vector3DToGLM(Vector) glm::vec3( Vector[0],Vector[1],Vector[2] )

#define Vector2DToInitializerList(Vector) { Vector[0],Vector[1] }
#define Vector2DToGLM(Vector) glm::vec2( Vector[0],Vector[1] )

static const Vector3D WorldRight = { 1.0f, 0.0f, 0.0f };
static const Vector3D WorldForward = { 0.0f, 1.0f, 0.0f };
static const Vector3D WorldUp = { 0.0f, 0.0f, 1.0f };

struct FBounds
{
	FBounds()
	{
		Minimum = Vector3D( 0.0f, 0.0f, 0.0f );
		Maximum = Vector3D( 0.0f, 0.0f, 0.0f );
	}

	FBounds( const Vector3D& Minimum, const Vector3D& Maximum )
	{
		this->Minimum = Minimum;
		this->Maximum = Maximum;
	}

	Vector3D Minimum;
	Vector3D Maximum;
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

	inline Vector3D EulerToDirection( const Vector3D& Euler )
	{
		Vector3D Direction;
		Direction[1] = cos( glm::radians( Euler[0] ) ) * cos( glm::radians( Euler[2] ) );
		Direction[2] = sin( glm::radians( Euler[0] ) );
		Direction[0] = cos( glm::radians( Euler[0] ) ) * sin( glm::radians( Euler[2] ) );

		return Direction.Normalized();
	}

	inline Vector3D DirectionToEuler( const Vector3D& Direction )
	{
		const float Pitch = atan2f( Direction.Z, sqrtf( Direction.X * Direction.X + Direction.Y * Direction.Y ) ) * ( 180.f / Math::Pi );
		const float Yaw = atan2f( Direction.Y, Direction.X ) * ( 180.f / Math::Pi );
		const float Roll = 0.0f;

		return Vector3D( Pitch, Roll, Yaw );
	}

	inline float LengthSquared( const glm::vec2& Vector )
	{
		return Vector[0] * Vector[0] + Vector[1] * Vector[1];
	}

	inline float Length( const glm::vec2& Vector )
	{
		return sqrtf( LengthSquared( Vector ) );
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

	inline float LengthSquared( const glm::vec3& Vector )
	{
		return Vector[0] * Vector[0] + Vector[1] * Vector[1] + Vector[2] * Vector[2];
	}

	inline float Length( const glm::vec3& Vector )
	{
		return sqrtf( LengthSquared( Vector ) );
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

	template<typename T>
	inline T Max(const T& A, const T& B)
	{
		return std::max( A, B );
	}

	template<typename T>
	inline T Min( const T& A, const T& B )
	{
		return std::min( A, B );
	}

	template<typename T>
	inline T Clamp( const T& X, const T& Minimum, const T& Maximum )
	{
		return Min( Max( Minimum, X ), Maximum );
	}

	inline float Lerp( float A, float B, const float Alpha )
	{
		return A + Alpha * ( B - A );
	}

	inline Vector3D Lerp( const Vector3D& A, const Vector3D& B, const float Alpha )
	{
		return A + Vector3D( Alpha, Alpha, Alpha ) * ( B - A );
	}

	inline float Abs( const float& Value )
	{
		return fabs( Value );
	}

	inline Vector3D Abs( const Vector3D& Value )
	{
		return { fabs( Value.X ), fabs( Value.Y ), fabs( Value.Z ) };
	}

	inline bool Equal( const float& A, const float& B, const float Tolerance = 0.001f )
	{
		float Difference = A - B;
		Difference = fabs( Difference );
		return Difference < Tolerance;
	}

	inline bool Equal( const Vector3D& A, const Vector3D& B, const float Tolerance = 0.001f )
	{
		Vector3D Difference = A - B;
		Difference.X = fabs( Difference.X );
		Difference.Y = fabs( Difference.Y );
		Difference.Z = fabs( Difference.Z );
		return Difference.X < Tolerance &&
			Difference.Y < Tolerance &&
			Difference.Z < Tolerance;
	}

	inline float VectorMax( const float& A, const float& B, const float& C )
	{
		return Max( Max( A, C ), B );
	}

	inline float VectorMin( const float& A, const float& B, const float& C )
	{
		return Min( Min( A, C ), B );
	}

	inline FBounds AABB(const Vector3D* Positions, uint32_t Count)
	{
		FBounds AABB;
		for( uint32_t VertexIndex = 0; VertexIndex < Count; VertexIndex++ )
		{
			const Vector3D& Position = Positions[VertexIndex];

			if( VertexIndex == 0 )
			{
				AABB.Minimum = Position;
				AABB.Maximum = Position;
			}

			if( Position[0] < AABB.Minimum[0] )
			{
				AABB.Minimum[0] = Position[0];
			}

			if( Position[1] < AABB.Minimum[1] )
			{
				AABB.Minimum[1] = Position[1];
			}

			if( Position[2] < AABB.Minimum[2] )
			{
				AABB.Minimum[2] = Position[2];
			}

			if( Position[0] > AABB.Maximum[0] )
			{
				AABB.Maximum[0] = Position[0];
			}

			if( Position[1] > AABB.Maximum[1] )
			{
				AABB.Maximum[1] = Position[1];
			}

			if( Position[2] > AABB.Maximum[2] )
			{
				AABB.Maximum[2] = Position[2];
			}
		}

		return AABB;
	}

	inline FBounds AABB( const FBounds& AABB, FTransform& Transform )
	{
		const auto& Minimum = AABB.Minimum;
		const auto& Maximum = AABB.Maximum;

		Vector3D Positions[8];
		Positions[0] = Vector3D( Minimum.X, Maximum.Y, Minimum.Z );
		Positions[1] = Vector3D( Maximum.X, Maximum.Y, Minimum.Z );
		Positions[2] = Vector3D( Maximum.X, Minimum.Y, Minimum.Z );
		Positions[3] = Vector3D( Minimum.X, Minimum.Y, Minimum.Z );

		Positions[4] = Vector3D( Minimum.X, Maximum.Y, Maximum.Z );
		Positions[5] = Vector3D( Maximum.X, Maximum.Y, Maximum.Z );
		Positions[6] = Vector3D( Maximum.X, Minimum.Y, Maximum.Z );
		Positions[7] = Vector3D( Minimum.X, Minimum.Y, Maximum.Z );

		Vector3D TransformedPositions[8];
		for( size_t PositionIndex = 0; PositionIndex < 8; PositionIndex++ )
		{
			TransformedPositions[PositionIndex] = Transform.Transform( Positions[PositionIndex] );
		}

		FBounds TransformedAABB = Math::AABB( TransformedPositions, 8 );
		return TransformedAABB;
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

	inline bool PointInBoundingBox( const Vector3D& Vector, const Vector3D& Minimum, const Vector3D& Maximum )
	{
		if( Vector.X > Minimum.X && Vector.Y > Minimum.Y && Vector.Z > Minimum.Z &&
			Vector.X < Maximum.X && Vector.Y < Maximum.Y && Vector.Z < Maximum.Z )
		{
			return true;
		}

		return false;
	}

	inline Vector3D ProjectOnVector( const Vector3D& U, const Vector3D& V )
	{
		return V * ( U.Dot( V ) / V.LengthSquared() );
	}

	inline Vector3D PointOnLine( const Vector3D& Point, const Vector3D& Start, const Vector3D End )
	{
		return Start + ProjectOnVector( Point - Start, End - Start );
	}

	// TODO: Not finished.
	inline bool LineInBoundingBox( const Vector3D& Start, const Vector3D& End, const Vector3D& Minimum, const Vector3D& Maximum )
	{
		return false;
	}

	inline bool BoundingBoxIntersection( const Vector3D& MinimumA, const Vector3D& MaximumA, const Vector3D& MinimumB, const Vector3D& MaximumB )
	{
		if( ( MinimumA.X < MaximumB.X && MaximumA.X > MinimumB.X ) &&
			( MinimumA.Y < MaximumB.Y && MaximumA.Y > MinimumB.Y ) &&
			( MinimumA.Z < MaximumB.Z && MaximumA.Z > MinimumB.Z ) )
		{
			return true;
		}

		return false;
	}

	inline bool PlaneIntersection( Vector3D& Intersection, const Vector3D& RayOrigin, const Vector3D& RayTarget, const Vector3D& PlaneOrigin, const Vector3D& PlaneNormal )
	{
		const Vector3D RayVector = RayTarget - RayOrigin;
		const Vector3D PlaneVector = PlaneOrigin - RayOrigin;

		const float DistanceRatio = PlaneVector.Dot( PlaneNormal ) / RayVector.Dot( PlaneNormal );

		Intersection = RayOrigin + RayVector * DistanceRatio;

		return DistanceRatio >= 0.0f;
	}

	inline void Seed( uint32_t Value )
	{
		std::srand( Value );
	}

	inline float Random()
	{
		return static_cast<float>( std::rand() ) / RAND_MAX;
	}

	inline float RandomRange( const float Minimum, const float Maximum )
	{
		return ( Maximum - Minimum ) * static_cast<float>( std::rand() ) / static_cast<float>( RAND_MAX ) + Minimum;
	}

	inline int32_t RandomRangeInteger( const int32_t Minimum, const int32_t Maximum )
	{
		return Minimum + ( std::rand() % ( Maximum - Minimum + 1 ) );
	}

	inline Vector4D FromGLM( const glm::vec4& Vector )
	{
		return Vector4D( Vector[0], Vector[1], Vector[2], Vector[3] );
	}

	inline glm::vec4 ToGLM( const Vector4D& Vector )
	{
		return glm::vec4( Vector.X, Vector.Y, Vector.Z, Vector.W );
	}

	inline Vector3D FromGLM( const glm::vec3& Vector )
	{
		return Vector3D( Vector[0], Vector[1], Vector[2] );
	}

	inline glm::vec3 ToGLM( const Vector3D& Vector )
	{
		return glm::vec3( Vector.X, Vector.Y, Vector.Z );
	}

	inline Vector2D FromGLM( const glm::vec2& Vector )
	{
		return Vector2D( Vector[0], Vector[1] );
	}

	inline glm::vec2 ToGLM( const Vector2D& Vector )
	{
		return glm::vec2( Vector.X, Vector.Y );
	}

	inline Matrix4D FromGLM( const glm::mat4& Matrix )
	{
		Matrix4D Result;

		Result[0] = { Matrix[0][0], Matrix[0][1], Matrix[0][2], Matrix[0][3] };
		Result[1] = { Matrix[1][0], Matrix[1][1], Matrix[1][2], Matrix[1][3] };
		Result[2] = { Matrix[2][0], Matrix[2][1], Matrix[2][2], Matrix[2][3] };
		Result[3] = { Matrix[3][0], Matrix[3][1], Matrix[3][2], Matrix[3][3] };

		return Result;
	}

	inline glm::mat4 ToGLM( const Matrix4D& Matrix )
	{
		glm::mat4 Result;

		Result[0] = { Matrix[0][0], Matrix[0][1], Matrix[0][2], Matrix[0][3] };
		Result[1] = { Matrix[1][0], Matrix[1][1], Matrix[1][2], Matrix[1][3] };
		Result[2] = { Matrix[2][0], Matrix[2][1], Matrix[2][2], Matrix[2][3] };
		Result[3] = { Matrix[3][0], Matrix[3][1], Matrix[3][2], Matrix[3][3] };

		return Result;
	}
}

struct FFrustumPlane
{

};

struct FFrustum
{
	FFrustumPlane Planes[6];
};
