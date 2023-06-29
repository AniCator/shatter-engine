// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/Rendering/Mesh.h>

#include <Engine/Profiling/Profiling.h>

#include <Engine/Physics/Physics.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Physics/Body/Plane.h>

#include <Engine/Physics/Geometry.h>
#include <Engine/Physics/Response.h>

#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#include <Engine/Display/UserInterface.h>

#include <Engine/World/World.h>

#ifdef ReleaseBuild // Disable debug drawing for release builds.
#define DrawDebugLines 0
#define DrawDebugTriangleCollisions 0
#define DrawNormalAndDistance 0
#else
#define DrawDebugLines 0
#define DrawDebugTriangleCollisions 1
#define DrawNormalAndDistance 0
#endif

size_t TextPosition = 0;

struct TriangleTree
{
	TriangleTree() = default;
	~TriangleTree()
	{
		delete Upper;
		delete Lower;
		Source = nullptr;
	}

	BoundingBox Bounds;

	TriangleTree* Upper = nullptr;
	TriangleTree* Lower = nullptr;

	std::vector<glm::uint> Indices;

	// Fetches the vertex data of a given index.
	const VertexFormat& Get( const glm::uint Index ) const
	{
		return Source->GetVertexData().Vertices[Index];
	}

	CMesh* Source = nullptr;
};

void CreateTriangleTree( CMesh* Mesh, FTransform& Transform, const BoundingBox& WorldBounds, TriangleTree*& Tree, CBody* Body );

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
		// UI::AddLine( SourceVertices.Position[Index], SourceVertices.Position[Index] + ContinuousVelocity, Color::White, 0.5f );
		// UI::AddLine( SourceVertices.Position[Index], TargetVertices.Position[Index], Color::Green, 0.5f );

		Result = Geometry::LineInBoundingBox( SourceVertices.Position[Index], TargetVertices.Position[Index], B );
		if( Result.Hit )
		{
			// Update the velocity?
			ContinuousVelocity = (Result.Position - SourceVertices.Position[Index]).Normalized() * ContinuousVelocity.Length();
			// UI::AddLine( SourceVertices.Position[Index], Result.Position, Color::Red, 1.0 );
			break;
		}
	}

	if( Result.Hit )
	{
		// UI::AddAABB( ContinuousBounds.Minimum, ContinuousBounds.Maximum, Color::Blue, 1.0 );
		// UI::AddAABB( Target.Minimum, Target.Maximum, Color::Yellow, 1.0 );
		// UI::AddAABB( B.Minimum, B.Maximum, Color::Red, 1.0 );

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
		// UI::AddAABB( Target.Minimum, Target.Maximum, Color::Purple, 1.0 );

		// UI::AddLine( Result.Position, Result.Position + WorldUp, Color::Green, 1.0 );

		return true;
	}
	else
	{
		// UI::AddAABB( ContinuousBounds.Minimum, ContinuousBounds.Maximum, Color::Black );
	}

	return Intersecting;
}

CollisionResponse CollisionResponseAABBAABBSwept( const BoundingBox& A, const Vector3D& Velocity, const BoundingBox& B, const Geometry::Result& Result )
{
	CollisionResponse Response;

	if( !Result.Hit )
		return Response::AABBAABB( A, B );

	auto Target = BoundingBox(
		A.Minimum + Velocity,
		A.Maximum + Velocity
	);

	const auto Offset = Target.Minimum - A.Minimum;
	const auto Unit = Offset.Normalized();

	Response.Distance = Result.Distance - ( B.Size() * Unit ).Length();
	if( Response.Distance <= 0.0f )
		return {};

	Response.Normal = Unit;

	Target = A;
	Target.Minimum += Unit * Result.Distance;
	Target.Maximum += Unit * Result.Distance;

	UI::AddAABB( A.Minimum, A.Maximum, Color::Red, 1.0 );
	UI::AddAABB( B.Minimum, B.Maximum, Color::Blue, 1.0 );
	UI::AddAABB( Target.Minimum, Target.Maximum, Color::Purple );
	UI::AddLine( A.Center(), A.Center() + Response.Normal * Response.Distance, Color::Cyan, 1.0 );

	return Response;
}

void DrawVertex( FTransform& Transform, const VertexFormat& Vertex, const CollisionResponse& Response, const Color& Color )
{
	auto WorldSpacePosition = Transform.Transform( Vertex.Position );
	auto WorldSpaceNormal = Transform.Rotate( Response.Normal );
	UI::AddLine(
		WorldSpacePosition,
		WorldSpacePosition + WorldSpaceNormal,
		Color
	);

	Vector3D VertexNormal;
	ByteToVector( Vertex.Normal, VertexNormal );
	VertexNormal = Transform.Rotate( VertexNormal ).Normalized() * 0.1f;

	UI::AddLine( WorldSpacePosition, WorldSpacePosition - VertexNormal, Color );
}

void DrawTriangle(
	FTransform& Transform,
	const VertexFormat& A,
	const VertexFormat& B,
	const VertexFormat& C,
	const CollisionResponse& Response,
	const Color& Color,
	const float Duration = -1.0f
)
{
	const auto PosA = Transform.Transform( A.Position );
	const auto PosB = Transform.Transform( B.Position );
	const auto PosC = Transform.Transform( C.Position );

	UI::AddLine(
		PosA,
		PosB,
		Color,
		Duration
	);

	UI::AddLine(
		PosB,
		PosC,
		Color,
		Duration
	);

	UI::AddLine(
		PosC,
		PosA,
		Color,
		Duration
	);

	/*Vector3D VertexNormal;
	ByteToVector( A.Normal, VertexNormal );
	VertexNormal = Transform.Rotate( VertexNormal ).Normalized() * 0.1f;

	const auto Center = ( PosA + PosB + PosC ) / 3.0f;
	UI::AddLine( Center, Center + VertexNormal, Color::Purple );

	const auto WorldSpaceNormal = Transform.Rotate( Response.Normal );
	UI::AddLine( Center, Center + WorldSpaceNormal, Color::Yellow );*/
}

