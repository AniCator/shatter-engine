// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PhysicsComponent.h"

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Physics/Physics.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#include <Engine/Display/UserInterface.h>

#include <Game/Game.h>

static const auto Gravity = Vector3D( 0.0f, 0.0f, 9.81f );

#define DrawDebugLines 0

#if DrawDebugLines == 1
size_t TextPosition = 0;
#endif

float sign( const float& Input )
{
	return ( 0.0f < Input ) - ( Input < 0.0f );
}

struct CollisionResponse
{
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

CPhysicsComponent::CPhysicsComponent( CMeshEntity* OwnerIn, const FBounds& LocalBounds, const bool Static, const bool Stationary )
{
	Owner = OwnerIn;
	Block = true;
	DeltaPosition = Vector3D( 0.0f, 0.0f, 0.0f );

	this->LocalBounds = LocalBounds;
	this->Static = Static;
	this->Stationary = Stationary;
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

	auto Response = CollisionResponseAABBAABB( WorldBounds, Component->WorldBounds );
	
	const bool UnmovingA = Static || Stationary;
	const bool UnmovingB = Component->Static || Component->Stationary;
	const float InverseMassA = UnmovingA ? 0.0f : 1.0f / Mass;
	const float InverseMassB = UnmovingB ? 0.0f : 1.0f / Component->Mass;

	const Vector3D RelativeVelocity = Component->Velocity - Velocity;
	float VelocityAlongNormal = RelativeVelocity.Dot( Response.Normal ) - Response.Distance * InverseMassA;

	if( VelocityAlongNormal < 0.0f )
	{
		Contacts++;
		const float Restitution = -1.01f;
		const float DeltaVelocity = Restitution * VelocityAlongNormal;
		const float Scale = DeltaVelocity / ( InverseMassA + InverseMassB );
		Vector3D Impulse = Response.Normal * Scale;

		Normal += Response.Normal;

		Velocity -= InverseMassA * Impulse;
		Component->Velocity += InverseMassB * Impulse;

#if DrawDebugLines == 1
		auto Position = Transform.GetPosition();
		const auto ImpulseEnd = Position + Impulse;
		UI::AddLine( Position, ImpulseEnd, Color( 255, 0, 0 ) );
		UI::AddCircle( ImpulseEnd, 5.0f, Color( 255, 0, 0 ) );
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

	UI::AddAABB( Bounds.Minimum, Bounds.Maximum, BoundsColor );
#endif
}

void CPhysicsComponent::Tick()
{
	Contacts = 0;

#if DrawDebugLines == 1
	TextPosition = 0;
#endif

	if( Static )
		return;

	CalculateBounds();

	float DeltaTime = GameLayersInstance->GetDeltaTime();
	if( DeltaTime > 1.0f )
	{
		DeltaTime = 0.333333f;
	}

	auto Transform = Owner->GetTransform();
	auto NewPosition = Transform.GetPosition();

	if( !Stationary )
	{
		Acceleration -= Gravity;
		Velocity += ( Acceleration * DeltaTime ) / Mass;

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

	if( !Stationary )
	{
		NewPosition += Velocity;

		if( NewPosition.Z < -15.0f )
		{
			if( Velocity.Z < 0.0f )
			{
				Velocity.Z = 0.0f;
			}

			NewPosition.Z = -15.0f;
			Normal = Vector3D( 0.0f, 0.0f, -1.0f );
			Contact = true;
		}
	}

	Velocity = NewPosition - PreviousTransform.GetPosition();

	Transform.SetPosition( NewPosition );

	Owner->SetTransform( Transform );
	Owner->Contact = Contact;

	PreviousTransform = Owner->GetTransform();
	Acceleration = Vector3D( 0.0f, 0.0f, 0.0f );
}

void CPhysicsComponent::Destroy( CPhysics* Physics )
{
	Physics->Unregister( this );
}

void CPhysicsComponent::CalculateBounds()
{
	Mass = ( ( LocalBounds.Maximum - LocalBounds.Minimum ) * Owner->GetTransform().GetSize() ).Length() * 45.0f;

	FBounds Temp = LocalBounds;
	auto Transform = Owner->GetLocalTransform();
	Temp.Minimum = Math::FromGLM( Transform.Position( Math::ToGLM( Temp.Minimum ) ) );
	Temp.Maximum = Math::FromGLM( Transform.Position( Math::ToGLM( Temp.Maximum ) ) );

	WorldBounds.Minimum.X = Math::Min( Temp.Minimum.X, Temp.Maximum.X );
	WorldBounds.Minimum.Y = Math::Min( Temp.Minimum.Y, Temp.Maximum.Y );
	WorldBounds.Minimum.Z = Math::Min( Temp.Minimum.Z, Temp.Maximum.Z );
	
	WorldBounds.Maximum.X = Math::Max( Temp.Minimum.X, Temp.Maximum.X );
	WorldBounds.Maximum.Y = Math::Max( Temp.Minimum.Y, Temp.Maximum.Y );
	WorldBounds.Maximum.Z = Math::Max( Temp.Minimum.Z, Temp.Maximum.Z );
}

FBounds CPhysicsComponent::GetBounds() const
{
	return WorldBounds;
}
