// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PhysicsComponent.h"

#include <Engine/Display/Rendering/Mesh.h>

#include <Engine/Physics/Physics.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Physics/Body/Plane.h>

#include <Engine/Physics/Geometry.h>

#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#include <Engine/Display/UserInterface.h>

#include <Engine/World/World.h>

constexpr auto GravityAcceleration = 9.81f;
static const auto Gravity = Vector3D( 0.0f, 0.0f, GravityAcceleration );

#define DrawDebugLines 0
#define DrawCollisionResponseAABBAABB 0
#define DrawNormalAndDistance 0

size_t TextPosition = 0;

void CreateBVH( CMesh* Mesh, FTransform& Transform, const BoundingBox& WorldBounds, TriangleTree*& Tree, CBody* Body );

void EnsureVolume( BoundingBox& Bounds )
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

void VisualizeBounds( TriangleTree* Tree, const FTransform* Transform = nullptr, const Color& BoundsColor = Color::Blue );

bool SweptIntersection( const BoundingBox& ContinuousBounds, Vector3D& ContinuousVelocity, const BoundingBox& B, Geometry::Result& Result )
{
	const bool Intersecting = Math::BoundingBoxIntersection( ContinuousBounds.Minimum, ContinuousBounds.Maximum, B.Minimum, B.Maximum );
	if( Intersecting )
		return true;

	UI::AddLine( ContinuousBounds.Minimum, ContinuousBounds.Minimum + ContinuousVelocity, Color::Blue, 0.5f );

	// Calculate where we want to be.
	auto Target = BoundingBox(
		ContinuousBounds.Minimum + ContinuousVelocity,
		ContinuousBounds.Maximum + ContinuousVelocity
	);

	// Retrieve the points of the bounding box.
	const auto SourceVertices = BoundingPoints( ContinuousBounds );
	const auto TargetVertices = BoundingPoints( Target );

	// Test if the boxes intersect.
	for( size_t Index = 0; Index < BoundingPoints::Count; Index++ )
	{
		UI::AddLine( SourceVertices.Position[Index], SourceVertices.Position[Index] + ContinuousVelocity, Color::White, 0.5f );
		UI::AddLine( SourceVertices.Position[Index], TargetVertices.Position[Index], Color::Green, 0.5f );

		Result = Geometry::LineInBoundingBox( SourceVertices.Position[Index], TargetVertices.Position[Index], B );
		if( Result.Hit )
		{
			// Update the velocity?
			ContinuousVelocity = (Result.Position - SourceVertices.Position[Index]).Normalized() * ContinuousVelocity.Length();
			UI::AddLine( SourceVertices.Position[Index], Result.Position, Color::Red, 10.0 );
			break;
		}
	}

	if( Result.Hit )
	{
		UI::AddAABB( ContinuousBounds.Minimum, ContinuousBounds.Maximum, Color::Blue, 10.0 );
		UI::AddAABB( Target.Minimum, Target.Maximum, Color::Yellow, 10.0 );
		UI::AddAABB( B.Minimum, B.Maximum, Color::Red, 10.0 );

		// Recalculate the new target.
		Target = BoundingBox(
			ContinuousBounds.Minimum + ContinuousVelocity,
			ContinuousBounds.Maximum + ContinuousVelocity
		);

		const auto Offset = Target.Minimum - ContinuousBounds.Minimum;
		const auto Unit = Offset.Normalized();
		
		Target = ContinuousBounds;
		Target.Minimum += Unit * Result.Distance;
		Target.Maximum += Unit * Result.Distance;
		UI::AddAABB( Target.Minimum, Target.Maximum, Color::Purple, 10.0 );

		UI::AddLine( Result.Position, Result.Position + WorldUp, Color::Green, 10.0 );

		return true;
	}
	else
	{
		UI::AddAABB( ContinuousBounds.Minimum, ContinuousBounds.Maximum, Color::Black );
	}

	return Intersecting;
}

