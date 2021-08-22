// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LightEntity.h"

#include <Engine/Resource/Assets.h>

static CEntityFactory<LightEntity> Factory( "light" );

void LightEntity::Construct()
{
	if( LightIndex < 0 )
	{
		LightIndex = AllocateLight();
	}

	if( LightIndex < 0 )
		return;

	Lights[LightIndex] = Information;
}

void LightEntity::Tick()
{
	CPointEntity::Tick();
}

void LightEntity::Frame()
{
	CPointEntity::Frame();
}

void LightEntity::Destroy()
{
	CPointEntity::Destroy();
}

void LightEntity::Reload()
{
	CPointEntity::Reload();

	const auto Position = Vector3D( Information.Position.x, Information.Position.y, Information.Position.z );
	Transform.SetPosition( Position );
}

void LightEntity::Load( const JSON::Vector& Objects )
{
	int Type;
	float Radius, Intensity, AngleInner, AngleOuter;
	Vector3D Position, Orientation, Color;

	JSON::Assign( Objects, "position", Position );
	JSON::Assign( Objects, "rotation", Orientation );
	JSON::Assign( Objects, "light_type", Type );
	JSON::Assign( Objects, "radius", Radius );
	JSON::Assign( Objects, "intensity", Intensity );
	JSON::Assign( Objects, "color", Color );
	JSON::Assign( Objects, "angle_inner", AngleInner );
	JSON::Assign( Objects, "angle_outer", AngleOuter );

	const auto Direction = Math::EulerToDirection( Orientation );
	Information.Position.x = Position.X;
	Information.Position.y = Position.Y;
	Information.Position.z = Position.Z;
	Information.Position.w = static_cast<float>( Type );

	Information.Direction.x = Direction.X;
	Information.Direction.y = Direction.Y;
	Information.Direction.z = Direction.Z;
	Information.Direction.w = Radius;

	Information.Color.x = Color.X;
	Information.Color.y = Color.Y;
	Information.Color.z = Color.Z;
	Information.Color.w = Intensity;

	Information.Properties.x = AngleInner;
	Information.Properties.y = AngleOuter;
	Information.Properties.z = 0.0f;
	Information.Properties.w = 0.0f;
}

void LightEntity::Import( CData& Data )
{
	Serialize::Import( Data, "pos", Information.Position );
	Serialize::Import( Data, "dir", Information.Direction );
	Serialize::Import( Data, "col", Information.Color );
	Serialize::Import( Data, "prp", Information.Properties );
}

void LightEntity::Export( CData& Data )
{
	Serialize::Export( Data, "pos", Information.Position );
	Serialize::Export( Data, "dir", Information.Direction );
	Serialize::Export( Data, "col", Information.Color );
	Serialize::Export( Data, "prp", Information.Properties );
}

void LightEntity::Initialize()
{
	// Reset the allocation index.
	AllocationIndex = -1;

	// Reset the light type values.
	for( int32_t Index = 0; Index < LightMaximum; Index++ )
	{
		Lights[Index].Position.w = -1.0f;
	}
}

Light LightEntity::Lights[LightMaximum];
ShaderStorageBuffer<Light> LightEntity::LightBuffer;
int32_t LightEntity::AllocationIndex = -1;

int32_t LightEntity::AllocateLight()
{
	if( AllocationIndex < LightMaximum )
	{
		AllocationIndex++;
		return AllocationIndex;
	}

	return -1;
}

void LightEntity::UploadToGPU()
{
	// Create a new light buffer which destroys the previous one.
	LightBuffer = ShaderStorageBuffer<Light>();

	// Initialize the light buffer with the lighting data.
	LightBuffer.Initialize( Lights, LightMaximum );
}

void LightEntity::Bind()
{
	LightBuffer.Bind();
}

LightIndices LightEntity::Fetch( const Vector3D& Position )
{
	Vector3D LightPosition;
	int32_t Closest[4] { -1, -1, -1, -1 };
	float Distance = FLT_MAX;
	for( int32_t Index = 0; Index <= AllocationIndex; Index++ )
	{
		if( Lights[Index].Position.w < 0.0f )
			continue;

		LightPosition.X = Lights[Index].Position.x;
		LightPosition.Y = Lights[Index].Position.y;
		LightPosition.Z = Lights[Index].Position.z;

		const auto LightDistance = LightPosition.DistanceSquared( Position );
		const bool FreeIndex = Closest[0] == -1 || Closest[1] == -1 || Closest[2] == -1 || Closest[3] == -1;
		if( LightDistance < Distance || FreeIndex )
		{
			Distance = LightDistance;

			Closest[3] = Closest[2];
			Closest[2] = Closest[1];
			Closest[1] = Closest[0];

			Closest[0] = Index;
		}
	}

	LightIndices Indices;
	Indices.Index[0] = Closest[0];
	Indices.Index[1] = Closest[1];
	Indices.Index[2] = Closest[2];
	Indices.Index[3] = Closest[3];

	return Indices;
}
