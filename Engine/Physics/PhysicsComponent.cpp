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
#define DrawCollisionResponseAABBAABB 1

size_t TextPosition = 0;

void VisualizeBounds( TriangleTree* Tree, FTransform* Transform = nullptr, Color BoundsColor = Color::Blue );

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

	const Vector3D SizeA = ( MaximumA - MinimumA );
	const Vector3D SizeB = ( MaximumB - MinimumB );

	const Vector3D HalfSizeA = SizeA * 0.5f;
	const Vector3D HalfSizeB = SizeB * 0.5f;

	const Vector3D CenterDistance = ( CenterA - CenterB );
	const Vector3D CenterDistanceSquared = CenterDistance * CenterDistance;
	if( CenterDistanceSquared.Z > CenterDistanceSquared.Y && CenterDistanceSquared.Z > CenterDistanceSquared.X)
	{
		Response.Distance = fabs( CenterDistance.Z ) - ( HalfSizeA.Z + HalfSizeB.Z);
		Response.Normal.X = 0.0f;
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = sign( -CenterDistance.Z );
	}
	else if( CenterDistanceSquared.Y > CenterDistanceSquared.X )
	{
		Response.Distance = fabs( CenterDistance.Y ) - ( HalfSizeA.Y + HalfSizeB.Y );
		Response.Normal.X = 0.0f;
		Response.Normal.Y = sign( -CenterDistance.Y );
		Response.Normal.Z = 0.0f;
	}
	else
	{
		Response.Distance = fabs( CenterDistance.X ) - ( HalfSizeA.X + HalfSizeB.X );
		Response.Normal.X = sign( -CenterDistance.X );
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = 0.0f;
	}


#if DrawCollisionResponseAABBAABB == 1
	UI::AddAABB( MinimumB, MaximumB, Color( 0, 127, 255 ) );

	bool IsInFront = false;
	auto ScreenPosition = UI::WorldToScreenPosition( CenterA, &IsInFront );
	if( IsInFront )
	{
		ScreenPosition.X += 10.0f;

		ScreenPosition.Y += TextPosition * 15.0f;
		char VectorString[64];
		sprintf_s( VectorString, "c %.2f %.2f %.2f", CenterDistance.X, CenterDistance.Y, CenterDistance.Z );
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

	// Response.Normal = FaceB.Cross( FaceA );
	auto SummedNormal = A.Normal + B.Normal + C.Normal;
	Response.Normal = -SummedNormal.Normalized();
	Response.Distance = Response.Normal.Dot( A.Normal );

	const float Radius = Extent.X * fabs( Response.Normal.X ) + Extent.Y * fabs( Response.Normal.Y ) + Extent.Z * fabs( Response.Normal.Z );
	if( Response.Distance > Radius )
	{
		Response.Distance = 0.0f;
	}

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
	const Vector3D Extent = ( LocalBounds.Maximum - LocalBounds.Minimum ) * 0.5f;

#if DrawDebugLines == 1
	auto Position = BodyTransform.GetPosition();
	// UI::AddAABB( Position + LocalBounds.Minimum, Position + LocalBounds.Maximum, Color::Green );
	// UI::AddAABB( Position + Tree->Bounds.Minimum, Position + Tree->Bounds.Maximum, Color::Blue );
#endif

	if( Math::BoundingBoxIntersection( LocalBounds.Minimum, LocalBounds.Maximum, Tree->Bounds.Minimum, Tree->Bounds.Maximum ) )
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
		else
		{
			CollisionResponse ResponseA = CollisionResponseTreeAABB( Tree->Upper, WorldBounds, BodyTransform );
			if( fabs( ResponseA.Distance ) > fabs( Response.Distance ) )
			{
				Response.Normal = ResponseA.Normal;
				Response.Distance = ResponseA.Distance;
			}

			CollisionResponse ResponseB = CollisionResponseTreeAABB( Tree->Lower, WorldBounds, BodyTransform );
			if( fabs( ResponseB.Distance ) > fabs( Response.Distance ) )
			{
				Response.Normal = ResponseB.Normal;
				Response.Distance = ResponseB.Distance;
			}

			Response = ResponseA;
			Response.Normal += ResponseB.Normal;
			Response.Distance += ResponseB.Distance;

			Response.Normal *= 0.5f;
			Response.Distance *= 0.5f;
		}
	}

	Response.Normal = Response.Normal.Normalized();

	return Response;
}

CBody::CBody( CMeshEntity* OwnerIn, const FBounds& LocalBounds, const bool Static, const bool Stationary )
{
	Owner = OwnerIn;
	Block = true;
	DeltaPosition = Vector3D( 0.0f, 0.0f, 0.0f );

	this->LocalBounds = LocalBounds;
	this->Static = Static;
	this->Stationary = Stationary;

	Tree = nullptr;
}

