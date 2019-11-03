// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PhysicsComponent.h"

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Physics/Physics.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#include <Engine/Display/UserInterface.h>

#include <Game/Game.h>
#include <Engine/World/World.h>

static const auto Gravity = Vector3D( 0.0f, 0.0f, 9.81f );

#define DrawDebugLines 1

#if DrawDebugLines == 1
size_t TextPosition = 0;
#endif

void VisualizeBounds( TriangleTree* Tree, FTransform* Transform = nullptr );

float sign( const float& Input )
{
	return static_cast<float>( ( 0.0f < Input ) - ( Input < 0.0f ) );
}

struct CollisionResponse
{
	CollisionResponse()
	{
		Point = Vector3D( 0.0f, 0.0f, 0.0f );
		Normal = Vector3D( 0.0f, 0.0f, 0.0f );
		Distance = 0.0f;
	}

	Vector3D Point;
	Vector3D Normal;
	float Distance;
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

	const Vector3D HalfWidthA = ( MaximumA - MinimumA ) * 0.5f;
	const Vector3D HalfWidthB = ( MaximumB - MinimumB ) * 0.5f;

	Vector3D AxisDistance = ( CenterA - CenterB );
	Vector3D Radius = AxisDistance;
	Radius.X = fabs( Radius.X );
	Radius.Y = fabs( Radius.Y );
	Radius.Z = fabs( Radius.Z );
	Radius = Radius - ( HalfWidthA + HalfWidthB );

	Vector3D LocalNormal;
	LocalNormal.X = fabs( Radius.X );
	LocalNormal.Y = fabs( Radius.Y );
	LocalNormal.Z = fabs( Radius.Z );

	AxisDistance = -AxisDistance;

	if( LocalNormal.X < LocalNormal.Y && LocalNormal.X < LocalNormal.Z )
	{
		Response.Distance = LocalNormal.X;
		Response.Normal.X = sign( AxisDistance.X );
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = 0.0f;
	}
	else if( LocalNormal.Y < LocalNormal.X && LocalNormal.Y < LocalNormal.Z )
	{
		Response.Distance = LocalNormal.Y;
		Response.Normal.X = 0.0f;
		Response.Normal.Y = sign( AxisDistance.Y );
		Response.Normal.Z = 0.0f;
	}
	else
	{
		Response.Distance = LocalNormal.Z;
		Response.Normal.X = 0.0f;
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = sign( AxisDistance.Z );
	}

#if DrawDebugLines == 1
	UI::AddAABB( MinimumB, MaximumB, Color( 0, 127, 255 ) );