// Takes in world space bounds and returns the response normal.
CollisionResponse CollisionResponseAABBAABB( const BoundingBox& A, const BoundingBox& B )
{
	CollisionResponse Response;

	const Vector3D& CenterA = A.Center();
	const Vector3D& CenterB = B.Center();

	const Vector3D SizeA = A.Size();
	const Vector3D SizeB = B.Size();

	const Vector3D HalfSizeA = SizeA * 0.5f;
	const Vector3D HalfSizeB = SizeB * 0.5f;

	const Vector3D CenterDistance = ( CenterB - CenterA );
	const auto Overlap = ( HalfSizeA + HalfSizeB ) - Math::Abs( CenterDistance );
	if( Overlap.X < Overlap.Y && Overlap.X < Overlap.Z )
	{
		Response.Distance = Overlap.X;
		Response.Normal.X = HalfSizeA.X * Math::Sign( CenterDistance.X );
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = 0.0f;
	}
	else if( Overlap.Y < Overlap.Z )
	{
		Response.Distance = Overlap.Y;
		Response.Normal.X = 0.0f;
		Response.Normal.Y = HalfSizeA.Y + Math::Sign( CenterDistance.Y );
		Response.Normal.Z = 0.0f;
	}
	else
	{
		Response.Distance = Overlap.Z;
		Response.Normal.X = 0.0f;
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = HalfSizeA.Z + Math::Sign( CenterDistance.Z );
	}

	return Response;
}

CollisionResponse CollisionResponseAABBAABBSwept( const BoundingBox& A, const Vector3D& Velocity, const BoundingBox& B, const Geometry::Result& Result )
{
	CollisionResponse Response;

	if( !Result.Hit )
		return CollisionResponseAABBAABB( A, B );

	auto Target = BoundingBox(
		A.Minimum + Velocity,
		A.Maximum + Velocity
	);

	const auto Offset = Target.Minimum - A.Minimum;
	const auto Unit = Offset.Normalized();

	Response.Distance = Result.Distance;
	Response.Normal = Unit;

	// Target = A;
	// Target.Minimum += Unit * Result.Distance;
	// Target.Maximum += Unit * Result.Distance;
	// UI::AddAABB( Target.Minimum, Target.Maximum, Color::Purple, 10.0 );

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

	// Direction vector pointing from B to A.
	const Vector3D EdgeA = VertexB - VertexA;

	// Direction vector pointing from C to B.
	const Vector3D EdgeB = VertexC - VertexB;

	// Direction vector pointing from A to C.
	const Vector3D EdgeC = VertexA - VertexC;

	Vector3D A00 = Vector3D( 0.0f, -EdgeA.Z, EdgeA.Y );
	const float PointA = VertexA.Z * VertexB.Y - VertexA.Y * VertexB.Z;
	const float PointC = VertexC.Z * ( VertexB.Y - VertexA.Z ) - VertexC.Z * ( VertexB.Z - VertexA.Z );
	const float RadiusA00 = Extent.Y * Math::Abs( EdgeA.Z ) + Extent.Z * Math::Abs( EdgeA.Y );
	if( Math::Max( -Math::Max( PointA, PointC ), Math::Min( PointA, PointC ) ) > RadiusA00 )
	{
		return Response;
	}

	Vector3D A01 = Vector3D( 0.0f, -EdgeB.Z, EdgeB.Y );
	const float RadiusA01 = Extent.Y * Math::Abs( EdgeB.Z ) + Extent.Z * Math::Abs( EdgeB.Y );
	if( AxisTest( A01, VertexA, VertexB, VertexC, RadiusA01 ) )
	{
		return Response;
	}

	Vector3D A02 = Vector3D( 0.0f, -EdgeC.Z, EdgeC.Y );
	const float RadiusA02 = Extent.Y * Math::Abs( EdgeC.Z ) + Extent.Z * Math::Abs( EdgeC.Y );
	if( AxisTest( A02, VertexA, VertexB, VertexC, RadiusA02 ) )
	{
		return Response;
	}

	Vector3D A10 = Vector3D( EdgeA.Z, 0.0f, -EdgeA.X );
	const float RadiusA10 = Extent.X * Math::Abs( EdgeA.Z ) + Extent.Z * Math::Abs( EdgeA.X );
	if( AxisTest( A10, VertexA, VertexB, VertexC, RadiusA10 ) )
	{
		return Response;
	}

	Vector3D A11 = Vector3D( EdgeB.Z, 0.0f, -EdgeB.X );
	const float RadiusA11 = Extent.X * Math::Abs( EdgeB.Z ) + Extent.Z * Math::Abs( EdgeB.X );
	if( AxisTest( A11, VertexA, VertexB, VertexC, RadiusA11 ) )
	{
		return Response;
	}

	Vector3D A12 = Vector3D( EdgeC.Z, 0.0f, -EdgeC.X );
	const float RadiusA12 = Extent.X * Math::Abs( EdgeC.Z ) + Extent.Z * Math::Abs( EdgeC.X );
	if( AxisTest( A12, VertexA, VertexB, VertexC, RadiusA12 ) )
	{
		return Response;
	}

	Vector3D A20 = Vector3D( -EdgeA.Y, EdgeA.X, 0.0f );
	const float RadiusA20 = Extent.X * Math::Abs( EdgeA.Y ) + Extent.Z * Math::Abs( EdgeA.X );
	if( AxisTest( A20, VertexA, VertexB, VertexC, RadiusA20 ) )
	{
		return Response;
	}

	Vector3D A21 = Vector3D( -EdgeB.Y, EdgeB.X, 0.0f );
	const float RadiusA21 = Extent.X * Math::Abs( EdgeB.Y ) + Extent.Z * Math::Abs( EdgeB.X );
	if( AxisTest( A21, VertexA, VertexB, VertexC, RadiusA21 ) )
	{
		return Response;
	}

	Vector3D A22 = Vector3D( -EdgeC.Y, EdgeC.X, 0.0f );
	const float RadiusA22 = Extent.X * Math::Abs( EdgeC.Y ) + Extent.Z * Math::Abs( EdgeC.X );
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

	Response.Normal = EdgeB.Cross( EdgeA );
	// Response.Normal -= A.Normal + B.Normal + C.Normal;
	Response.Distance = Math::Abs( Response.Normal.Dot( VertexA ) );

	const float Radius = Extent.X * Math::Abs( Response.Normal.X ) + Extent.Y * Math::Abs( Response.Normal.Y ) + Extent.Z * Math::Abs( Response.Normal.Z );
	const float DistanceFromCenter = Response.Normal.Dot( Extent ) - Response.Distance;
	Response.Distance = Radius * DistanceFromCenter;

	return Response;
}

