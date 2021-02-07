// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PhysicsComponent.h"

#include <Engine/Display/Rendering/Mesh.h>

#include <Engine/Physics/Physics.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Physics/Body/Plane.h>

#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#include <Engine/Display/UserInterface.h>

#include <Game/Game.h>
#include <Engine/World/World.h>

static const auto Gravity = Vector3D( 0.0f, 0.0f, 9.81f );

#define DrawDebugLines 1
#define DrawCollisionResponseAABBAABB 0
#define DrawNormalAndDistance 0

size_t TextPosition = 0;

void CreateBVH( CMesh* Mesh, FTransform& Transform, const FBounds& WorldBounds, TriangleTree*& Tree, CBody* Body );

void EnsureVolume( FBounds& Bounds )
{
	if( Math::Equal( Bounds.Minimum.Y, Bounds.Maximum.Y ) )
	{
		Bounds.Minimum.Y -= 0.01f;
		Bounds.Maximum.Y += 0.01f;
	}

	if( Math::Equal( Bounds.Minimum.Y, Bounds.Maximum.Y ) )
	{
		Bounds.Minimum.Y -= 0.01f;
		Bounds.Maximum.Y += 0.01f;
	}

	if( Math::Equal( Bounds.Minimum.Z, Bounds.Maximum.Z ) )
	{
		Bounds.Minimum.Z -= 0.01f;
		Bounds.Maximum.Z += 0.01f;
	}
}

void VisualizeBounds( TriangleTree* Tree, FTransform* Transform = nullptr, Color BoundsColor = Color::Blue );

struct CollisionResponse
{
	Vector3D Point{ 0.0f, 0.0f, 0.0f };
	Vector3D Normal{ 0.0f, 0.0f, 0.0f };
	float Distance = 0.0f;
};

// Takes in world space bounds and returns the response normal.
CollisionResponse CollisionResponseAABBAABB( const FBounds& A, const FBounds& B )
{
	CollisionResponse Response;

	const Vector3D& MinimumA = A.Minimum;
	const Vector3D& MaximumA = A.Maximum;

	const Vector3D& MinimumB = B.Minimum;
	const Vector3D& MaximumB = B.Maximum;

	const Vector3D& CenterA = ( A.Maximum + A.Minimum ) * 0.5f;
	const Vector3D& CenterB = ( B.Maximum + B.Minimum ) * 0.5f;

	const Vector3D SizeA = ( MaximumA - MinimumA );
	const Vector3D SizeB = ( MaximumB - MinimumB );

	const Vector3D HalfSizeA = SizeA * 0.5f;
	const Vector3D HalfSizeB = SizeB * 0.5f;

	const Vector3D CenterDistance = ( CenterA - CenterB );
	const Vector3D CenterDistanceSquared = CenterDistance * CenterDistance;
	if( CenterDistanceSquared.Z > CenterDistanceSquared.Y && CenterDistanceSquared.Z > CenterDistanceSquared.X )
	{
		Response.Distance = fabs( CenterDistance.Z ) - ( HalfSizeA.Z + HalfSizeB.Z );
		Response.Normal.X = 0.0f;
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = Math::Sign( -CenterDistance.Z );
	}
	else if( CenterDistanceSquared.Y > CenterDistanceSquared.X )
	{
		Response.Distance = fabs( CenterDistance.Y ) - ( HalfSizeA.Y + HalfSizeB.Y );
		Response.Normal.X = 0.0f;
		Response.Normal.Y = Math::Sign( -CenterDistance.Y );
		Response.Normal.Z = 0.0f;
	}
	else
	{
		Response.Distance = fabs( CenterDistance.X ) - ( HalfSizeA.X + HalfSizeB.X );
		Response.Normal.X = Math::Sign( -CenterDistance.X );
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = 0.0f;
	}

	Response.Distance *= -1.0f;


#if DrawCollisionResponseAABBAABB == 1
	bool IsInFront = false;
	auto ScreenPosition = UI::WorldToScreenPosition( CenterA, &IsInFront );
	if( IsInFront )
	{
		ScreenPosition.X += 10.0f;

		ScreenPosition.Y += TextPosition * 15.0f;
		/*char VectorString[64];
		sprintf_s( VectorString, "c %.2f %.2f %.2f", CenterDistance.X, CenterDistance.Y, CenterDistance.Z );
		UI::AddText( ScreenPosition, VectorString );

		TextPosition++;*/

		auto Start = CenterA + Vector3D( -4.0f, 0.0f, 0.0f );
		auto End = Start + Response.Normal * Response.Distance;
		UI::AddLine( Start, End, Color( 0, 255, 255 ) );
		UI::AddCircle( End, 5.0f, Color( 0, 255, 255 ) );
	}
#endif

	return Response;
}