void AddText( const Vector3D& A, const Vector3D& B, const Vector3D& C, const Color& Color )
{
	UI::AddText( A, "O", nullptr, Color );
	UI::AddText( B, "O", nullptr, Color );
	UI::AddText( C, "O", nullptr, Color );
}

bool PointInTriangle( 
	const Vector3D& TriangleCornerA, 
	const Vector3D& TriangleCornerB, 
	const Vector3D& TriangleCornerC,
	const Vector3D& Point
)
{
	// Calculate the triangle corners relative to the given point.
	const Vector3D A = TriangleCornerA - Point;
	const Vector3D B = TriangleCornerB - Point;
	const Vector3D C = TriangleCornerC - Point;

	// Calculate the triangle edge normals.
	const Vector3D U = B.Cross( C );
	const Vector3D V = C.Cross( A );
	const Vector3D W = A.Cross( B );

	UI::AddCircle( { 0.0f, 0.0f, 0.0f }, 2.0f, Color::White );

	// Pyramid visualization
	UI::AddLine( Vector3D::Zero, A, Color::Purple );
	UI::AddLine( Vector3D::Zero, B, Color::Purple );
	UI::AddLine( Vector3D::Zero, C, Color::Purple );

	const Vector3D BC = ( B + C ) * 0.5f;
	const Vector3D CA = ( C + A ) * 0.5f;
	const Vector3D AB = ( A + B ) * 0.5f;
	UI::AddLine( BC, BC + U, Color::White );
	UI::AddLine( CA, CA + V, Color::White );
	UI::AddLine( AB, AB + W, Color::White );

	if( U.Dot( V ) < 0.0f )
	{
		UI::AddLine( B, C, Color::Red );
		UI::AddLine( C, A, Color::Red );
		UI::AddLine( A, B, Color::Red );
		return false;
	}
	else if( U.Dot( W ) < 0.0f )
	{
		UI::AddLine( B, C, Color::Yellow );
		UI::AddLine( C, A, Color::Yellow );
		UI::AddLine( A, B, Color::Yellow );
		return false;
	}

	UI::AddLine( B, C, Color::Green );
	UI::AddLine( C, A, Color::Green );
	UI::AddLine( A, B, Color::Green );
	return true;
}

CollisionResponse CollisionResponseTreeAABB( TriangleTree* Tree, const BoundingBox& WorldBounds, FTransform& Transform, FTransform& OtherTransform );

void ProcessTreeLeaf( 
	TriangleTree* Leaf, 
	const BoundingBox& WorldBounds, 
	FTransform& Transform, 
	FTransform& OtherTransform, 
	CollisionResponse& Response 
)
{
	if( !Leaf )
		return;

	CollisionResponse LeafResponse = CollisionResponseTreeAABB( Leaf, WorldBounds, Transform, OtherTransform );
	if( LeafResponse.Distance < Response.Distance )
	{
		Response.Normal = LeafResponse.Normal;
		Response.Distance = LeafResponse.Distance;
	}

	// Old statement.
	// if( Math::Abs( ResponseA.Distance ) < Math::Abs( Response.Distance ) || Response.Distance < 0.0001f )
}

void ProcessLeafTriangles(
	TriangleTree* Leaf,
	FTransform& Transform,
	const Vector3D& Center,
	const Vector3D& Extent,
	CollisionResponse& Response
)
{
	if( !Leaf )
		return;

#if DrawDebugTriangleCollisions == 1
	VisualizeBounds( Leaf, &Transform, Color::Red );
#endif

	for( unsigned int Index = 0; Index < Leaf->Indices.size(); )
	{
		const VertexFormat& VertexA = Leaf->Get( Leaf->Indices[Index] );
		const VertexFormat& VertexB = Leaf->Get( Leaf->Indices[Index + 1] );
		const VertexFormat& VertexC = Leaf->Get( Leaf->Indices[Index + 2] );

		CollisionResponse TriangleResponse = Response::TriangleAABB( VertexA, VertexB, VertexC, Center, Extent );
		// if( Math::Abs( TriangleResponse.Distance ) < Math::Abs( Response.Distance ) || Response.Distance < 0.0001f )
		if( TriangleResponse.Distance < Response.Distance )
		{
			Response.Normal = TriangleResponse.Normal;
			Response.Distance = TriangleResponse.Distance;

#if DrawDebugTriangleCollisions == 1
			// DrawVertex( Transform, VertexA, TriangleResponse, Color( 255, 0, 0 ) );
			// DrawVertex( Transform, VertexB, TriangleResponse, Color( 0, 255, 0 ) );
			// DrawVertex( Transform, VertexC, TriangleResponse, Color( 0, 0, 255 ) );
			DrawTriangle( Transform, VertexA, VertexB, VertexC, TriangleResponse, Color::White );
#endif
		}
#if DrawDebugTriangleCollisions == 1
		else
		{
			DrawVertex( Transform, VertexA, TriangleResponse, Color( 127, 0, 0 ) );
			DrawVertex( Transform, VertexB, TriangleResponse, Color( 0, 127, 0 ) );
			DrawVertex( Transform, VertexC, TriangleResponse, Color( 0, 0, 127 ) );
		}
#endif

		Index += 3;
	}
}