CollisionResponse CollisionResponseTreeAABB( TriangleTree* Tree, const BoundingBox& WorldBounds, FTransform& Transform, FTransform& OtherTransform )
{
	CollisionResponse Response;

	if( !Tree )
		return Response;

	BoundingBox LocalBounds = Math::AABB( WorldBounds, Transform.GetTransformationMatrixInverse() );

	const Vector3D Center = ( LocalBounds.Maximum + LocalBounds.Minimum ) * 0.5f;
	Vector3D Extent = ( LocalBounds.Maximum - LocalBounds.Minimum ) * 0.5f;
	Extent.X = Math::Abs( Extent.X );
	Extent.Y = Math::Abs( Extent.Y );
	Extent.Z = Math::Abs( Extent.Z );

	const Vector3D WorldCenter = Transform.Transform( Center );

	/*FBounds TreeRelativeBounds = Math::AABB( LocalBounds, Transform );
	UI::AddAABB( TreeRelativeBounds.Minimum, TreeRelativeBounds.Maximum );

	FBounds TreeBounds = Math::AABB( Tree->Bounds, Transform );
	UI::AddAABB( TreeBounds.Minimum, TreeBounds.Maximum, Color( 255, 255, 0 ) );*/

	auto& MinimumA = LocalBounds.Minimum;
	auto& MinimumB = Tree->Bounds.Minimum;

	auto& MaximumA = LocalBounds.Maximum;
	auto& MaximumB = Tree->Bounds.Maximum;

	int Intersects = 0;
	/*if( ( MinimumA.X < MaximumB.X && MaximumA.X > MinimumB.X ) )
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
	}*/

	if( ( Intersects > 1 ) || Math::BoundingBoxIntersection( LocalBounds.Minimum, LocalBounds.Maximum, Tree->Bounds.Minimum, Tree->Bounds.Maximum ) )
	{
		Response.Distance = -1.0f;
		if( !Tree->Upper || !Tree->Lower )
		{
#if DrawDebugLines == 1
			// VisualizeBounds( Tree, &Transform, Color::Red );
#endif

			for( unsigned int Index = 0; Index < Tree->Vertices.size(); )
			{
				const FVertex& VertexA = Tree->Vertices[Index];
				const FVertex& VertexB = Tree->Vertices[Index + 1];
				const FVertex& VertexC = Tree->Vertices[Index + 2];

				CollisionResponse TriangleResponse = CollisionResponseTriangleAABB( VertexA, VertexB, VertexC, Center, Extent );
				// if( Math::Abs( TriangleResponse.Distance ) < Math::Abs( Response.Distance ) || Response.Distance < 0.0001f )
				if( Math::Abs( TriangleResponse.Distance ) < Math::Abs( Response.Distance ) )
				{
					Response.Normal = TriangleResponse.Normal;
					Response.Distance = TriangleResponse.Distance;

#if DrawDebugLines == 1
					// UI::AddLine( Transform.Transform( VertexA.Position ), Transform.Transform( VertexA.Position + TriangleResponse.Normal * TriangleResponse.Distance ), Color( 255, 0, 0 ) );
					// UI::AddLine( Transform.Transform( VertexB.Position ), Transform.Transform( VertexB.Position + TriangleResponse.Normal * TriangleResponse.Distance ), Color( 0, 255, 0 ) );
					// UI::AddLine( Transform.Transform( VertexC.Position ), Transform.Transform( VertexC.Position + TriangleResponse.Normal * TriangleResponse.Distance ), Color( 0, 0, 255 ) );
#endif
				}
#if DrawDebugLines == 1
				else
				{
					// UI::AddLine( Transform.Transform( VertexA.Position ), Transform.Transform( VertexA.Position + TriangleResponse.Normal * TriangleResponse.Distance ), Color( 0, 0, 0 ) );
					// UI::AddLine( Transform.Transform( VertexB.Position ), Transform.Transform( VertexB.Position + TriangleResponse.Normal * TriangleResponse.Distance ), Color( 0, 0, 0 ) );
					// UI::AddLine( Transform.Transform( VertexC.Position ), Transform.Transform( VertexC.Position + TriangleResponse.Normal * TriangleResponse.Distance ), Color( 0, 0, 0 ) );
				}
#endif

				Index += 3;
			}
		}
		
		CollisionResponse ResponseA;
		CollisionResponse ResponseB;
		if( Tree->Upper )
		{
			ResponseA = CollisionResponseTreeAABB( Tree->Upper, WorldBounds, Transform, OtherTransform );
			// if( Math::Abs( ResponseA.Distance ) < Math::Abs( Response.Distance ) || Response.Distance < 0.0001f )
			if( Math::Abs( ResponseA.Distance ) < Math::Abs( Response.Distance ) )
			{
				Response.Normal = ResponseA.Normal;
				Response.Distance = ResponseA.Distance;
			}
		}

		if( Tree->Lower )
		{
			ResponseB = CollisionResponseTreeAABB( Tree->Lower, WorldBounds, Transform, OtherTransform );
			// if( Math::Abs( ResponseB.Distance ) < Math::Abs( Response.Distance ) || Response.Distance < 0.0001f )
			if( Math::Abs( ResponseB.Distance ) < Math::Abs( Response.Distance ) )
			{
				Response.Normal = ResponseB.Normal;
				Response.Distance = ResponseB.Distance;
			}
		}
	}

	// Response.Normal = Transform.Transform( Response.Normal );
	
	// UI::AddLine( WorldCenter, WorldCenter + Response.Normal );

	Response.Normal = Response.Normal.Normalized();

	if( Response.Distance < 0.0f )
	{
		Response.Distance = 0.0f;
	}

	return Response;
}