bool AxisTest( const Vector3D& Axis, const Vector3D& A, const Vector3D& B, const Vector3D& C, const float& Radius )
{
	const float PointA = A.Dot( Axis );
	const float PointB = B.Dot( Axis );
	const float PointC = C.Dot( Axis );
	return Math::Max( -Math::VectorMax( PointA, PointB, PointC ), Math::VectorMin( PointA, PointB, PointC ) ) > Radius;
}

void AddText( const Vector3D& A, const Vector3D& B, const Vector3D& C, const Color& Color )
{
	UI::AddText( A, "O", nullptr, Color );
	UI::AddText( B, "O", nullptr, Color );
	UI::AddText( C, "O", nullptr, Color );
}

CollisionResponse CollisionResponseTriangleAABB( const FVertex& A, const FVertex& B, const FVertex& C, const Vector3D& Center, const Vector3D& Extent )
{
	CollisionResponse Response;

	const Vector3D VertexA = A.Position - Center;
	const Vector3D VertexB = B.Position - Center;
	const Vector3D VertexC = C.Position - Center;

	const Vector3D FaceA = VertexB - VertexA;
	const Vector3D FaceB = VertexC - VertexB;
	const Vector3D FaceC = VertexA - VertexC;

	Vector3D A00 = Vector3D( 0.0f, -FaceA.Z, FaceA.Y );
	const float PointA = VertexA.Z * VertexB.Y - VertexA.Y * VertexB.Z;
	const float PointC = VertexC.Z * ( VertexB.Y - VertexA.Z ) - VertexC.Z * ( VertexB.Z - VertexA.Z );
	const float RadiusA00 = Extent.Y * fabs( FaceA.Z ) + Extent.Z * fabs( FaceA.Y );
	if( Math::Max( -Math::Max( PointA, PointC ), Math::Min( PointA, PointC ) ) > RadiusA00 )
	{
		return Response;
	}

	Vector3D A01 = Vector3D( 0.0f, -FaceB.Z, FaceB.Y );
	const float RadiusA01 = Extent.Y * fabs( FaceB.Z ) + Extent.Z * fabs( FaceB.Y );
	if( AxisTest( A01, VertexA, VertexB, VertexC, RadiusA01 ) )
	{
		return Response;
	}

	Vector3D A02 = Vector3D( 0.0f, -FaceC.Z, FaceC.Y );
	const float RadiusA02 = Extent.Y * fabs( FaceC.Z ) + Extent.Z * fabs( FaceC.Y );
	if( AxisTest( A02, VertexA, VertexB, VertexC, RadiusA02 ) )
	{
		return Response;
	}

	Vector3D A10 = Vector3D( FaceA.Z, 0.0f, -FaceA.X );
	const float RadiusA10 = Extent.X * fabs( FaceA.Z ) + Extent.Z * fabs( FaceA.X );
	if( AxisTest( A10, VertexA, VertexB, VertexC, RadiusA10 ) )
	{
		return Response;
	}

	Vector3D A11 = Vector3D( FaceB.Z, 0.0f, -FaceB.X );
	const float RadiusA11 = Extent.X * fabs( FaceB.Z ) + Extent.Z * fabs( FaceB.X );
	if( AxisTest( A11, VertexA, VertexB, VertexC, RadiusA11 ) )
	{
		return Response;
	}

	Vector3D A12 = Vector3D( FaceC.Z, 0.0f, -FaceC.X );
	const float RadiusA12 = Extent.X * fabs( FaceC.Z ) + Extent.Z * fabs( FaceC.X );
	if( AxisTest( A12, VertexA, VertexB, VertexC, RadiusA12 ) )
	{
		return Response;
	}

	Vector3D A20 = Vector3D( -FaceA.Y, FaceA.X, 0.0f );
	const float RadiusA20 = Extent.X * fabs( FaceA.Y ) + Extent.Z * fabs( FaceA.X );
	if( AxisTest( A20, VertexA, VertexB, VertexC, RadiusA20 ) )
	{
		return Response;
	}

	Vector3D A21 = Vector3D( -FaceB.Y, FaceB.X, 0.0f );
	const float RadiusA21 = Extent.X * fabs( FaceB.Y ) + Extent.Z * fabs( FaceB.X );
	if( AxisTest( A21, VertexA, VertexB, VertexC, RadiusA21 ) )
	{
		return Response;
	}

	Vector3D A22 = Vector3D( -FaceC.Y, FaceC.X, 0.0f );
	const float RadiusA22 = Extent.X * fabs( FaceC.Y ) + Extent.Z * fabs( FaceC.X );
	if( AxisTest( A22, VertexA, VertexB, VertexC, RadiusA22 ) )
	{
		return Response;
	}

	if( Math::VectorMax( VertexA.X, VertexB.X, VertexC.X ) < -Extent.X || Math::VectorMin( VertexA.X, VertexB.X, VertexC.X ) > Extent.X )
	{
		return Response;
	}

	if( Math::VectorMax( VertexA.Y, VertexB.Y, VertexC.Y ) < -Extent.Y || Math::VectorMin( VertexA.Y, VertexB.Y, VertexC.Y ) > Extent.Y )
	{
		return Response;
	}

	if( Math::VectorMax( VertexA.Z, VertexB.Z, VertexC.Z ) < -Extent.Z || Math::VectorMin( VertexA.Z, VertexB.Z, VertexC.Z ) > Extent.Z )
	{
		return Response;
	}

	// Response.Normal = FaceB.Cross( FaceA );
	Response.Normal += A.Normal + B.Normal + C.Normal;
	Response.Distance = fabs( Response.Normal.Dot( A.Normal ) );
	Response.Normal = Response.Normal.Normalized();

	/*const float Radius = Extent.X * fabs( Response.Normal.X ) + Extent.Y * fabs( Response.Normal.Y ) + Extent.Z * fabs( Response.Normal.Z );
	if( Response.Distance > Radius )
	{
		Response.Distance = Radius;
	}*/

	return Response;
}

