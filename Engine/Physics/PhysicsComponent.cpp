// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PhysicsComponent.h"

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Physics/Physics.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#include <Game/Game.h>

CPhysicsComponent::CPhysicsComponent( CMeshEntity* OwnerIn )
{
	Owner = OwnerIn;
	Static = Owner->IsStatic();
}

CPhysicsComponent::~CPhysicsComponent()
{
}

void CPhysicsComponent::Construct( CPhysics* Physics )
{
	Physics->Register( this );
	CalculateBounds();

	PreviousTransform = Owner->GetTransform();
	PreviousVelocity = Vector3D( 0.0f, 0.0f, 0.0f );
}

void CPhysicsComponent::Tick()
{
	if( Static )
		return;

	CalculateBounds();

	Vector3D Velocity = Vector3D( 0.0f, 0.0f, 0.0f );
	const float DeltaTime = GameLayersInstance->GetDeltaTime();

	if( fabs( CorrectiveForce.X ) > 0.01f ||
		fabs( CorrectiveForce.Y ) > 0.01f ||
		fabs( CorrectiveForce.Z ) > 0.01f )
	{
		Owner->Contact = true;

		auto Transform = Owner->GetTransform();

		auto PositionA = Transform.GetPosition();
		auto PositionB = PreviousTransform.GetPosition();
		Velocity = PositionB - PositionA;

		Transform.SetPosition( Transform.GetPosition() - PreviousVelocity * 0.05f * DeltaTime );
		Owner->SetTransform( Transform );
		CorrectiveForce = Vector3D( 0.0f, 0.0f, 0.0f );
	}
	else
	{
		Owner->Contact = false;

		auto Transform = Owner->GetTransform();

		auto PositionA = Transform.GetPosition();
		auto PositionB = PreviousTransform.GetPosition();
		Velocity = PositionB - PositionA;

		auto Bounds = Owner->Mesh->GetBounds();
		auto Difference = Bounds.Maximum - Bounds.Minimum;

		Velocity.Z += Difference.Length() * 0.000981f + PreviousVelocity.Z * DeltaTime;
		Transform.SetPosition( PositionA - Vector3D( 0.0f, 0.0f, Velocity.Z ) );
		Owner->SetTransform( Transform );
	}

	PreviousTransform = Owner->GetTransform();
	PreviousVelocity = Velocity;
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
}

FBounds CPhysicsComponent::GetBounds() const
{
	return Bounds;
}