CBody::~CBody()
{
	delete Tree;
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
	if( !Physics )
		return;
	
	this->Physics = Physics;
	Physics->Register( this );
	CalculateBounds();

	PreviousTransform = GetTransform();

	auto Transform = GetTransform();
	if( Owner && ( Owner->IsStatic() || Owner->IsStationary() ) && TriangleMesh )
	{
		auto* Mesh = Owner->CollisionMesh ? Owner->CollisionMesh : Owner->Mesh;

		// TODO: Fix triangle tree and triangle collisions.
		CreateBVH( Mesh, Transform, LocalBounds, Tree, this );
	}
}

void CBody::PreCollision()
{
	Normal = Vector3D::Zero;
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

	// Calculate and apply gravity.
	Vector3D EnvironmentalForce = Vector3D::Zero;
	if( AffectedByGravity && Math::Equal( Acceleration, Vector3D::Zero ) )
	{
		EnvironmentalForce -= Gravity;
	}

	Acceleration += EnvironmentalForce;
}

CollisionResponse CalculateResponse( CBody* A, CBody* B, const Geometry::Result& SweptResult )
{
	CollisionResponse Response;

	if( A->Static && !B->Static )
	{
		if( A->Owner && A->Tree && A->TriangleMesh && !B->Stationary )
		{
			Response = CollisionResponseTreeAABB( A->Tree, B->WorldBounds, A->PreviousTransform, B->PreviousTransform );
		}
		else
		{
			if( B->Continuous )
			{
				Response = CollisionResponseAABBAABBSwept( B->WorldBounds, B->Velocity, A->WorldBounds, SweptResult );
			}
			else
			{
				Response = CollisionResponseAABBAABB( B->WorldBounds, A->WorldBounds );
			}
		}
	}
	else if( !A->Static || !B->Static )
	{
		if( B->Continuous )
		{
			Response = CollisionResponseAABBAABBSwept( B->WorldBounds, B->Velocity, A->WorldBounds, SweptResult );
		}
		else
		{
			Response = CollisionResponseAABBAABB( B->WorldBounds, A->WorldBounds );
		}
	}

	return Response;
}