CollisionResponse CollisionResponseTreeAABB( TriangleTree* Tree, const BoundingBox& WorldBounds, FTransform& Transform, FTransform& OtherTransform )
{
	CollisionResponse Response;

	if( !Tree )
		return Response;

	BoundingBox LocalBounds = Math::AABB( WorldBounds, Transform.GetTransformationMatrixInverse() );

	const Vector3D Center = LocalBounds.Center();
	const Vector3D Extent = LocalBounds.Size();

	/*FBounds TreeRelativeBounds = Math::AABB( LocalBounds, Transform );
	UI::AddAABB( TreeRelativeBounds.Minimum, TreeRelativeBounds.Maximum );

	FBounds TreeBounds = Math::AABB( Tree->Bounds, Transform );
	UI::AddAABB( TreeBounds.Minimum, TreeBounds.Maximum, Color( 255, 255, 0 ) );*/

	if( !Math::BoundingBoxIntersection( LocalBounds.Minimum, LocalBounds.Maximum, Tree->Bounds.Minimum, Tree->Bounds.Maximum ) )
		return Response;
	
	// Prime the response's starting distance.
	Response.Distance = INFINITY;

	ProcessTreeLeaf( Tree->Upper, WorldBounds, Transform, OtherTransform, Response );
	ProcessTreeLeaf( Tree->Lower, WorldBounds, Transform, OtherTransform, Response );

	if( !Tree->Upper || !Tree->Lower )
	{
		ProcessLeafTriangles( Tree, Transform, Center, Extent, Response );
	}

	Response.Normal = Transform.GetRotationMatrix().Transform( Response.Normal ) * -1.0f;
	const float Size = Response.Normal.Normalize();
	Response.Distance *= Size * -1.0f; // Scale the penetration distance.

	// const Vector3D WorldCenter = Transform.Transform( Center );
	// UI::AddLine( WorldCenter, WorldCenter - Response.Normal, Color::Yellow, 1.0f );

	return Response;
}

CBody::~CBody()
{
	delete Tree;
}

void CBody::Construct()
{
	Contacts.reserve( 8 );

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
	Contacts.reserve( 8 );

	if( !Physics )
		return;

	this->Physics = Physics;

	// Calculate the initial bounds before registering.
	CalculateBounds();
	
	PreviousTransform = GetTransform();

	auto Transform = GetTransform();
	if( Owner && ( Owner->IsStatic() || Owner->IsStationary() ) && Type == BodyType::TriangleMesh )
	{
		auto* Mesh = Owner->CollisionMesh ? Owner->CollisionMesh : Owner->Mesh;

		// TODO: Fix triangle tree and triangle collisions.
		CreateTriangleTree( Mesh, Transform, LocalBounds, Tree, this );
	}

	Physics->Register( this );
}

void ApplyLinearVelocity( CBody* Body )
{
	const auto NoLinearVelocity = Math::Equal( Body->LinearVelocity, Vector3D::Zero );
	if( NoLinearVelocity )
		return;

	// Apply linear velocity before testing collisions.
	auto Transform = Body->GetTransform();
	auto Position = Transform.GetPosition();
	auto NewPosition = Position;
	NewPosition += Body->LinearVelocity;
	Transform.SetPosition( NewPosition );
	Body->SetTransform( Transform );

	// Reset.
	Body->LinearVelocity = { 0.0f, 0.0f, 0.0f };

	// Make sure we recalculate the bounds because the position has been changed.
	Body->CalculateBounds();

	// We have applied linear velocity so we've got to activate.
	Body->LastActivity = Body->Physics->CurrentTime;
}

void CBody::PreCollision()
{
	// Don't reset the values if we're sleeping.
	if( Sleeping )
		return;

	Normal = Vector3D::Zero;
	Contacts.clear();

	if( Static )
		return;

	ApplyLinearVelocity( this );
}

void GenerateGravity( CBody* Body )
{
	if( !Body->AffectedByGravity )
		return; // Body is not affected by gravity.

	// Ensure the gravity force affects all objects equally by multiplying it with the mass of an object.
	Body->Acceleration += Body->Gravity * Body->Mass;
}

constexpr float DragCoefficientSphereSmooth = 0.1f;
constexpr float DragCoefficientSphere = 0.47f;
constexpr float DragCoefficient = DragCoefficientSphere * 0.01f * 0.01f;
constexpr float DragCoefficientSquared = DragCoefficient * DragCoefficient;
void GenerateDrag( CBody* Body )
{
	float Drag = Body->Velocity.Length();
	Vector3D Direction = Body->Velocity;
	if( Drag != 0.0f )
	{
		Direction /= Drag;
	}

	Drag = DragCoefficient * Drag + DragCoefficientSquared * ( Drag * Drag );
	Body->Acceleration -= Direction * Drag;
}

void CBody::Simulate()
{
	// We don't need to simulate environmental factors for bodies that don't move.
	if( !IsKinetic() )
		return;

	// Calculate and apply gravity.
	GenerateGravity( this );

	// Apply drag force.
	GenerateDrag( this );
}