CBody::~CBody()
{
	if( Tree )
	{
		delete Tree;
	}
}

void CBody::Construct( CPhysics* Physics )
{
	Physics->Register( this );
	CalculateBounds();

	PreviousTransform = Owner->GetTransform();
	DeltaPosition = Vector3D( 0.0f, 0.0f, 0.0f );
	Velocity = Vector3D( 0.0f, 0.0f, 0.0f );
	Depenetration = Vector3D( 0.0f, 0.0f, 0.0f );
}

void CBody::PreCollision()
{
	Normal = Vector3D( 0.0f, 0.0f, 0.0f );
	Contacts = 0;
}

void CBody::Collision( CBody* Body )
{
	const float DeltaTime = GameLayersInstance->GetDeltaTime();
	auto Transform = Owner->GetTransform();

	Contact = true;

	CollisionResponse Response;

	bool DebugUsingTree = false;
	// Dynamic interaction with static geometry.
	if( !Static && Body->Static )
	{
		if( Body->Owner && Body->Tree )
		{
			Response = CollisionResponseTreeAABB( Body->Tree, WorldBounds, Body->PreviousTransform );
			// Response.Distance = Response.Distance;
			DebugUsingTree = true;

		}
		else
		{
			Response = CollisionResponseAABBAABB( WorldBounds, Body->WorldBounds );
		}
	}
	else
	{
		Response = CollisionResponseAABBAABB( WorldBounds, Body->WorldBounds );
	}
	
	const Vector3D RelativeVelocity = Body->Velocity - Velocity;
	float VelocityAlongNormal = RelativeVelocity.Dot( Response.Normal ) + Response.Distance * InverseMass;

	if( VelocityAlongNormal < 0.0f )
	{
		Contacts++;
		const float Restitution = -1.01f;
		const float DeltaVelocity = Restitution * VelocityAlongNormal;
		const float Scale = DeltaVelocity / ( InverseMass + Body->InverseMass );
		Vector3D Impulse = Response.Normal * Scale;

		Normal += Response.Normal;
		Depenetration = DeltaVelocity * Response.Normal * Response.Distance;

		Velocity -= InverseMass * Impulse;
		Body->Velocity += Body->InverseMass * Impulse;

		if( Depenetration.X > 0.001f || Depenetration.Y > 0.001f || Depenetration.Z > 0.001f )
		{
			Transform.SetPosition( Transform.GetPosition() + Depenetration );
			Owner->SetTransform( Transform );
			Collision( Body );
		}

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
		UI::AddAABB( WorldBounds.Minimum + ProjectionPosition, WorldBounds.Maximum + ProjectionPosition, Color( 255, 0, 127 ) );

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
		}
#endif

#if DrawDebugLines == 1
		if( Contacts > 0 )
		{
			UI::AddLine( WorldBounds.Minimum, Body->WorldBounds.Minimum, Color::White );
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
		UI::AddAABB( Body->WorldBounds.Minimum, Body->WorldBounds.Maximum, BoundsColor );
	}
#endif
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
	if( Vertices.size() <= 25 )
		return;

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

void CreateBVH( CMesh* Mesh, FTransform& Transform, const FBounds& WorldBounds, TriangleTree*& Tree )
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
	const bool Unmoving = Static || Stationary;
	InverseMass = Unmoving ? 0.0f : 1.0f / Mass;

	auto Transform = Owner->GetTransform();
	if( Owner->IsStationary() )
	{
		CreateBVH( Owner->Mesh, Transform, LocalBounds, Tree );
	}

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

#if DrawDebugLines == 1
		UI::AddLine( Position, Position + NormalForce * 100.0f, Color( 32, 255, 255 ) );
#endif
	}

	if( !Stationary )
	{
		NewPosition += Velocity;

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

		Velocity = NewPosition - PreviousTransform.GetPosition();

		if( Contacts > 0 )
		{
			// NewPosition += Depenetration;

			Depenetration = Vector3D( 0.0f, 0.0f, 0.0f );
		}
	}

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
	TextPosition = 0;
}

void CBody::Destroy( CPhysics* Physics )
{
	Physics->Unregister( this );
}

void CBody::CalculateBounds()
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

	Mass = ( WorldBounds.Maximum - WorldBounds.Minimum ).Length() * 45.0f;
}

FBounds CBody::GetBounds() const
{
	return WorldBounds;
}

void CBody::Debug()
{
	if( !Owner )
		return;

	const static Color DebugColor = Color( 0, 255, 64 );
	UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, DebugColor );

	// VisualizeBounds( Tree, &PreviousTransform );
}