void Interpenetration( CBody* A, CBody* B, CollisionResponse& Response )
{
	if( A->TriangleMesh || Response.Distance <= 0.001f || B->Static || B->Stationary )
		return;

	const auto Penetration = ( Response.Normal * Response.Distance );
	B->Depenetration += Penetration;
	B->Velocity -= Penetration;

	Response.Distance *= -1.0f;
}

void Friction( CBody* A, CBody* B, const CollisionResponse& Response, const Vector3D& RelativeVelocity, const float& InverseMassTotal, const float& ImpulseScale )
{
	auto Tangent = RelativeVelocity - ( Response.Normal * RelativeVelocity.Dot( Response.Normal ) );
	const auto ValidTangent = !Math::Equal( Tangent.LengthSquared(), 0.0f );
	if( !ValidTangent )
		return;

	Tangent.Normalize();

	auto FrictionScale = RelativeVelocity.Dot( Tangent ) * -1.0f / InverseMassTotal;
	const auto ValidFrictionScale = !Math::Equal( FrictionScale, 0.0f );
	if( ValidFrictionScale )
	{
		constexpr float FixedFriction = 1.0f;
		const auto Friction = sqrtf( FixedFriction );
		if( FrictionScale > ImpulseScale * Friction )
		{
			FrictionScale = ImpulseScale * Friction;
		}
		else if( FrictionScale < -ImpulseScale * Friction )
		{
			FrictionScale = -ImpulseScale * Friction;
		}

		const auto TangentImpulse = Tangent * FrictionScale;
		// const auto TangentImpulse = Tangent * ( RelativeVelocity.Dot( Tangent ) * -1.0f * 0.0f );
		A->Velocity -= A->InverseMass * TangentImpulse;
		B->Velocity += B->InverseMass * TangentImpulse;
	}
}