CollisionResponse CalculateResponseSphere( CBody* A, CBody* B, const Geometry::Result& SweptResult )
{
	auto InnerSphereA = A->InnerSphere();

	switch( B->Type )
	{
	case BodyType::TriangleMesh:
		break; // TODO: Sphere v. Triangle Mesh.
	case BodyType::Plane:
		return Response::SpherePlane( InnerSphereA, { B->PreviousTransform.GetPosition(), B->PreviousTransform.Rotate( WorldUp )});
	case BodyType::AABB:
		return Response::SphereAABB( InnerSphereA, B->WorldBounds );
	case BodyType::Sphere:
	{
		auto InnerSphereB = B->InnerSphere();
		return Response::SphereSphere( InnerSphereA, InnerSphereB );
	}
	default:
		break;
	}

	// No response generated.
	return {};
}

CollisionResponse CalculateResponsePlane( CBody* A, CBody* B, const Geometry::Result& SweptResult )
{
	switch( B->Type )
	{
	case BodyType::TriangleMesh:
		break; // TODO: Plane v. Triangle Mesh.
	case BodyType::Plane:
		break; // TODO: Plane v. Plane.
	case BodyType::AABB:
	{
		// TODO: Plane v. AABB. (interpret as sphere for now)
		return Response::SpherePlane( B->WorldSphere, { A->PreviousTransform.GetPosition(), A->PreviousTransform.Rotate( WorldUp ) } );
	}
	case BodyType::Sphere:
	{
		auto InnerSphereB = B->InnerSphere();
		return Response::SpherePlane( InnerSphereB, { A->PreviousTransform.GetPosition(), A->PreviousTransform.Rotate( WorldUp ) } );
	}
	default:
		break;
	}

	// No response generated.
	return {};
}

CollisionResponse CalculateResponseAABB( CBody* A, CBody* B, const Geometry::Result& SweptResult )
{
	switch( B->Type )
	{
	case BodyType::TriangleMesh:
		if( A->Continuous )
		{
			return CollisionResponseAABBAABBSwept( A->WorldBounds, A->Velocity, B->WorldBounds, SweptResult );
		}
		else
		{
			return CollisionResponseTreeAABB( B->Tree, A->WorldBounds, B->PreviousTransform, A->PreviousTransform );
			// return Response::AABBAABB( A->WorldBounds, B->WorldBounds );
		}
	case BodyType::Plane:
	{
		// TODO: AABB v. Plane. (using sphere v. AABB here temporarily)
		return Response::SpherePlane( A->WorldSphere, { B->PreviousTransform.GetPosition(), B->PreviousTransform.Rotate( WorldUp ) } );
	}
	case BodyType::AABB:
		if( A->Continuous )
		{
			return CollisionResponseAABBAABBSwept( A->WorldBounds, A->Velocity, B->WorldBounds, SweptResult );
		}
		else
		{
			return Response::AABBAABB( A->WorldBounds, B->WorldBounds );
		}
	case BodyType::Sphere:
	{
		auto InnerSphereB = B->InnerSphere();
		return Response::SphereAABB( InnerSphereB, A->WorldBounds );
	}
	default:
		break;
	}

	// No response generated.
	return {};
}

CollisionResponse CalculateResponseTriangleMesh( CBody* A, CBody* B, const Geometry::Result& SweptResult )
{
	switch( B->Type )
	{
	case BodyType::TriangleMesh:
		break; // TODO: Triangle Mesh v. Triangle Mesh.
	case BodyType::Plane:
		break; // TODO: Triangle Mesh v. Plane.
	case BodyType::AABB:
		return CollisionResponseTreeAABB( A->Tree, B->WorldBounds, A->PreviousTransform, B->PreviousTransform );
	case BodyType::Sphere:
		break; // TODO: Triangle Mesh v. Sphere.
	default:
		break;
	}

	// No response generated.
	return {};
}

CollisionResponse CalculateResponse( CBody* A, CBody* B, const Geometry::Result& SweptResult )
{
	if( !A->IsKinetic() )
		return {}; // Only generate responses for kinetic objects.

	switch( A->Type )
	{
	case BodyType::TriangleMesh:
		return CalculateResponseTriangleMesh( A, B, SweptResult );
	case BodyType::Plane:
		return CalculateResponsePlane( A, B, SweptResult );
	case BodyType::AABB:
		return CalculateResponseAABB( A, B, SweptResult );
	case BodyType::Sphere:
		return CalculateResponseSphere( A, B, SweptResult );
	default:
		break;
	}

	// No response generated.
	return {};
}

void Friction( CBody* A, CBody* B, const CollisionResponse& Response, const Vector3D& RelativeVelocity, const float& InverseMassTotal, const float& ImpulseScale )
{
	auto Tangent = RelativeVelocity - ( Response.Normal * RelativeVelocity.Dot( Response.Normal ) );
	if( Math::Equal( Tangent.LengthSquared(), 0.0f, 0.0001f ) )
		return; // Invalid tangent vector.

	Tangent.Normalize();

	// UI::AddLine( A->WorldSphere.Origin(), A->WorldSphere.Origin() + Tangent, Color::Green, 0.1f );

	auto FrictionScale = RelativeVelocity.Dot( Tangent * -1.0f ) / InverseMassTotal;
	if( Math::Equal( FrictionScale, 0.0f ) )
		return; // Scale shouldn't be zero.

	const auto Friction = sqrtf( A->Friction * B->Friction );
	if( FrictionScale > ImpulseScale * Friction )
	{
		FrictionScale = ImpulseScale * Friction;
	}
	else if( FrictionScale < -ImpulseScale * Friction )
	{
		FrictionScale = -ImpulseScale * Friction;
	}

	const auto TangentImpulse = Tangent * FrictionScale;
	A->Velocity -= A->InverseMass * TangentImpulse;
	B->Velocity += B->InverseMass * TangentImpulse;
}