CollisionResponse CollisionResponseTreeAABB( TriangleTree* Tree, const FBounds& WorldBounds, FTransform& BodyTransform )
{
	CollisionResponse Response;

	if( !Tree )
		return Response;

	FBounds LocalBounds = WorldBounds;
	auto& Matrix = BodyTransform.GetTransformationMatrixInverse();
	LocalBounds.Maximum = Matrix.Transform( LocalBounds.Maximum );
	LocalBounds.Minimum = Matrix.Transform( LocalBounds.Minimum );

	const Vector3D Center = ( LocalBounds.Maximum + LocalBounds.Minimum ) * 0.5f;
	Vector3D Extent = ( LocalBounds.Maximum - LocalBounds.Minimum ) * 0.5f;
	Extent.X = Math::Abs( Extent.X );
	Extent.Y = Math::Abs( Extent.Y );
	Extent.Z = Math::Abs( Extent.Z );

#if DrawDebugLines == 1
	// auto Position = BodyTransform.GetPosition();
	// FBounds DebugBounds = Tree->Bounds;
	// auto& DebugMatrix = BodyTransform.GetTransformationMatrix();
	// 
	// DebugBounds.Maximum = DebugMatrix.Transform( DebugBounds.Maximum );
	// DebugBounds.Minimum = DebugMatrix.Transform( DebugBounds.Minimum );
	// 
	// UI::AddAABB( Position - LocalBounds.Minimum, Position - LocalBounds.Maximum, Color::Green );
	// UI::AddAABB( Position - Tree->Bounds.Minimum, Position - Tree->Bounds.Maximum, Color::Blue );
	// 
	// UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, Color::Green );
	// UI::AddAABB( DebugBounds.Minimum, DebugBounds.Maximum, Color::Blue );
#endif

	auto& MinimumA = LocalBounds.Minimum;
	auto& MinimumB = Tree->Bounds.Minimum;

	auto& MaximumA = LocalBounds.Maximum;
	auto& MaximumB = Tree->Bounds.Maximum;

	int Intersects = 0;
	if( ( MinimumA.X < MaximumB.X && MaximumA.X > MinimumB.X ) )
	{
		Intersects++;
	}
	
	if( ( MinimumA.Y < MaximumB.Y && MaximumA.Y > MinimumB.Y ) )
	{
		Intersects++;
	}
	
	if( ( MinimumA.Z < MaximumB.Z && MaximumA.Z > MinimumB.Z ) )
	{
		Intersects++;
	}

	if( ( Intersects > 1 ) || Math::BoundingBoxIntersection( LocalBounds.Minimum, LocalBounds.Maximum, Tree->Bounds.Minimum, Tree->Bounds.Maximum ) )
	{
		if( !Tree->Upper || !Tree->Lower )
		{
#if DrawDebugLines == 1
			VisualizeBounds( Tree, &BodyTransform, Color::Red );
#endif

			for( unsigned int Index = 0; Index < Tree->Vertices.size(); )
			{
				const FVertex& VertexA = Tree->Vertices[Index];
				const FVertex& VertexB = Tree->Vertices[Index + 1];
				const FVertex& VertexC = Tree->Vertices[Index + 2];

				CollisionResponse TriangleResponse = CollisionResponseTriangleAABB( VertexA, VertexB, VertexC, Center, Extent );
				if( fabs( TriangleResponse.Distance ) > fabs( Response.Distance ) )
				{
					Response.Normal = TriangleResponse.Normal;
					Response.Distance = TriangleResponse.Distance;

#if DrawDebugLines == 1
					const Vector3D TriangleCenter = ( VertexA.Position + VertexB.Position + VertexC.Position ) / 3.0f;
					UI::AddLine( BodyTransform.Transform(VertexA.Position), BodyTransform.Transform(VertexA.Position + TriangleResponse.Normal * TriangleResponse.Distance), Color( 255, 0, 0 ) );
					UI::AddLine( BodyTransform.Transform(VertexB.Position), BodyTransform.Transform(VertexB.Position + TriangleResponse.Normal * TriangleResponse.Distance), Color( 0, 255, 0 ) );
					UI::AddLine( BodyTransform.Transform(VertexC.Position), BodyTransform.Transform(VertexC.Position + TriangleResponse.Normal * TriangleResponse.Distance), Color( 0, 0, 255 ) );
#endif
				}
#if DrawDebugLines == 1
				else
				{
					UI::AddLine( BodyTransform.Transform( VertexA.Position ), BodyTransform.Transform( VertexA.Position + TriangleResponse.Normal * TriangleResponse.Distance ), Color( 0, 0, 0 ) );
					UI::AddLine( BodyTransform.Transform( VertexB.Position ), BodyTransform.Transform( VertexB.Position + TriangleResponse.Normal * TriangleResponse.Distance ), Color( 0, 0, 0 ) );
					UI::AddLine( BodyTransform.Transform( VertexC.Position ), BodyTransform.Transform( VertexC.Position + TriangleResponse.Normal * TriangleResponse.Distance ), Color( 0, 0, 0 ) );
				}
#endif

				Index += 3;
			}
		}
		
		CollisionResponse ResponseA;
		CollisionResponse ResponseB;
		if( Tree->Upper )
		{
			ResponseA = CollisionResponseTreeAABB( Tree->Upper, WorldBounds, BodyTransform );
			if( fabs( ResponseA.Distance ) > fabs( Response.Distance ) )
			{
				Response.Normal = ResponseA.Normal;
				Response.Distance = ResponseA.Distance;
			}
		}

		if( Tree->Lower )
		{
			ResponseB = CollisionResponseTreeAABB( Tree->Lower, WorldBounds, BodyTransform );
			if( fabs( ResponseB.Distance ) > fabs( Response.Distance ) )
			{
				Response.Normal = ResponseB.Normal;
				Response.Distance = ResponseB.Distance;
			}
		}
	}

	Response.Normal = Response.Normal.Normalized();

	return Response;
}

