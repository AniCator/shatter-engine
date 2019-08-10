// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PhysicsComponent.h"

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Physics/Physics.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#include <Engine/Display/UserInterface.h>

#include <Game/Game.h>

static const auto Gravity = Vector3D( 0.0f, 0.0f, 9.81f );

CPhysicsComponent::CPhysicsComponent( CMeshEntity* OwnerIn )
{
	Owner = OwnerIn;
	Static = Owner->IsStatic();
	Block = true;

	auto Bounds = Owner->Mesh->GetBounds();
	Mass = (( Bounds.Maximum - Bounds.Minimum ) * Owner->GetTransform().GetSize()).Length() * 45.0f;
}

CPhysicsComponent::~CPhysicsComponent()
{
}

void CPhysicsComponent::Construct( CPhysics* Physics )
{
	Physics->Register( this );
	CalculateBounds();

	PreviousTransform = Owner->GetTransform();
	Velocity = Vector3D( 0.0f, 0.0f, 0.0f );
}

void CPhysicsComponent::Collision( CPhysicsComponent* Component )
{
	const float DeltaTime = GameLayersInstance->GetDeltaTime();

	Contact = true;
	const Vector3D& MinimumA = Bounds.Minimum;
	const Vector3D& MaximumA = Bounds.Maximum;

	const Vector3D& MinimumB = Component->Bounds.Minimum;
	const Vector3D& MaximumB = Component->Bounds.Maximum;

	const Vector3D& CenterA = Position;
	const Vector3D& CenterB = Component->Position;

	Vector3D Delta = ( CenterA - CenterB );
	Vector3D Overlap = Velocity;

	Vector3D Normal = Vector3D( 0.0f, 0.0f, 0.0f );
	if( Overlap.X < Overlap.Y && Overlap.X < Overlap.Z )
	{
		Normal.X = ( 0.0f < Overlap.X ) - ( Overlap.X < 0.0f );
	}
	else if( Overlap.Y < Overlap.X && Overlap.Y < Overlap.Z )
	{
		Normal.Y = ( 0.0f < Overlap.Y ) - ( Overlap.Y < 0.0f );
	}
	else
	{
		Normal.Z = ( 0.0f < Overlap.Z ) - ( Overlap.Z < 0.0f );
	}

	const Vector3D RelativeVelocity = Component->Velocity - Velocity;
	float VelocityAlongNormal = RelativeVelocity.Dot( Normal );

	if( VelocityAlongNormal < 0.0f )
	{
		const float InverseMassA = Static ? 0.0f : 1.0f / Mass;
		const float InverseMassB = Component->Static ? 0.0f : 1.0f / Component->Mass;

		const float Restitution = -1.001f;
		const float Scale = ( Restitution * VelocityAlongNormal ) / ( InverseMassA + InverseMassB );
		Vector3D Impulse = Normal * Scale;

		Velocity -= InverseMassA * Impulse;
		Component->Velocity += InverseMassB * Impulse;

		UI::AddLine( Position, Position - Impulse, Color::Red );
		UI::AddCircle( Position - Impulse, 5.0f, Color::Red );
	}

	Overlap.X = CenterA.X + CenterB.X - fabs( Delta.X );
	Overlap.Y = CenterA.Y + CenterB.Y - fabs( Delta.Y );
	Overlap.Z = CenterA.Z + CenterB.Z - fabs( Delta.Z );

	if( !Static )
	{
		auto TransformA = Owner->GetTransform();
		auto NewPositionA = TransformA.GetPosition();
		NewPositionA += Normal * VelocityAlongNormal;
		TransformA.SetPosition( NewPositionA );
		Owner->SetTransform( TransformA );
	}
}

void CPhysicsComponent::Tick()
{
	Contacts = 0;

	if( Static )
		return;

	CalculateBounds();

	const float DeltaTime = GameLayersInstance->GetDeltaTime();

	auto Transform = Owner->GetTransform();
	auto NewPosition = Transform.GetPosition();

	Acceleration -= Gravity;
	Velocity += ( Acceleration * DeltaTime ) / Mass;

	UI::AddLine( Position, Position + Velocity * 10.0f, Color::Green );
	UI::AddCircle( Position + Velocity * 10.0f, 3.0f, Color::Green );
	UI::AddLine( Position, Position + Acceleration, Color::Blue );
	UI::AddCircle( Position + Acceleration, 3.0f, Color::Blue );

	char MassString[32];
	sprintf_s( MassString, "%.2f kg", Mass );
	UI::AddText( Position, MassString );

	NewPosition += Velocity;

	if( NewPosition.Z < -0.5f )
	{
		if( Velocity.Z < 0.0f )
		{
			Velocity.Z = 0.0f;
		}

		NewPosition.Z = -0.5f;
	}

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
	FBounds Temp = Owner->Mesh->GetBounds();
	auto Transform = Owner->GetLocalTransform();
	Temp.Minimum = Math::FromGLM( Transform.Position( Math::ToGLM( Temp.Minimum ) ) );
	Temp.Maximum = Math::FromGLM( Transform.Position( Math::ToGLM( Temp.Maximum ) ) );

	Bounds.Minimum.X = Temp.Minimum.X > Temp.Maximum.X ? Temp.Maximum.X : Temp.Minimum.X;
	Bounds.Minimum.Y = Temp.Minimum.Y > Temp.Maximum.Y ? Temp.Maximum.Y : Temp.Minimum.Y;
	Bounds.Minimum.Z = Temp.Minimum.Z > Temp.Maximum.Z ? Temp.Maximum.Z : Temp.Minimum.Z;
	
	Bounds.Maximum.X = Temp.Maximum.X > Temp.Minimum.X ? Temp.Maximum.X : Temp.Minimum.X;
	Bounds.Maximum.Y = Temp.Maximum.Y > Temp.Minimum.Y ? Temp.Maximum.Y : Temp.Minimum.Y;
	Bounds.Maximum.Z = Temp.Maximum.Z > Temp.Minimum.Z ? Temp.Maximum.Z : Temp.Minimum.Z;

	Position = ( Bounds.Maximum + Bounds.Minimum ) * 0.5f;
}

FBounds CPhysicsComponent::GetBounds() const
{
	return Bounds;
}