void Detect( CBody* A, CBody* B, const Geometry::Result& SweptResult )
{
	const float InverseMassTotal = A->InverseMass + B->InverseMass;
	if( InverseMassTotal <= 0.0f )
		return; // Both objects are of infinite mass and thus considered immovable.

	ContactManifold Manifold;
	Manifold.Response = CalculateResponse( A, B, SweptResult );
	Manifold.Other = B;

	// If the normal is zero, that means we didn't collide.
	if( Math::Equal( Manifold.Response.Normal, Vector3D::Zero ) )
		return;

	A->Contacts.emplace_back( Manifold );
}

void Interpenetration( CBody* A, const ContactManifold& Manifold )
{
	if( A->Surface == PhysicalSurface::Water || Manifold.Other->Surface == PhysicalSurface::Water )
		return; // Ignore water penetration.

	// Are the bodies penetrating?
	if( Manifold.Response.Distance <= 0.0f )
		return;
	
	// Resolve penetration.
	const float InverseMassTotal = A->InverseMass + Manifold.Other->InverseMass;
	const auto Offset = 0.5f * Manifold.Response.Normal * Manifold.Response.Distance / InverseMassTotal;
	A->Depenetration += Offset * A->InverseMass;
	Manifold.Other->Depenetration -= Offset * Manifold.Other->InverseMass;
}

void Integrate( CBody* A, const ContactManifold& Manifold )
{
	const float InverseMassTotal = A->InverseMass + Manifold.Other->InverseMass;
	const Vector3D RelativeVelocity = Manifold.Other->Velocity - A->Velocity;

	float SeparatingVelocity = RelativeVelocity.Dot( Manifold.Response.Normal );
	if( SeparatingVelocity >= 0.0f )
		return; // The bodies are moving away from each other.

	const float Restitution = 1.0f + Math::Min( A->Restitution, Manifold.Other->Restitution );
	float ResponseForce = -SeparatingVelocity * Restitution;

	// Account for acceleration.
	/*const Vector3D RelativeAcceleration = A->Acceleration - Manifold.Other->Acceleration;
	const float SeparatingAcceleration = RelativeAcceleration.Dot( Manifold.Response.Normal );
	if( SeparatingAcceleration < 0.0f )
	{
		ResponseForce += SeparatingAcceleration * Restitution;

		if( ResponseForce < 0.0f )
		{
			ResponseForce = 0.0f;
		}
	}*/

	// ResponseForce = ResponseForce + SeparatingVelocity;

	Manifold.Other->ContactEntity = A->Owner;
	A->ContactEntity = Manifold.Other->Owner;

	if( A->Surface == PhysicalSurface::Water )
	{
		// Manifold.Other->Acceleration -= Manifold.Other->InverseMass * Manifold.Other->Gravity * 0.314;
		return; // TODO: Buoyancy.
	}

	if( Manifold.Other->Surface == PhysicalSurface::Water )
	{
		// A->Acceleration -= A->InverseMass * A->Gravity * 0.314;
		return; // TODO: Buoyancy.
	}
	
	const float ImpulseScale = ResponseForce / InverseMassTotal;
	const Vector3D Impulse = Manifold.Response.Normal * ImpulseScale;

	A->Velocity -= A->InverseMass * Impulse;
	Manifold.Other->Velocity += Manifold.Other->InverseMass * Impulse;

	Friction( A, Manifold.Other, Manifold.Response, RelativeVelocity, InverseMassTotal, ImpulseScale );
}

constexpr size_t DebugTableSize = 7;
const Color DebugTable[DebugTableSize] = {
	Color( 0, 0, 255 ),
	Color( 0, 64, 127 ),
	Color( 0, 127, 64 ),
	Color( 0, 255, 0 ),
	Color( 64, 127, 0 ),
	Color( 127, 64, 0 ),
	Color( 255, 0, 0 ),
};

void Resolve( CBody* A )
{
	if( A->Contacts.empty() )
		return; // No contacts to resolve.

	// Resolve penetration first.
	size_t Debug = 0;
	for( const auto& Manifold : A->Contacts )
	{
		/*if( Manifold.Other )
		{
			UI::AddAABB( Manifold.Other->WorldBounds.Minimum, Manifold.Other->WorldBounds.Maximum, DebugTable[Debug++] );
			Debug = Debug % DebugTableSize;
		}*/

		Interpenetration( A, Manifold );
	}

	constexpr size_t Iterations = 8;
	size_t Iteration = 0;
	while( Iteration < Iterations )
	{
		float SeparationThreshold = 0.0f;
		size_t ManifoldIndex = 0;
		for( size_t Index = 0; Index < A->Contacts.size(); Index++ )
		{
			const ContactManifold& Manifold = A->Contacts[Index];
			const Vector3D RelativeVelocity = Manifold.Other->Velocity - A->Velocity;

			float SeparatingVelocity = RelativeVelocity.Dot( Manifold.Response.Normal );
			if( SeparatingVelocity >= SeparationThreshold )
				continue;

			SeparationThreshold = SeparatingVelocity;
			ManifoldIndex = Index;
		}

		Integrate( A, A->Contacts[ManifoldIndex] );
		Iteration++;
	}
}