//MakeBody()
//{
//	Owner = OwnerIn;
//	Block = true;
//	DeltaPosition = Vector3D( 0.0f, 0.0f, 0.0f );
//
//	this->LocalBounds = LocalBounds;
//	this->Static = Static;
//	this->Stationary = Stationary;
//
//	Tree = nullptr;
//}

CBody::CBody()
{
	CalculateBounds();
}

CBody::~CBody()
{
	if( Tree )
	{
		delete Tree;
	}
}

void CBody::Construct()
{
	CEntity* Entity = nullptr;
	if( Owner && Owner->ShouldCollide() )
	{
		Entity = Owner;
	}
	else if( Ghost )
	{
		Entity = Ghost;
	}

	if( Entity )
	{
		auto* World = Entity->GetWorld();
		if( World )
		{
			auto* Physics = World->GetPhysics();
			if( Physics )
			{
				Construct( Physics );
			}
		}
	}
}

void CBody::Construct( CPhysics* Physics )
{
	Physics->Register( this );
	CalculateBounds();

	PreviousTransform = GetTransform();
	DeltaPosition = Vector3D( 0.0f, 0.0f, 0.0f );
	Velocity = Vector3D( 0.0f, 0.0f, 0.0f );
	Depenetration = Vector3D( 0.0f, 0.0f, 0.0f );

	auto Transform = GetTransform();

	if( Owner && ( Owner->IsStatic() || Owner->IsStationary() ) && TriangleMesh )
	{
		auto* Mesh = Owner->CollisionMesh ? Owner->CollisionMesh : Owner->Mesh;
		CreateBVH( Mesh, Transform, LocalBounds, Tree, this );
	}
}

void CBody::PreCollision()
{
	Normal = Vector3D( 0.0f, 0.0f, 0.0f );
	Contact = false;
	Contacts = 0;
}

void CBody::Simulate()
{
	// We don't need to simulate environmental factors for bodies that don't move.
	if( Static || Stationary )
		return;

	// We can't apply environmental factors if we don't have an owner.
	if( !Owner )
		return;

	const float DeltaTime = GameLayersInstance->GetDeltaTime();

	// Calculate and apply gravity.
	Vector3D EnvironmentalForce = Vector3D::Zero;
	if( AffectedByGravity )
	{
		const Vector3D EnvironmentalAcceleration = -Gravity;
		EnvironmentalForce += ( EnvironmentalAcceleration * DeltaTime ) * InverseMass;
	}

	// Temporary friction factor that only applies to the XY plane.
	Velocity.X *= 0.9f;
	Velocity.Y *= 0.9f;

	auto Transform = GetTransform();
	Transform.SetPosition( Transform.GetPosition() + EnvironmentalForce );
	Owner->SetTransform( Transform );
}