	bool IsInFront = false;
	auto ScreenPosition = UI::WorldToScreenPosition( CenterA, &IsInFront );
	if( IsInFront )
	{
		ScreenPosition.X += 10.0f;

		ScreenPosition.Y += TextPosition * 15.0f;
		char VectorString[64];
		sprintf_s( VectorString, "N %.2f %.2f %.2f", Response.Normal.X, Response.Normal.Y, Response.Normal.Z );
		UI::AddText( ScreenPosition, VectorString );

		TextPosition++;
		ScreenPosition.Y += 15.0f;
		sprintf_s( VectorString, "d %.2f %.2f %.2f", AxisDistance.X, AxisDistance.Y, AxisDistance.Z );
		UI::AddText( ScreenPosition, VectorString );

		TextPosition++;
		ScreenPosition.Y += 15.0f;
		sprintf_s( VectorString, "r %.2f %.2f %.2f", Radius.X, Radius.Y, Radius.Z );
		UI::AddText( ScreenPosition, VectorString );

		TextPosition++;

		const auto NormalEnd = CenterA + Response.Normal * Response.Distance;
		UI::AddLine( CenterA, NormalEnd, Color( 0, 255, 255 ) );
		UI::AddCircle( NormalEnd, 5.0f, Color( 0, 255, 255 ) );
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

	Response.Normal = -FaceA.Cross( FaceB );
	Response.Normal = Response.Normal.Normalized();
	Response.Distance = Response.Normal.Dot( VertexA );

	const float Radius = Extent.X * fabs( Response.Normal.X ) + Extent.Y * fabs( Response.Normal.Y ) + Extent.Z * fabs( Response.Normal.Z );
	if( Response.Distance > Radius )
	{
		Response.Distance = 0.0f;
	}

	return Response;
}

CollisionResponse CollisionResponseTreeAABB( TriangleTree* Tree, const FBounds& WorldBounds, FTransform& ComponentTransform )
{
	CollisionResponse Response;

	if( !Tree )
		return Response;

	FBounds LocalBounds = WorldBounds;
	auto& Matrix = glm::inverse( ComponentTransform.GetTransformationMatrix() );
	LocalBounds.Maximum = Math::FromGLM( glm::vec3( Matrix * glm::vec4( Math::ToGLM( LocalBounds.Maximum ), 1.0f ) ) );
	LocalBounds.Minimum = Math::FromGLM( glm::vec3( Matrix * glm::vec4( Math::ToGLM( LocalBounds.Minimum ), 1.0f ) ) );

	const Vector3D Center = ( LocalBounds.Maximum + LocalBounds.Minimum ) * 0.5f;
	const Vector3D Extent = ( LocalBounds.Maximum - LocalBounds.Minimum ) * 0.5f;

	bool TouchesBounds = false;
	if( Math::BoundingBoxIntersection( LocalBounds.Minimum, LocalBounds.Maximum, Tree->Bounds.Minimum, Tree->Bounds.Maximum ) )
	{
		if( !Tree->Upper && !Tree->Lower )
		{
			VisualizeBounds( Tree, &ComponentTransform );

			for( unsigned int Index = 0; Index < Tree->Vertices.size(); )
			{
				const Vector3D& VertexA = Tree->Vertices[Index];
				const Vector3D& VertexB = Tree->Vertices[Index + 1];
				const Vector3D& VertexC = Tree->Vertices[Index + 2];

				CollisionResponse TriangleResponse = CollisionResponseTriangleAABB( VertexA, VertexB, VertexC, Center, Extent );
				if( fabs( TriangleResponse.Distance ) > fabs( Response.Distance ) )
				{
					Response.Normal = TriangleResponse.Normal;
					Response.Distance = TriangleResponse.Distance;

#if DrawDebugLines == 1
					const Vector3D TriangleCenter = ( VertexA + VertexB + VertexC ) / 3.0f;
					UI::AddLine( VertexA, VertexA - TriangleResponse.Normal * TriangleResponse.Distance, Color( 255, 0, 0 ) );
					UI::AddLine( VertexB, VertexB - TriangleResponse.Normal * TriangleResponse.Distance, Color( 0, 255, 0 ) );
					UI::AddLine( VertexC, VertexC - TriangleResponse.Normal * TriangleResponse.Distance, Color( 0, 0, 255 ) );
#endif
				}

				Index += 3;
			}
		}
		else
		{
			CollisionResponse ResponseA = CollisionResponseTreeAABB( Tree->Upper, WorldBounds, ComponentTransform );
			if( fabs( ResponseA.Distance ) > fabs( Response.Distance ) )
			{
				Response.Normal = ResponseA.Normal;
				Response.Distance = ResponseA.Distance;
			}

			CollisionResponse ResponseB = CollisionResponseTreeAABB( Tree->Lower, WorldBounds, ComponentTransform );
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

CPhysicsComponent::CPhysicsComponent( CMeshEntity* OwnerIn, const FBounds& LocalBounds, const bool Static, const bool Stationary )
{
	Owner = OwnerIn;
	Block = true;
	DeltaPosition = Vector3D( 0.0f, 0.0f, 0.0f );

	this->LocalBounds = LocalBounds;
	this->Static = Static;
	this->Stationary = Stationary;

	Tree = nullptr;
}

CPhysicsComponent::~CPhysicsComponent()
{
}

void CPhysicsComponent::Construct( CPhysics* Physics )
{
	Physics->Register( this );
	CalculateBounds();

	PreviousTransform = Owner->GetTransform();
	DeltaPosition = Vector3D( 0.0f, 0.0f, 0.0f );
	Velocity = Vector3D( 0.0f, 0.0f, 0.0f );
	Depenetration = Vector3D( 0.0f, 0.0f, 0.0f );
}

void CPhysicsComponent::PreCollision()
{
	Normal = Vector3D( 0.0f, 0.0f, 0.0f );
}

void CPhysicsComponent::Collision( CPhysicsComponent* Component )
{
	const float DeltaTime = GameLayersInstance->GetDeltaTime();
	auto Transform = Owner->GetTransform();

	Contact = true;

	CollisionResponse Response;

	// Dynamic interaction with static geometry.
	if( !Static && Component->Static )
	{
		if( Component->Owner && Component->Tree )
		{
			Response = CollisionResponseTreeAABB( Component->Tree, WorldBounds, Component->PreviousTransform );
		}
		else
		{
			Response = CollisionResponseAABBAABB( WorldBounds, Component->WorldBounds );
		}
	}
	else
	{
		Response = CollisionResponseAABBAABB( WorldBounds, Component->WorldBounds );
	}
	
	const Vector3D RelativeVelocity = Component->Velocity - Velocity;
	float VelocityAlongNormal = RelativeVelocity.Dot( Response.Normal ) - Response.Distance * InverseMass;

	if( VelocityAlongNormal < 0.0f )
	{
		Contacts++;
		const float Restitution = -1.01f;
		const float DeltaVelocity = Restitution * VelocityAlongNormal;
		const float Scale = DeltaVelocity / ( InverseMass + Component->InverseMass );
		Vector3D Impulse = Response.Normal * Scale;

		Normal += Response.Normal;

		Velocity -= InverseMass * Impulse;
		Component->Velocity += Component->InverseMass * Impulse;

#if DrawDebugLines == 1
		auto Position = Transform.GetPosition();
		const auto ImpulseEnd = Position + Impulse + ( InverseMass * Impulse * Mass );
		UI::AddLine( Position, ImpulseEnd, Color( 255, 0, 0 ) );
		UI::AddCircle( ImpulseEnd, 5.0f, Color( 255, 0, 0 ) );
		UI::AddText( ImpulseEnd, std::to_string( VelocityAlongNormal ).c_str() );
#endif
	}

	if( !Static && !Stationary )
	{
		UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, Color::Blue );

		Vector3D ProjectionPosition = Response.Normal * Response.Distance;
		UI::AddLine( Transform.GetPosition(), Transform.GetPosition() - Depenetration, Color( 0, 0, 255 ) );
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
#endif
}

std::vector<Vector3D> GatherVertices( std::vector<Vector3D>& Vertices, const Vector3D& Median, const bool Direction, const size_t Axis )
{
	std::vector<Vector3D> GatheredVertices;
	GatheredVertices.reserve( Vertices.size() );

	for( unsigned int Index = 0; Index < Vertices.size(); )
	{
		bool InsideBounds = false;

		Vector3D TriangleCenter = Vertices[Index] + Vertices[Index + 1] + Vertices[Index + 2];
		TriangleCenter /= 3.0f;

		for( unsigned int TriangleIndex = 0; TriangleIndex < 3; TriangleIndex++ )
		{
			Vector3D Vertex = Vertices[Index + TriangleIndex];
			if( Direction )
			{
				if( Axis == 0 && Vertex.X >= Median.X )
				{
					InsideBounds = true;
				}
				else if( Axis == 1 && Vertex.Y >= Median.Y )
				{
					InsideBounds = true;
				}
				else if( Axis == 2 && Vertex.Z >= Median.Z )
				{
					InsideBounds = true;
				}
			}
			else
			{
				if( Axis == 0 && Vertex.X < Median.X )
				{
					InsideBounds = true;
				}
				else if( Axis == 1 && Vertex.Y < Median.Y )
				{
					InsideBounds = true;
				}
				else if( Axis == 2 && Vertex.Z < Median.Z )
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

	std::vector<Vector3D>& Vertices = Tree->Vertices;
	if( Vertices.size() <= 25 )
		return;

	Vector3D LocalMedian = Vector3D( 0.0f, 0.0f, 0.0f );
	for( size_t Index = 0; Index < Vertices.size(); Index++ )
	{
		LocalMedian += Vertices[Index];
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

	if( Depth > 0 )
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
		BuildMedian( Tree->Upper, Transform, UpperBounds, Depth - 1 );

		Tree->Lower->Vertices = GatherVertices( Vertices, Median, false, Axis );
		BuildMedian( Tree->Lower, Transform, LowerBounds, Depth - 1 );
	}
}

void VisualizeBounds( TriangleTree* Tree, FTransform* Transform )
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
					UI::AddAABB( Transform->Transform( Tree->Bounds.Minimum ), Transform->Transform( Tree->Bounds.Maximum ) );
				}
				else
				{
					UI::AddAABB( Tree->Bounds.Minimum, Tree->Bounds.Maximum );
				}
				

				for( size_t Index = 0; Index < Tree->Vertices.size(); )
				{
					Vector3D VertexA = Tree->Vertices[Index];
					Vector3D VertexB = Tree->Vertices[Index + 1];
					Vector3D VertexC = Tree->Vertices[Index + 2];

					if( Transform )
					{
						VertexA = Transform->Transform( VertexA );
					}

					// UI::AddText( VertexA, std::to_string( Index ).c_str() );

					Index += 3;
				}
			}
		}

		VisualizeBounds( Tree->Upper );
		VisualizeBounds( Tree->Lower );
	}
}

void VisualizeBVH( CMesh* Mesh, FTransform& Transform, const FBounds& WorldBounds, TriangleTree*& Tree )
{
	if( !Tree )
	{
		Tree = new TriangleTree();

		const auto& VertexData = Mesh->GetVertexData();
		const auto& IndexData = Mesh->GetIndexData();
		const auto& VertexBufferData = Mesh->GetVertexBufferData();

		std::vector<Vector3D>& Vertices = Tree->Vertices;
		Vertices.reserve( VertexBufferData.IndexCount );

		for( unsigned int Index = 0; Index < VertexBufferData.IndexCount; )
		{
			Vertices.emplace_back( VertexData.Vertices[IndexData.Indices[Index]].Position );
			Vertices.emplace_back( VertexData.Vertices[IndexData.Indices[Index] + 1].Position );
			Vertices.emplace_back( VertexData.Vertices[IndexData.Indices[Index] + 2].Position );

			Index += 3;
		}

		BuildMedian( Tree, Transform, WorldBounds, 16 );
	}

	// VisualizeBounds( Tree );
}

void CPhysicsComponent::Tick()
{
#if DrawDebugLines == 1
	TextPosition = 0;
#endif

	const bool Unmoving = Static || Stationary;
	InverseMass = Unmoving ? 0.0f : 1.0f / Mass;

	auto Transform = Owner->GetTransform();
	VisualizeBVH( Owner->Mesh, Transform, LocalBounds, Tree );

	if( Static )
		return;

	CalculateBounds();

	float DeltaTime = GameLayersInstance->GetDeltaTime();
	if( DeltaTime > 1.0f )
	{
		DeltaTime = 0.333333f;
	}

	auto NewPosition = Transform.GetPosition();

	if( !Stationary )
	{
		Acceleration -= Gravity;
		Velocity += ( Acceleration * DeltaTime ) * InverseMass;

		Velocity.X *= 0.9f;
		Velocity.Y *= 0.9f;
	}

#if DrawDebugLines == 1
	auto Position = Transform.GetPosition();
	UI::AddLine( Position, Position + Velocity * 10.0f, Color::Green );
	UI::AddCircle( Position + Velocity * 10.0f, 3.0f, Color::Green );
	UI::AddLine( Position, Position + Acceleration, Color::Blue );
	UI::AddCircle( Position + Acceleration, 3.0f, Color::Blue );

	bool IsInFront = false;
	auto ScreenPosition = UI::WorldToScreenPosition( Position, &IsInFront );
	if( IsInFront )
	{
		ScreenPosition.Y += TextPosition * 15.0f;
		char MassString[32];
		sprintf_s( MassString, "%.2f kg", Mass );
		UI::AddText( ScreenPosition, MassString );

		TextPosition++;

		ScreenPosition.X += 10.0f;
		ScreenPosition.Y += 15.0f;
		char String[32];
		sprintf_s( String, "v %.2f %.2f %.2f", Velocity.X, Velocity.Y, Velocity.Z );
		UI::AddText( ScreenPosition, String );

		TextPosition++;
	}
#endif

	if( Contacts > 0 )
	{
		Vector3D NormalForce = Velocity.Dot( Normal ) * Normal;
		Velocity -= NormalForce;
	}

	if( !Stationary )
	{
		NewPosition += Velocity;

		if( Contacts > 0 )
		{
			NewPosition += Depenetration;
			Depenetration = Vector3D( 0.0f, 0.0f, 0.0f );
		}

		if( NewPosition.Z < -20.0f )
		{
			if( Velocity.Z < 0.0f )
			{
				Velocity.Z = 0.0f;
			}

			NewPosition.Z = -20.0f;
			Normal = Vector3D( 0.0f, 0.0f, -1.0f );
			Contact = true;
		}
	}

	Velocity = NewPosition - PreviousTransform.GetPosition();

	// if( !Stationary && Contacts > 0 )
	// {
	// 	Velocity -= Depenetration;
	// 	Depenetration = Vector3D( 0.0f, 0.0f, 0.0f );
	// }

	Transform.SetPosition( NewPosition );

	Owner->SetTransform( Transform );
	Owner->Contact = Contact;

	PreviousTransform = Owner->GetTransform();
	Acceleration = Vector3D( 0.0f, 0.0f, 0.0f );

	Contacts = 0;
}

void CPhysicsComponent::Destroy( CPhysics* Physics )
{
	Physics->Unregister( this );
}

void CPhysicsComponent::CalculateBounds()
{
	FTransform Transform = Owner->GetTransform();
	const auto& Minimum = LocalBounds.Minimum;
	const auto& Maximum = LocalBounds.Maximum;

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
		Positions[PositionIndex] = Transform.Transform( Positions[PositionIndex] );
	}

	FBounds TransformedAABB = Math::AABB( Positions, 8 );
	WorldBounds = TransformedAABB;

	Mass = ( WorldBounds.Maximum - WorldBounds.Minimum ).Length() * 100.0f;
}

FBounds CPhysicsComponent::GetBounds() const
{
	return WorldBounds;
}

void CPhysicsComponent::Debug()
{
	if( !Owner )
		return;

	FTransform Transform = Owner->GetTransform();
	VisualizeBounds( Tree, &Transform );
}