bool Integrate( CBody* A, CBody* B, const Geometry::Result& SweptResult )
{
	const auto InverseMassTotal = A->InverseMass + B->InverseMass;
	if( InverseMassTotal <= 0.0f )
		return false;

	CollisionResponse Response = CalculateResponse( A, B, SweptResult );

	const Vector3D RelativeVelocity = A->Velocity - B->Velocity;
	// float VelocityAlongNormal = RelativeVelocity.Dot( Response.Normal ) + Response.Distance * InverseMass; // Old calculation of the seperating velocity.

	const auto SeparatingVelocity = RelativeVelocity.Dot( Response.Normal );
	if( SeparatingVelocity > 0.0f ) // Check if the bodies are moving away from each other.
		return false;

	float DeltaVelocity = -SeparatingVelocity * B->Restitution;

	const auto AccelerationCausedVelocity = ( A->Acceleration - B->Acceleration ).Dot( Response.Normal ) * ( 1.0f / 60.0f );
	if( AccelerationCausedVelocity < 0.0f )
	{
		DeltaVelocity += B->Restitution * AccelerationCausedVelocity;

		DeltaVelocity = Math::Max( 0.0f, DeltaVelocity );
	}

	DeltaVelocity -= SeparatingVelocity;

	const float ImpulseScale = DeltaVelocity / InverseMassTotal;
	const Vector3D Impulse = Response.Normal * ImpulseScale;

	A->Normal += Response.Normal;

	Interpenetration( A, B, Response );

	A->Velocity += A->InverseMass * Impulse;
	B->Velocity -= B->InverseMass * Impulse;

	Friction( A, B, Response, RelativeVelocity, InverseMassTotal, ImpulseScale );

	return true;
}

bool CBody::Collision( CBody* Body )
{
	if( !Body->Block )
		return false;

	if( ShouldIgnoreBody( Body ) )
		return false;

	const BoundingBox& BoundsA = Body->GetBounds();
	const BoundingBox& BoundsB = GetBounds();

	Geometry::Result SweptResult;
	if( Body->Continuous )
	{
		if( !SweptIntersection( BoundsA, Body->LinearVelocity, BoundsB, SweptResult ) )
			return false;
	}
	else
	{
		if( !Math::BoundingBoxIntersection( BoundsA.Minimum, BoundsA.Maximum, BoundsB.Minimum, BoundsB.Maximum ) )
			return false;
	}

	if( IsType<CPlaneBody>( Body ) )
		return false;
	auto Transform = GetTransform();

	Contact = true;

	bool Collided = false;

	Integrate( this, Body, SweptResult );

	Collided = true;
	Contacts++;
	Body->Contacts++;

#if DrawNormalAndDistance == 1
	const auto BodyTransform = Body->GetTransform();
	// UI::AddLine( BodyTransform.GetPosition(), BodyTransform.GetPosition() + Response.Normal * Response.Distance, Color( 255, 0, 255 ) );
	UI::AddLine( BodyTransform.GetPosition(), BodyTransform.GetPosition() + Body->Depenetration, Color( 255, 0, 255 ) );
#endif

#if DrawDebugLines == 1
	auto Position = Transform.GetPosition();
	const auto ImpulseEnd = Position + Impulse + ( InverseMass * Impulse * Mass );
	UI::AddLine( Position, Position + Response.Normal.Normalized(), Color( 0, 255, 0 ) );
	UI::AddLine( Position, ImpulseEnd, Color( 255, 0, 0 ) );
	UI::AddCircle( ImpulseEnd, 5.0f, Color( 255, 0, 255 ) );
	UI::AddText( ImpulseEnd, std::to_string( VelocityAlongNormal ).c_str() );
#endif

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

	if( Contacts > 0 )
	{
		UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, BoundsColor );
	}

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

void BuildMedian( TriangleTree*& Tree, FTransform& Transform, const BoundingBox& WorldBounds, const size_t Depth )
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

	BoundingBox UpperBounds = WorldBounds;
	BoundingBox LowerBounds = UpperBounds;

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