bool CBody::Collision( CBody* Body )
{
	if( !Body->Block )
		return false;

	if( IsType<CPlaneBody>( Body ) )
		return false;

	if( ShouldIgnoreBody( Body ) )
		return false;

	const FBounds& BoundsA = Body->GetBounds();
	const FBounds& BoundsB = GetBounds();
	if( !Math::BoundingBoxIntersection( BoundsA.Minimum, BoundsA.Maximum, BoundsB.Minimum, BoundsB.Maximum ) )
		return false;

	const float DeltaTime = GameLayersInstance->GetDeltaTime();
	auto Transform = GetTransform();

	Contact = true;

	CollisionResponse Response;

	bool DebugUsingTree = false;
	// Dynamic interaction with static geometry.
	if( Static && !Body->Static )
	{
		if( Owner && Tree && TriangleMesh && !Body->Stationary )
		{
			Response = CollisionResponseTreeAABB( Tree, WorldBounds, PreviousTransform );
			DebugUsingTree = true;

		}
		else
		{
			Response = CollisionResponseAABBAABB( Body->WorldBounds, WorldBounds );
		}
	}
	else
	{
		Response = CollisionResponseAABBAABB( Body->WorldBounds, WorldBounds );
	}
	
	const Vector3D RelativeVelocity = Velocity - Body->Velocity;
	float VelocityAlongNormal = RelativeVelocity.Dot( Response.Normal ) + Response.Distance * InverseMass;

	bool Collided = false;
	if( VelocityAlongNormal < 0.0f )
	{
		Collided = true;
		Contacts++;
		const float Restitution = -1.01f;
		const float DeltaVelocity = Restitution * VelocityAlongNormal;
		const float Scale = DeltaVelocity / ( InverseMass + Body->InverseMass );
		Vector3D Impulse = Response.Normal * Scale;

		Normal += Response.Normal;

		if( Response.Distance > 0.0f && !Body->Static && !Body->Stationary )
		{
			auto Penetration = ( Response.Normal * VelocityAlongNormal );
			Body->Depenetration -= Penetration;
		}
		else
		{
			// Velocity -= InverseMass * Impulse;
			// Body->Velocity -= Body->InverseMass * Impulse;
		}

#if DrawNormalAndDistance == 1
		const auto BodyTransform = Body->GetTransform();
		// UI::AddLine( BodyTransform.GetPosition(), BodyTransform.GetPosition() + Response.Normal * Response.Distance, Color( 255, 0, 255 ) );
		UI::AddLine( BodyTransform.GetPosition(), BodyTransform.GetPosition() + Body->Depenetration, Color( 255, 0, 255 ) );
#endif

#if DrawDebugLines == 1
		auto Position = Transform.GetPosition();
		const auto ImpulseEnd = Position + Impulse + ( InverseMass * Impulse * Mass );
		UI::AddLine( Position, ImpulseEnd, Color( 255, 0, 0 ) );
		UI::AddCircle( ImpulseEnd, 5.0f, Color( 255, 0, DebugUsingTree ? 255 : 0 ) );
		UI::AddText( ImpulseEnd, std::to_string( VelocityAlongNormal ).c_str() );
#endif
	}

	if( !Static && !Stationary )
	{
		Vector3D ProjectionPosition = Response.Normal * Response.Distance;

#if DrawCollisionResponseAABBAABB == 1
		/*UI::AddAABB( WorldBounds.Minimum + ProjectionPosition, WorldBounds.Maximum + ProjectionPosition, Color( 255, 0, 127 ) );

		const Vector3D& CenterA = ( WorldBounds.Maximum + WorldBounds.Minimum ) * 0.5f;
		bool IsInFront = false;
		auto ScreenPosition = UI::WorldToScreenPosition( CenterA, &IsInFront );
		if( IsInFront )
		{
			ScreenPosition.X += 10.0f;

			ScreenPosition.Y += TextPosition * 15.0f;
			char VectorString[64];
			sprintf_s( VectorString, "rN %.2f %.2f %.2f (rD %.2f)", Response.Normal.X, Response.Normal.Y, Response.Normal.Z, Response.Distance );
			UI::AddText( ScreenPosition, VectorString );

			TextPosition++;
		}*/
#endif

#if DrawDebugLines == 1
		if( Contacts > 0 )
		{
			// UI::AddLine( WorldBounds.Minimum, Body->WorldBounds.Minimum, Color::White );
		}
#endif
	}

#if DrawDebugLines == 1
	Color BoundsColor = Color( 0, 255, 0 );
	if( Contacts < 1 )
	{
		BoundsColor = Color::Red;
	}
	else if( Contacts == 1 )
	{
		BoundsColor = Color( 255, 127, 0 );
	}
	else if( Contacts == 3 )
	{
		BoundsColor = Color( 255, 255, 0 );
	}
	else if( Contacts == 4 )
	{
		BoundsColor = Color( 127, 255, 0 );
	}

	UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, BoundsColor );

	if( Body->Static )
	{
		BoundsColor = Color::White;
		// UI::AddAABB( Body->WorldBounds.Minimum, Body->WorldBounds.Maximum, BoundsColor );
	}