void CBody::Collision( CBody* Body )
{
	if( !Block || !Body->Block )
		return;

	// Check the outer sphere bounds.
	if( !Continuous && !WorldSphere.Intersects( Body->WorldSphere ) )
		return;

	if( ShouldIgnoreBody( Body ) )
		return;

	// CCD
	Geometry::Result SweptResult;

	// Only perform the bounding box check for non-spherical objects.
	if( Type != BodyType::Sphere )
	{
		if( Continuous )
		{
			if( !SweptIntersection( WorldBounds, Velocity + LinearVelocity, Body->WorldBounds, SweptResult ) )
				return;
		}
		else
		{
			if( !WorldBoundsSIMD.Intersects( Body->WorldBoundsSIMD ) )
				return;
		}
	}

	Detect( this, Body, SweptResult );
}

std::vector<glm::uint> GatherVertices( const TriangleTree* Tree, std::vector<glm::uint>& Indices, const Vector3D& Median, const bool Direction, const size_t Axis )
{
	std::vector<glm::uint> GatheredIndices;
	GatheredIndices.reserve( Indices.size() );

	for( unsigned int Index = 0; Index < Indices.size(); )
	{
		bool InsideBounds = false;

		Vector3D TriangleCenter = Tree->Get( Indices[Index] ).Position + Tree->Get( Indices[Index + 1] ).Position + Tree->Get( Indices[Index + 2] ).Position;
		TriangleCenter /= 3.0f;

		for( unsigned int TriangleIndex = 0; TriangleIndex < 3; TriangleIndex++ )
		{
			const VertexFormat& Vertex = Tree->Get( Indices[Index + TriangleIndex] );
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
			GatheredIndices.emplace_back( Index );
			GatheredIndices.emplace_back( Index + 1 );
			GatheredIndices.emplace_back( Index + 2 );
		}

		Index += 3;
	}

	return GatheredIndices;
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

	std::vector<glm::uint>& Indices = Tree->Indices;

	Vector3D LocalMedian = Vector3D( 0.0f, 0.0f, 0.0f );
	for( size_t Index = 0; Index < Indices.size(); Index++ )
	{
		LocalMedian += Tree->Get( Indices[Index] ).Position;
	}

	LocalMedian /= static_cast<float>( Indices.size() );
	Vector3D Median = LocalMedian; // Transform.Transform( LocalMedian );

	Median = ( WorldBounds.Minimum + WorldBounds.Maximum ) * 0.5f;

	BoundingBox UpperBounds = WorldBounds;
	BoundingBox LowerBounds = UpperBounds;

	size_t Axis = 0;
	Vector3D Size = WorldBounds.Size();

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

	if( Depth > 0 && Indices.size() > 25 )
	{
		if( !Tree->Upper )
		{
			Tree->Upper = new TriangleTree();
		}

		if( !Tree->Lower )
		{
			Tree->Lower = new TriangleTree();
		}

		Tree->Upper->Indices = GatherVertices( Tree, Indices, Median, true, Axis );
		if( Tree->Upper->Indices.size() == 0 )
		{
			delete Tree->Upper;
			Tree->Upper = nullptr;
		}
		else
		{
			BuildMedian( Tree->Upper, Transform, UpperBounds, Depth - 1 );
		}

		Tree->Lower->Indices = GatherVertices( Tree, Indices, Median, false, Axis );
		if( Tree->Lower->Indices.size() == 0 )
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
			if( !Tree->Indices.empty() )
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
				

				//for( size_t Index = 0; Index < Tree->Vertices.size(); )
				//{
				//	VertexFormat VertexA = Tree->Vertices[Index];
				//	VertexFormat VertexB = Tree->Vertices[Index + 1];
				//	VertexFormat VertexC = Tree->Vertices[Index + 2];

				//	if( Transform )
				//	{
				//		VertexA.Position = Transform->Transform( VertexA.Position );
				//	}

				//	// UI::AddText( VertexA, std::to_string( Index ).c_str() );

				//	Index += 3;
				//}
			}
		}

		VisualizeBounds( Tree->Upper, Transform, BoundsColor );
		VisualizeBounds( Tree->Lower, Transform, BoundsColor );
	}
}

void CreateTriangleTree( CMesh* Mesh, FTransform& Transform, const BoundingBox& WorldBounds, TriangleTree*& Tree, CBody* Body )
{
	if( Tree )
		return;
	
	ProfileMemory( "Physics Triangle Trees" );

	Tree = new TriangleTree();
	Tree->Source = Mesh;

	const auto& IndexData = Mesh->GetIndexData();
	const auto& VertexBufferData = Mesh->GetVertexBufferData();

	std::vector<glm::uint>& Indices = Tree->Indices;
	Indices.reserve( VertexBufferData.IndexCount );

	for( unsigned int Index = 0; Index < VertexBufferData.IndexCount; )
	{
		Indices.emplace_back( IndexData.Indices[Index] );
		Indices.emplace_back( IndexData.Indices[Index + 1] );
		Indices.emplace_back( IndexData.Indices[Index + 2] );

		Index += 3;
	}

	// TODO: Fix transformation issues with a depth greater than 0.
	BuildMedian( Tree, Transform, Mesh->GetBounds(), 0 );
}

// Constraint that stops a body from moving below the specified value, on the Z-axis.
void ConstrainZ( CBody* Body, Vector3D& Position, float Height )
{
	if( Position.Z >= Height )
		return;
	
	if( Body->Velocity.Z < 0.0f )
	{
		// Reset the velocity to help stabilize the body.
		Body->Velocity.Z = 0.0f;
	}

	Position.Z = Height;
	Body->Normal = Vector3D( 0.0f, 0.0f, 1.0f );
	Body->Contact = true;

	// Fake friction through damping.
	Body->Velocity.X *= Body->Damping;
	Body->Velocity.Y *= Body->Damping;
}