void VisualizeBounds( TriangleTree* Tree, const FTransform* Transform, const Color& BoundsColor )
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

					BoundingBox TransformedAABB = Math::AABB( Positions, 8 );

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

		VisualizeBounds( Tree->Upper, Transform, BoundsColor );
		VisualizeBounds( Tree->Lower, Transform, BoundsColor );
	}
}

void CreateBVH( CMesh* Mesh, FTransform& Transform, const BoundingBox& WorldBounds, TriangleTree*& Tree, CBody* Body )
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
	auto Transform = GetTransform();	
	if( Static )
		return;

	CalculateBounds();
	auto NewPosition = Transform.GetPosition();

	if( !Static && !Stationary )
	{
		const auto DeltaTime = static_cast<float>( Physics->TimeStep );

		// Apply depenetration.
		NewPosition -= Depenetration;
		Normal = Depenetration * -1.0f;
		Depenetration = { 0.0f, 0.0f, 0.0f };

		Velocity += Acceleration * DeltaTime;
		Velocity += LinearVelocity;
		NewPosition += Velocity * DeltaTime;

		// Damping is used to simulate drag.
		Velocity *= powf( Damping, DeltaTime );

		// Clear the acceleration and linear velocity.
		Acceleration = { 0.0f, 0.0f, 0.0f };
		LinearVelocity = { 0.0f, 0.0f, 0.0f };

		// Penetration friction?
		// const float Factor = Math::Saturate( ( Depenetration.Normalized().Dot( Velocity.Normalized() ) * -1.0f ) + 1.0f );
		// Velocity *= powf( Factor, DeltaTime );

		// Hard limit Z to stop you from falling forever.
		constexpr float MinimumHeight = -200.0f;
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

		if( Contacts > 0 )
		{
			Contact = true;
		}
	}

	const auto Difference = NewPosition - PreviousTransform.GetPosition();
	const auto DifferenceLengthSquared = Difference.LengthSquared();
	if( DifferenceLengthSquared > 0.001f || LastActivity < 0.0 )
	{
		LastActivity = Physics->CurrentTime;
	}

	const auto TimeSinceActivity = Physics->CurrentTime - LastActivity;
	Sleeping = TimeSinceActivity > 10.0;
	if( Static || Stationary )
	{
		Sleeping = false;
	}

	Normal.Normalize();
	Transform.SetPosition( NewPosition );

	Contact = Contact || Contacts > 0;

	if( Owner )
	{
		Owner->SetTransform( Transform );
		Owner->Contact = Contact;

		PreviousTransform = Owner->GetTransform();
	}

	CalculateBounds();
	TextPosition = 0;
}

void CBody::Destroy()
{
	if( !Physics )
		return;
	
	Physics->Unregister( this );
}

void CBody::CalculateBounds()
{
	const auto Transform = GetTransform();
	WorldBounds = Math::AABB( LocalBounds, Transform );

	if( Continuous )
	{
		WorldBoundsSwept = WorldBounds;
		WorldBoundsSwept.Minimum += Velocity;
		WorldBoundsSwept.Maximum += Velocity;
		WorldBoundsSwept = WorldBoundsSwept.Combine( WorldBounds );
	}

	// Ensure we have at least some volume.
	EnsureVolume( WorldBounds );

	// Update the inverse mass once.
	if( InverseMass < 0.0f )
	{
		Mass = ( WorldBounds.Maximum - WorldBounds.Minimum ).Length() * 40.0f;
		const bool Unmoving = Static || Stationary;
		InverseMass = Unmoving ? 0.0f : 1.0f / Mass;
	}
}

const BoundingBox& CBody::GetBounds() const
{
	return WorldBounds;
}

const FTransform& CBody::GetTransform() const
{
	if( Owner )
	{
		return Owner->GetTransform();
	}

	if( Ghost )
	{
		auto* Point = ::Cast<CPointEntity>( Ghost );
		if( Point )
		{
			return Point->GetTransform();
		}
	}

	// Couldn't fetch the entity transform.
	static const FTransform Identity = FTransform();
	return Identity;
}