#endif

	return Collided;
}

std::vector<FVertex> GatherVertices( std::vector<FVertex>& Vertices, const Vector3D& Median, const bool Direction, const size_t Axis )
{
	std::vector<FVertex> GatheredVertices;
	GatheredVertices.reserve( Vertices.size() );

	for( unsigned int Index = 0; Index < Vertices.size(); )
	{
		bool InsideBounds = false;

		Vector3D TriangleCenter = Vertices[Index].Position + Vertices[Index + 1].Position + Vertices[Index + 2].Position;
		TriangleCenter /= 3.0f;

		for( unsigned int TriangleIndex = 0; TriangleIndex < 3; TriangleIndex++ )
		{
			FVertex Vertex = Vertices[Index + TriangleIndex];
			if( Direction )
			{
				if( Axis == 0 && Vertex.Position.X >= Median.X )
				{
					InsideBounds = true;
				}
				else if( Axis == 1 && Vertex.Position.Y >= Median.Y )
				{
					InsideBounds = true;
				}
				else if( Axis == 2 && Vertex.Position.Z >= Median.Z )
				{
					InsideBounds = true;
				}
			}
			else
			{
				if( Axis == 0 && Vertex.Position.X < Median.X )
				{
					InsideBounds = true;
				}
				else if( Axis == 1 && Vertex.Position.Y < Median.Y )
				{
					InsideBounds = true;
				}
				else if( Axis == 2 && Vertex.Position.Z < Median.Z )
				{
					InsideBounds = true;
				}
			}

			if( InsideBounds )
				break;
		}
		
		if( InsideBounds )
		{
			GatheredVertices.emplace_back( Vertices[Index] );
			GatheredVertices.emplace_back( Vertices[Index + 1] );
			GatheredVertices.emplace_back( Vertices[Index + 2] );
		}

		Index += 3;
	}

	return GatheredVertices;
}

bool VectorCompare( const Vector3D& A, const Vector3D& B )
{
	return A.X < B.X
		&& A.Y < B.Y
		&& A.Z < B.Z;
}

void BuildMedian( TriangleTree*& Tree, FTransform& Transform, const FBounds& WorldBounds, const size_t Depth )
{
	if( !Tree )
	{
		Tree = new TriangleTree();
	}

	std::vector<FVertex>& Vertices = Tree->Vertices;

	Vector3D LocalMedian = Vector3D( 0.0f, 0.0f, 0.0f );
	for( size_t Index = 0; Index < Vertices.size(); Index++ )
	{
		LocalMedian += Vertices[Index].Position;
	}

	LocalMedian /= static_cast<float>( Vertices.size() );
	Vector3D Median = LocalMedian;// Transform.Transform( LocalMedian );

	// Median = ( WorldBounds.Minimum + WorldBounds.Maximum ) * 0.5f;

	FBounds UpperBounds = WorldBounds;
	FBounds LowerBounds = UpperBounds;

	size_t Axis = 0;
	Vector3D Size = ( WorldBounds.Maximum - WorldBounds.Minimum );

	if( Size.X > Size.Y && Size.X > Size.Z )
	{
		Axis = 0;
	}
	else if( Size.Y > Size.X && Size.Y > Size.Z )
	{
		Axis = 1;
	}
	else
	{
		Axis = 2;
	}

	if( Axis == 0 )
	{
		UpperBounds.Minimum.X = Math::Max( Median.X, UpperBounds.Minimum.X );
		UpperBounds.Maximum.X = Math::Max( Median.X, UpperBounds.Maximum.X );
		LowerBounds.Minimum.X = Math::Min( Median.X, LowerBounds.Minimum.X );
		LowerBounds.Maximum.X = Math::Min( Median.X, LowerBounds.Maximum.X );
	}
	else if( Axis == 1 )
	{
		UpperBounds.Minimum.Y = Math::Max( Median.Y, UpperBounds.Minimum.Y );
		UpperBounds.Maximum.Y = Math::Max( Median.Y, UpperBounds.Maximum.Y );
		LowerBounds.Minimum.Y = Math::Min( Median.Y, LowerBounds.Minimum.Y );
		LowerBounds.Maximum.Y = Math::Min( Median.Y, LowerBounds.Maximum.Y );
	}
	else if( Axis == 2 )
	{
		UpperBounds.Minimum.Z = Math::Max( Median.Z, UpperBounds.Minimum.Z );
		UpperBounds.Maximum.Z = Math::Max( Median.Z, UpperBounds.Maximum.Z );
		LowerBounds.Minimum.Z = Math::Min( Median.Z, LowerBounds.Minimum.Z );
		LowerBounds.Maximum.Z = Math::Min( Median.Z, LowerBounds.Maximum.Z );
	}

	Tree->Bounds = WorldBounds;

	// Ensure we have at least some volume.
	EnsureVolume( Tree->Bounds );

	if( Depth > 0 && Vertices.size() > 25 )
	{
		if( !Tree->Upper )
		{
			Tree->Upper = new TriangleTree();
		}

		if( !Tree->Lower )
		{
			Tree->Lower = new TriangleTree();
		}

		Tree->Upper->Vertices = GatherVertices( Vertices, Median, true, Axis );
		if( Tree->Upper->Vertices.size() == 0 )
		{
			delete Tree->Upper;
			Tree->Upper = nullptr;
		}
		else
		{
			BuildMedian( Tree->Upper, Transform, UpperBounds, Depth - 1 );
		}

		Tree->Lower->Vertices = GatherVertices( Vertices, Median, false, Axis );
		if( Tree->Lower->Vertices.size() == 0 )
		{
			delete Tree->Lower;
			Tree->Lower = nullptr;
		}
		else
		{
			BuildMedian( Tree->Lower, Transform, LowerBounds, Depth - 1 );
		}
	}
}