// Constraint that stops a body from moving below the specified value, on the Z-axis.
void ConstrainBox( CBody* Body, Vector3D& Position, Vector3D BoxOrigin, Vector3D BoxExtent )
{
	BoxExtent = Math::Abs( BoxExtent );
	Vector3D Maximum = BoxOrigin + BoxExtent;
	Vector3D Minimum = BoxOrigin - BoxExtent;

	constexpr float BoxRestitution = 0.25f;
	float Force = BoxRestitution;
	float Radius = Body->InnerSphere().GetRadius();

	// Account for the radius of the collision sphere.
	Maximum = Maximum - Radius;
	Minimum = Minimum + Radius;

	bool Contact = false;
	if( Position.X > Maximum.X )
	{
		Position.X = Maximum.X;
		Body->Velocity.X = -Force * Body->Velocity.X;
		Contact = true;

		// Damping
		Body->Velocity.Y *= Body->Damping;
		Body->Velocity.Z *= Body->Damping;
	}

	if( Position.Y > Maximum.Y )
	{
		Position.Y = Maximum.Y;
		Body->Velocity.Y = -Force * Body->Velocity.Y;
		Contact = true;

		// Damping
		Body->Velocity.X *= Body->Damping;
		Body->Velocity.Z *= Body->Damping;
	}

	if( Position.Z > Maximum.Z )
	{
		Position.Z = Maximum.Z;
		Body->Velocity.Z = -Force * Body->Velocity.Z;
		Contact = true;

		// Damping
		Body->Velocity.X *= Body->Damping;
		Body->Velocity.Y *= Body->Damping;
	}

	if( Position.X < Minimum.X )
	{
		Position.X = Minimum.X;
		Body->Velocity.X = -Force * Body->Velocity.X;
		Contact = true;

		// Damping
		Body->Velocity.Y *= Body->Damping;
		Body->Velocity.Z *= Body->Damping;
	}

	if( Position.Y < Minimum.Y )
	{
		Position.Y = Minimum.Y;
		Body->Velocity.Y = -Force * Body->Velocity.Y;
		Contact = true;

		// Damping
		Body->Velocity.X *= Body->Damping;
		Body->Velocity.Z *= Body->Damping;
	}

	if( Position.Z < Minimum.Z )
	{
		Position.Z = Minimum.Z;
		Body->Velocity.Z = -Force * Body->Velocity.Z;
		Contact = true;

		// Damping
		Body->Velocity.X *= Body->Damping;
		Body->Velocity.Y *= Body->Damping;
	}

	if( Contact )
	{
		Body->Contact = true;
	}
}

// Constraint that stops a body from moving below the specified value, on the Z-axis.
void ConstrainSphere( CBody* Body, Vector3D& Position, const BoundingSphere& Sphere )
{
	Vector3D SphereNormal = Position - Sphere.Origin();
	const float Distance = SphereNormal.Length();
	const float Radius = Body->WorldSphere.GetRadius();
	const float AdjustedRadius = Sphere.GetRadius() - Radius;
	if( Distance < AdjustedRadius )
		return; // We're inside the sphere.

	if( Distance == 0.0f )
		return; // Invalid distance.

	SphereNormal /= Distance; // Normalize.
	
	const float Direction = Body->Velocity.Dot( SphereNormal );
	if( Direction < 0.0f )
		return; // Moving away.

	Body->Normal = SphereNormal * -1.0f;
	Body->Velocity -= SphereNormal * Direction;
	Body->Contact = true;
	// Position = Sphere.Origin() + SphereNormal * ( AdjustedRadius - Radius );
}

void CBody::Tick()
{
	auto Transform = GetTransform();	
	if( Static )
		return;

	CalculateBounds();
	auto NewPosition = Transform.GetPosition();

	bool TriedToMove = false;

	// Update contact boolean.
	Contact = Contacts.size() > 0;

	if( IsKinetic() )
	{
		// Resolve contact manifolds.
		Resolve( this );

		const auto DeltaTime = static_cast<float>( Physics->TimeStep );

		// Apply depenetration.
		NewPosition -= Depenetration;
		Normal = Depenetration * -1.0f;
		Depenetration = { 0.0f, 0.0f, 0.0f };

		// Semi-implicit Euler integration.
		Velocity += InverseMass * Acceleration * DeltaTime;
		NewPosition += Velocity * DeltaTime;

		// Subtract the linear velocity that was applied during this tick.
		// Velocity -= LinearVelocity;

		// Damping is used to simulate drag.
		// Velocity *= powf( Damping, DeltaTime );

		TriedToMove = !Math::Equal( Transform.GetPosition(), NewPosition, 0.001f );

		// Clear the acceleration and linear velocity.
		Acceleration = { 0.0f, 0.0f, 0.0f };
		LinearVelocity = { 0.0f, 0.0f, 0.0f };

		// Hard limit Z to stop you from falling forever.
		constexpr float MinimumHeight = -200.0f;
		if( Type == BodyType::Sphere )
		{
			// ConstrainBox( this, NewPosition, Vector3D::Zero, MinimumHeight * -1.0f );
			// BoundingSphere Border( Vector3D::Zero, MinimumHeight * -1.0f );
			// ConstrainSphere( this, NewPosition, Border );
		}
		else
		{
			ConstrainZ( this, NewPosition, MinimumHeight );
		}
	}

	const auto Difference = NewPosition - PreviousTransform.GetPosition();
	const auto DifferenceLengthSquared = Difference.LengthSquared();
	if( LastActivity < 0.0 || TriedToMove )
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
		const auto DeltaTime = static_cast<float>( Physics->TimeStep );

		WorldBoundsSwept = WorldBounds;
		WorldBoundsSwept.Minimum += Velocity * DeltaTime;
		WorldBoundsSwept.Maximum += Velocity * DeltaTime;
		WorldBoundsSwept = WorldBoundsSwept.Combine( WorldBounds );

		// Use larger bounds for the SIMD query.
		auto InflatedBounds = WorldBoundsSwept;
		// InflatedBounds.Minimum -= {0.01f, 0.01f, 0.01f};
		// InflatedBounds.Maximum += {0.01f, 0.01f, 0.01f};

		WorldBoundsSweptSIMD = InflatedBounds;
	}

	// Ensure we have at least some volume.
	EnsureVolume( WorldBounds );

	// Use larger bounds for the SIMD query.
	auto InflatedBounds = WorldBounds;
	// InflatedBounds.Minimum -= {0.01f, 0.01f, 0.01f};
	// InflatedBounds.Maximum += {0.01f, 0.01f, 0.01f};

	WorldBoundsSIMD = InflatedBounds;
	WorldSphere = WorldBounds;

	// Update the inverse mass once.
	if( InverseMass < 0.0f )
	{
		constexpr float DefaultDensity = 40.0f;
		const float Diameter = InnerSphere().GetRadius() * 2.0f;
		Mass = Diameter * DefaultDensity;
		const bool Unmoving = Static || Stationary || Mass == 0.0f;
		InverseMass = Unmoving ? 0.0f : 1.0f / Mass;
	}
}