void CBody::SetTransform( const FTransform& NewTransform ) const
{
	if( Owner )
	{
		Owner->SetTransform( NewTransform );
	}

	if( Ghost )
	{
		auto* Point = ::Cast<CPointEntity>( Ghost );
		if( Point )
		{
			Point->SetTransform( NewTransform );
		}
	}
}

void CBody::Debug() const
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

	if( Sleeping )
	{
		BoundsColor = Color( 0, 0, 0 );
	}

	UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, BoundsColor );
	VisualizeBounds( Tree, &PreviousTransform );

	if( Static || Sleeping )
		return;

	Vector2D Offset = { 0.0f,0.0f };
	UI::AddText( WorldBounds.Minimum, "Mass", Mass, Color::White, Offset );
	Offset.Y += 20.0f;
	UI::AddText( WorldBounds.Minimum, "Restitution", Restitution, Color::White, Offset );
	Offset.Y += 20.0f;
	UI::AddText( WorldBounds.Minimum, "Damping", Damping, Color::White, Offset );
	Offset.Y += 20.0f;
	UI::AddText( WorldBounds.Minimum, "Acceleration", Acceleration, Color::White, Offset );
	Offset.Y += 20.0f;
	UI::AddText( WorldBounds.Minimum, "Velocity", Velocity, Color::White, Offset );
	Offset.Y += 20.0f;
}

BodyType CBody::GetType() const
{
	return BodyType::AABB;
}

bool CBody::ShouldIgnoreBody( CBody* Body ) const
{
	const auto* MeshEntity = ::Cast<CMeshEntity>( Body->Owner );
	if( !MeshEntity )
		return false;

	if( IgnoredBodies.empty() )
		return false;

	const auto BodyCount = IgnoredBodies.size();
	for( size_t Index = 0; Index < BodyCount; Index++ )
	{
		if( IgnoredBodies[Index] == MeshEntity )
			return true;
	}

	return false;
}

void CBody::Ignore( CBody* Body, const bool Clear )
{
	Ignore( ::Cast<CMeshEntity>( Body->Owner ), Clear );
}

void CBody::Ignore( CMeshEntity* Entity, const bool Clear )
{
	if( !Entity )
		return;

	auto DiscoveredBody = IgnoredBodies.end();
	for( auto Iterator = std::begin( IgnoredBodies ); Iterator != std::end( IgnoredBodies ); ++Iterator )
	{
		if( Entity == *Iterator )
		{
			DiscoveredBody = Iterator;
		}
	}
	
	if( Clear )
	{
		// The body was found, erase it from the list.
		if( DiscoveredBody != IgnoredBodies.end() )
		{
			IgnoredBodies.erase( DiscoveredBody );
		}
	}
	else
	{
		// The body wasn't found, add it to the list.
		if( DiscoveredBody == IgnoredBodies.end() )
		{
			IgnoredBodies.emplace_back( Entity );
		}
	}
}

void CBody::Query( const BoundingBox& Box, QueryResult& Result ) const
{
	if( Continuous )
	{
		if( !Box.Intersects( WorldBoundsSwept ) )
			return;
	}
	else
	{
		if( !Box.Intersects( GetBounds() ) )
			return;
	}
	
	Result.Hit = true;
	Result.Objects.emplace_back( const_cast<Testable*>( static_cast<const Testable*>( this ) ) );
}

Geometry::Result CBody::Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore ) const
{
	Geometry::Result Empty;
	if( !Block )
		return Empty;

	for( auto* Ignored : Ignore )
	{
		if( this == Ignored )
		{
			return Empty;
		}
	}

	Geometry::Result Result = Geometry::LineInBoundingBox( Start, End, GetBounds() );
	Result.Body = const_cast<CBody*>( this );

	// if( Result.Hit )
	// {
	// 	UI::AddCircle( ( GetBounds().Minimum + GetBounds().Maximum ) * 0.5f, 2.0f, Color::Green );
	// 	UI::AddCircle( Result.Position, 2.0f, Color::Purple );
	// 	UI::AddLine( ( GetBounds().Minimum + GetBounds().Maximum ) * 0.5f, Result.Position, Color::White );
	// }

	return Result;
}