void VisualizeBounds( TriangleTree* Tree, FTransform* Transform, Color BoundsColor )
{
	if( Tree )
	{
		if( !Tree->Upper && !Tree->Lower )
		{
			if( Tree->Vertices.size() > 0 )
			{
				Vector3D Median = ( Tree->Bounds.Maximum + Tree->Bounds.Minimum ) * 0.5f;

				if( Transform )
				{
					Median = Transform->Transform( Median );
				}

				// UI::AddText( Median, "Median" );

				if( Transform )
				{
					const auto& Minimum = Tree->Bounds.Minimum;
					const auto& Maximum = Tree->Bounds.Maximum;

					Vector3D Positions[8];
					Positions[0] = Vector3D( Minimum.X, Maximum.Y, Minimum.Z );
					Positions[1] = Vector3D( Maximum.X, Maximum.Y, Minimum.Z );
					Positions[2] = Vector3D( Maximum.X, Minimum.Y, Minimum.Z );
					Positions[3] = Vector3D( Minimum.X, Minimum.Y, Minimum.Z );

					Positions[4] = Vector3D( Minimum.X, Maximum.Y, Maximum.Z );
					Positions[5] = Vector3D( Maximum.X, Maximum.Y, Maximum.Z );
					Positions[6] = Vector3D( Maximum.X, Minimum.Y, Maximum.Z );
					Positions[7] = Vector3D( Minimum.X, Minimum.Y, Maximum.Z );

					for( size_t PositionIndex = 0; PositionIndex < 8; PositionIndex++ )
					{
						Positions[PositionIndex] = Transform->Transform( Positions[PositionIndex] );
					}

					FBounds TransformedAABB = Math::AABB( Positions, 8 );

					UI::AddAABB( TransformedAABB.Minimum, TransformedAABB.Maximum, BoundsColor );
				}
				else
				{
					UI::AddAABB( Tree->Bounds.Minimum, Tree->Bounds.Maximum, BoundsColor );
				}
				

				for( size_t Index = 0; Index < Tree->Vertices.size(); )
				{
					FVertex VertexA = Tree->Vertices[Index];
					FVertex VertexB = Tree->Vertices[Index + 1];
					FVertex VertexC = Tree->Vertices[Index + 2];

					if( Transform )
					{
						VertexA.Position = Transform->Transform( VertexA.Position );
					}

					// UI::AddText( VertexA, std::to_string( Index ).c_str() );

					Index += 3;
				}
			}
		}

		VisualizeBounds( Tree->Upper, Transform );
		VisualizeBounds( Tree->Lower, Transform );
	}
}

void CreateBVH( CMesh* Mesh, FTransform& Transform, const FBounds& WorldBounds, TriangleTree*& Tree, CBody* Body )
{
	if( !Tree )
	{
		Tree = new TriangleTree();

		const auto& VertexData = Mesh->GetVertexData();
		const auto& IndexData = Mesh->GetIndexData();
		const auto& VertexBufferData = Mesh->GetVertexBufferData();

		std::vector<FVertex>& Vertices = Tree->Vertices;
		Vertices.reserve( VertexBufferData.IndexCount );

		for( unsigned int Index = 0; Index < VertexBufferData.IndexCount; )
		{
			Vertices.emplace_back( VertexData.Vertices[IndexData.Indices[Index]] );
			Vertices.emplace_back( VertexData.Vertices[IndexData.Indices[Index] + 1] );
			Vertices.emplace_back( VertexData.Vertices[IndexData.Indices[Index] + 2] );

			Index += 3;
		}

		BuildMedian( Tree, Transform, Mesh->GetBounds(), 16 );
	}

	// VisualizeBounds( Tree );
}

