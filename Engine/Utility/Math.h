// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <math.h>
#include <algorithm>

#include <Engine/Utility/Math/Vector.h>
#include <Engine/Utility/Math/Matrix.h>

#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/matrix_transform.hpp>
#include <ThirdParty/glm/gtx/quaternion.hpp>

#define Vector3DToInitializerList(Vector) { Vector[0],Vector[1],Vector[2] }
#define Vector3DToGLM(Vector) glm::vec3( Vector[0],Vector[1],Vector[2] )

#define Vector2DToInitializerList(Vector) { Vector[0],Vector[1] }
#define Vector2DToGLM(Vector) glm::vec2( Vector[0],Vector[1] )

const static Matrix4D IdentityMatrix = Matrix4D();
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

	inline float Lerp( float A, float B, const float Alpha )
	{
		return A + Alpha * ( B - A );
	}

	inline Vector3D Lerp( const Vector3D& A, const Vector3D& B, const float Alpha )
	{
		return A + Vector3D( Alpha, Alpha, Alpha ) * ( B - A );
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
		if( ( MinimumA.X < MaximumB.X && MaximumA.X  > MinimumB.X ) &&
			( MinimumA.Y  < MaximumB.Y && MaximumA.Y > MinimumB.Y ) &&
			( MinimumA.Z  < MaximumB.Z && MaximumA.Z  > MinimumB.Z ) )
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
		return ( Maximum - Minimum ) * static_cast<int32_t>( std::rand() ) / static_cast<int32_t>( RAND_MAX ) + Minimum;
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

struct FTransform
{
public:
	FTransform()
	{
		TransformationMatrix = IdentityMatrix;
		RotationMatrix = IdentityMatrix;
		ScaleMatrix = IdentityMatrix;
		TransformationMatrixInverse = IdentityMatrix;
		StoredPosition = Vector3D( 0.0f, 0.0f, 0.0f );
		StoredOrientation = Vector3D( 0.0f, 0.0f, 0.0f );
		StoredSize = Vector3D( 1.0f, 1.0f, 1.0f );

		Dirty();
	}

	FTransform( const Vector3D& Position, const Vector3D& Orientation, const Vector3D& Size )
	{
		SetTransform( Position, Orientation, Size );
	}

	void SetPosition( const Vector3D& Position )
	{
		this->StoredPosition = Position;
		Dirty();
	}

	void SetOrientation( const Vector3D& Orientation )
	{
		this->StoredOrientation = Orientation;
		Dirty();
	}

	void SetSize( const Vector3D& Size )
	{
		this->StoredSize = Size;
		Dirty();
	}

	void SetTransform( const Vector3D& Position, const Vector3D& Orientation, const Vector3D& Size )
	{
		this->StoredPosition = Position;
		this->StoredOrientation = Orientation;
		this->StoredSize = Size;
		Dirty();
	}

	void SetTransform( const Vector3D& Position, const Vector3D& Orientation )
	{
		this->StoredPosition = Position;
		this->StoredOrientation = Orientation;
		Dirty();
	}

	Matrix4D& GetRotationMatrix()
	{
		Update();
		return RotationMatrix;
	}

	Matrix4D& GetTransformationMatrix()
	{
		Update();
		return TransformationMatrix;
	}

	Matrix4D& GetTransformationMatrixInverse()
	{
		Update();
		return TransformationMatrixInverse;
	}

	const Vector3D& GetPosition() const
	{
		return StoredPosition;
	}

	const Vector3D& GetOrientation() const
	{
		return StoredOrientation;
	}

	const Vector3D& GetSize() const
	{
		return StoredSize;
	}

	Vector3D Position( const Vector3D& Position )
	{
		Update();
		return TransformationMatrix.Transform( Position );
	}

	Vector3D Rotate( const Vector3D& Position )
	{
		Update();
		return RotationMatrix.Transform( Position );
	}

	glm::vec3 RotateEuler( const glm::vec3& EulerRadians )
	{
		Update();
		return glm::eulerAngles( glm::quat( Math::ToGLM( RotationMatrix ) ) * glm::quat( EulerRadians ) );
	}

	Vector3D Scale( const Vector3D& Position )
	{
		Update();
		return ScaleMatrix.Transform( Position );
	}

	glm::vec3 Transform( const glm::vec3& Position )
	{
		Update();
		return Math::ToGLM( TransformationMatrix ) * glm::vec4( Position, 1.0f );
	}

	Vector3D Transform( const Vector3D& Position )
	{
		Update();
		Vector3D Vector = TransformationMatrix.Transform( Position );
		return Vector;
	}

	FTransform Transform( const FTransform& B )
	{
		Update();

		auto NewPosition = Position( B.GetPosition() );
		Vector3D Position3D = { NewPosition[0], NewPosition[1], NewPosition[2] };

		auto OrientationRadians = glm::radians( Math::ToGLM( B.GetOrientation() ) );
		auto NewOrientation = RotateEuler( OrientationRadians );
		NewOrientation = glm::degrees( NewOrientation );
		Vector3D Orientation3D = { NewOrientation[0], NewOrientation[1], NewOrientation[2] };

		auto NewSize = Scale( B.GetSize() );
		Vector3D Size3D = { NewSize[0], NewSize[1], NewSize[2] };

		return FTransform( Position3D, Orientation3D, Size3D );
	}

private:
	void Dirty()
	{
		ShouldUpdate = true;
	}

	void Update()
	{
		if( ShouldUpdate )
		{
			static const glm::vec3 AxisX = glm::vec3( 1.0f, 0.0f, 0.0f );
			static const glm::vec3 AxisY = glm::vec3( 0.0f, 1.0f, 0.0f );
			static const glm::vec3 AxisZ = glm::vec3( 0.0f, 0.0f, 1.0f );

			ScaleMatrix = IdentityMatrix;
			ScaleMatrix.Scale( { StoredSize[0], StoredSize[1], StoredSize[2] } );

			Vector3D Radians = StoredOrientation;
			Radians.X = glm::radians( Radians.X );
			Radians.Y = glm::radians( Radians.Y );
			Radians.Z = glm::radians( Radians.Z );

			glm::quat Pitch = glm::angleAxis( Radians.X, AxisX );
			glm::quat Yaw = glm::angleAxis( Radians.Y, AxisY );
			glm::quat Roll = glm::angleAxis( Radians.Z, AxisZ );

			glm::quat Quaternion = Yaw * Pitch * Roll;
			RotationMatrix = Math::FromGLM( glm::toMat4( Quaternion ) );

			TranslationMatrix = IdentityMatrix;
			TranslationMatrix.Translate( { StoredPosition[0], StoredPosition[1], StoredPosition[2] } );
			// TranslationMatrix = Math::ToGLM( TranslateTest );
			// TranslationMatrix = glm::translate( IdentityMatrix, { StoredPosition[0], StoredPosition[1], StoredPosition[2] } );

			TransformationMatrix = TranslationMatrix * RotationMatrix * ScaleMatrix;
			TransformationMatrixInverse = Math::FromGLM( glm::inverse( Math::ToGLM( TransformationMatrix ) ) );
		}
	}

	Matrix4D TranslationMatrix;
	Matrix4D RotationMatrix;
	Matrix4D TransformationMatrix;
	Matrix4D TransformationMatrixInverse;
	Matrix4D ScaleMatrix;

	Vector3D StoredPosition;
	Vector3D StoredOrientation;
	Vector3D StoredSize;

	bool ShouldUpdate;
};