BoundingBoxSIMD CBody::GetBounds() const
{
	if( Continuous )
	{
		return WorldBoundsSwept;
	}

	return WorldBoundsSIMD;
}

BoundingBox CBody::GetWorldBounds() const
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

ConfigurationVariable<bool> DisplayBodyInfo( "debug.Physics.DisplayBodyInfo", false );
ConfigurationVariable<bool> DisplayTriangleTree( "debug.Physics.DisplayTriangleTree", false );
ConfigurationVariable<bool> DisplaySphereBounds( "debug.Physics.DisplaySphereBounds", true );
void CBody::Debug() const
{
	if( !Owner && !Ghost )
		return;

	Color BoundsColor = Color( 0, 255, 0 );
	const size_t Count = Contacts.size();
	if( Count < 1 )
	{
		BoundsColor = Color::Red;
	}
	else if( Count == 1 )
	{
		BoundsColor = Color( 255, 127, 0 );
	}
	else if( Count == 3 )
	{
		BoundsColor = Color( 255, 255, 0 );
	}
	else if( Count == 4 )
	{
		BoundsColor = Color( 127, 255, 0 );
	}

	if( Sleeping )
	{
		BoundsColor = Color( 0, 0, 0 );
	}

	UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, BoundsColor );
	// PointInTriangle( { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { sinf( Physics->CurrentTime ) * 2.0f, 0.5f, 0.0f } );

	if( DisplaySphereBounds )
		UI::AddSphere( WorldSphere.Origin(), WorldSphere.GetRadius(), BoundsColor );

	if( DisplayTriangleTree )
		VisualizeBounds( Tree, &PreviousTransform );

	if( Static || Sleeping || !DisplayBodyInfo )
		return;

	Vector2D Offset = { 0.0f,0.0f };
	UI::AddText( WorldBounds.Minimum, "Mass", Mass, Color::White, Offset );
	Offset.Y += 20.0f;
	UI::AddText( WorldBounds.Minimum, "Restitution", Restitution, Color::White, Offset );
	Offset.Y += 20.0f;
	UI::AddText( WorldBounds.Minimum, "Friction", Friction, Color::White, Offset );
	Offset.Y += 20.0f;
	UI::AddText( WorldBounds.Minimum, "Damping", Damping, Color::White, Offset );
	Offset.Y += 20.0f;
	UI::AddText( WorldBounds.Minimum, "Acceleration", Acceleration, Color::White, Offset );
	Offset.Y += 20.0f;
	UI::AddText( WorldBounds.Minimum, "Velocity", Velocity, Color::White, Offset );
	Offset.Y += 20.0f;
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

void CBody::Query( const BoundingBoxSIMD& Box, QueryResult& Result )
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
	Result.Objects.push_back( this );
}

Geometry::Result CBody::Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore ) const
{
	Geometry::Result Empty;
	if( !Block || Type == BodyType::TriangleMesh ) // TODO: Triangle mesh support
		return Empty;

	for( auto* Ignored : Ignore )
	{
		if( this == Ignored )
		{
			return Empty;
		}
	}

	Geometry::Result Result = Geometry::LineInBoundingBox( Start, End, WorldBounds );
	Result.Body = const_cast<CBody*>( this );

	// if( Result.Hit )
	// {
	// 	UI::AddCircle( ( GetBounds().Minimum + GetBounds().Maximum ) * 0.5f, 2.0f, Color::Green );
	// 	UI::AddCircle( Result.Position, 2.0f, Color::Purple );
	// 	UI::AddLine( ( GetBounds().Minimum + GetBounds().Maximum ) * 0.5f, Result.Position, Color::White );
	// }

	return Result;
}

BoundingSphere CBody::InnerSphere() const
{
	// Find the shortest axis.
	auto SmallestFit = Math::Min( WorldBounds.Size() );
	return BoundingSphere( WorldBounds.Center(), SmallestFit * 0.5f );
}