void CBody::Tick()
{
	Handled = false;

	auto Transform = GetTransform();	
	if( Static )
		return;

	CalculateBounds();

	auto NewPosition = Transform.GetPosition();

#if DrawDebugLines == 1
	UI::AddLine( NewPosition, NewPosition + Velocity, Color( 32, 255, 255 ) );
#endif

	if( !Stationary )
	{
		// Velocity -= Depenetration;
		NewPosition += Velocity;

		const float MinimumHeight = -21.0f;
		if( NewPosition.Z < MinimumHeight )
		{
			if( Velocity.Z < 0.0f )
			{
				Velocity.Z = 0.0f;
			}

			NewPosition.Z = MinimumHeight;
			Normal = Vector3D( 0.0f, 0.0f, -1.0f );
			Contact = true;
		}

		// NormalForce = Velocity.Dot( Normal ) * Normal;

		if( Contacts > 0 )
		{
			Contact = true;
			// Velocity = Math::Abs( Velocity.Dot( Depenetration ) ) * Velocity;
			const float Factor = Math::Saturate( ( Depenetration.Normalized().Dot( Velocity.Normalized() ) * -1.0f ) + 1.0f );
			// UI::AddLine( NewPosition, NewPosition + Velocity.Normalized(), Color( 255, 0, 255 ) );
			// UI::AddLine( NewPosition, NewPosition - Depenetration.Normalized(), Color( 0, 255, 255 ) );
			// UI::AddText( NewPosition, std::to_string( Factor ).c_str() );
			Velocity *= Factor;
		}
		else
		{
			Velocity = NewPosition - PreviousTransform.GetPosition();
		}

		// Apply depenetration.
		NewPosition -= Depenetration;
		Normal = Depenetration.Normalized() * -1.0f;
		Depenetration = Vector3D( 0.0f, 0.0f, 0.0f );
	}

	Transform.SetPosition( NewPosition );

	Contact = Contact || Contacts > 0;

	if( Owner )
	{
		Owner->SetTransform( Transform );
		Owner->Contact = Contact;

		PreviousTransform = Owner->GetTransform();
	}

	CalculateBounds();
	Acceleration = Vector3D( 0.0f, 0.0f, 0.0f );
	TextPosition = 0;
}

void CBody::Destroy( CPhysics* Physics )
{
	Physics->Unregister( this );
}

void CBody::CalculateBounds()
{
	FTransform Transform = GetTransform();
	WorldBounds = Math::AABB( LocalBounds, Transform );

	// Ensure we have at least some volume.
	EnsureVolume( WorldBounds );

	Mass = ( WorldBounds.Maximum - WorldBounds.Minimum ).Length() * 45.0f;

	const bool Unmoving = Static || Stationary;
	InverseMass = Unmoving ? 0.0f : 1.0f / Mass;
}

FBounds CBody::GetBounds() const
{
	return WorldBounds;
}

const FTransform& CBody::GetTransform() const
{
	if( Owner )
	{
		return Owner->GetTransform();
	}
	else if( Ghost )
	{
		auto* Point = Cast<CPointEntity>( Ghost );
		if( Point )
		{
			return Point->GetTransform();
		}
	}

	// Couldn't fetch the entity transform.
	static const FTransform Identity = FTransform();
	return Identity;
}

void CBody::Debug()
{
	if( !Owner && !Ghost )
		return;

	Color BoundsColor = Color( 0, 255, 0 );
	if( Contacts < 1 )
	{
		BoundsColor = Color::Red;
	}
	else if( Contacts == 1 )
	{
		BoundsColor = Color( 255, 127, 0 );
	}
	else if( Contacts == 3 )
	{
		BoundsColor = Color( 255, 255, 0 );
	}
	else if( Contacts == 4 )
	{
		BoundsColor = Color( 127, 255, 0 );
	}

	UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, BoundsColor );

	// VisualizeBounds( Tree, &PreviousTransform );
}

BodyType CBody::GetType() const
{
	return BodyType::AABB;
}

bool CBody::ShouldIgnoreBody( CBody* Body ) const
{
	const auto* MeshEntity = Cast<CMeshEntity>( Body->Owner );
	if( !MeshEntity )
		return false;

	for( const auto* IgnoredBody : IgnoredBodies )
	{
		if( IgnoredBody == MeshEntity )
			return true;
	}

	return false;
}

void CBody::Ignore( CBody* Body, const bool Clear )
{
	Ignore( Cast<CMeshEntity>( Body->Owner ), Clear );
}

void CBody::Ignore( CMeshEntity* Entity, const bool Clear )
{
	if( !Entity )
		return;

	if( Clear )
	{
		const auto DiscoveredBody = IgnoredBodies.find( Entity );
		if( DiscoveredBody != IgnoredBodies.end() )
		{
			IgnoredBodies.erase( DiscoveredBody );
		}
	}
	else
	{
		IgnoredBodies.insert( Entity );
	}
}
