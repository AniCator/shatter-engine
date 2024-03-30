#include "Recording.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/Physics/Body/Body.h>

void Recording::Apply( CMeshEntity* Entity, const double Time )
{
	if( Samples.size() <= 1 )
		return; // Not enough samples to play back.

	const auto ReplayStart = Samples[0].Time;
	size_t Index = 0;
	for( const auto& Sample : Samples )
	{
		const auto ReplayTime = Sample.Time - ReplayStart;
		if( ReplayTime > Time )
			break; // We've found our current sample.

		Index++;
	}

	if( Index >= Samples.size() )
	{
		Index = Samples.size() - 1;
	}

	auto& Sample = Samples[Index];
	Entity->SetTransform( Sample.Transform );

	// Update the physics data.
	if( auto* Body = Entity->GetBody() )
	{
		Body->PreviousTransform = Sample.Transform;
		Body->Acceleration = Vector3D::Zero;
		Body->Velocity = Vector3D::Zero;
	}

	auto& Instance = Entity->GetAnimationInstance();
	Instance.Stack.resize( Sample.Entries );
	for( size_t Index = 0; Index < Sample.Entries; Index++ )
	{
		auto& Entry = Sample.Stack[Index];
		if( Entry.Animation.PositionKeys.empty() && Entry.Animation.RotationKeys.empty() && Entry.Animation.ScalingKeys.empty() )
		{
			// Animation hasn't been copied yet.
			const auto& Animations = Instance.Mesh->GetSkeleton().Animations;
			const auto Iterator = Animations.find( Entry.Animation.Name );
			if( Iterator != Animations.end() )
			{
				Entry.Animation = Iterator->second;
			}
		}

		Instance.Stack[Index] = Sample.Stack[Index];
	}
}

void Recording::Apply( CRenderable& Renderable, Animator::Instance& Instance, const double Time )
{
	if( Samples.size() <= 1 )
		return; // Not enough samples to play back.

	const auto ReplayStart = Samples[0].Time;
	size_t Index = 0;
	for( const auto& Sample : Samples )
	{
		const auto ReplayTime = Sample.Time - ReplayStart;
		if( ReplayTime > Time )
			break; // We've found our current sample.

		Index++;
	}

	if( Index >= Samples.size() )
	{
		Index = Samples.size() - 1;
	}

	auto& Sample = Samples[Index];
	Renderable.GetRenderData().Transform = Sample.Transform;

	Instance.Stack.resize( Sample.Entries );
	for( size_t Index = 0; Index < Sample.Entries; Index++ )
	{
		auto& Entry = Sample.Stack[Index];
		if( Entry.Animation.PositionKeys.empty() && Entry.Animation.RotationKeys.empty() && Entry.Animation.ScalingKeys.empty() )
		{
			// Animation hasn't been copied yet.
			const auto& Animations = Instance.Mesh->GetSkeleton().Animations;
			const auto Iterator = Animations.find( Entry.Animation.Name );
			if( Iterator != Animations.end() )
			{
				Entry.Animation = Iterator->second;
			}
		}

		Instance.Stack[Index] = Sample.Stack[Index];
	}
}

CData& operator<<( CData& Data, const Recording& Recording )
{
	DataMarker::Mark( Data, "rec" );

	// Indicate how many samples we have.
	uint32_t SampleSize = Recording.Samples.size();
	Data << SampleSize;

	for( const auto& Sample : Recording.Samples )
	{
		DataMarker::Mark( Data, "s" );

		Data << Sample.Time;
		Data << Sample.Transform;
		Data << Sample.Entries;

		for( uint32_t EntryIndex = 0; EntryIndex < Sample.Entries; EntryIndex++ )
		{
			DataMarker::Mark( Data, "e" );
			const auto& Entry = Sample.Stack[EntryIndex];
			Data << Entry.Weight;
			Data << Entry.Mask;
			Data << Entry.Type;
			Data << Entry.Time;
			Data << Entry.PlayRate;
			Data << Entry.Loop;
			Data << Entry.Fixed;
			DataString::Encode( Data, Entry.Animation.Name );
		}
	}

	return Data;
}

CData& operator>>( CData& Data, Recording& Recording )
{
	if( !DataMarker::Check( Data, "rec" ) )
		return Data; // No samples found.

	// Retrieve up the sample count.
	uint32_t SampleSize = 0;
	Data >> SampleSize;

	// Allocate space for the samples.
	Recording.Samples.clear();
	Recording.Samples.resize( SampleSize );

	for( uint32_t Index = 0; Index < SampleSize; Index++ )
	{
		if( !DataMarker::Check( Data, "s" ) )
			break; // Not a valid sample.

		auto& Sample = Recording.Samples[Index];
		Data >> Sample.Time;
		Data >> Sample.Transform;
		Data >> Sample.Entries;

		for( uint32_t EntryIndex = 0; EntryIndex < Sample.Entries; EntryIndex++ )
		{
			if( !DataMarker::Check( Data, "e" ) )
				break; // Not a valid entry.

			auto& Entry = Sample.Stack[EntryIndex];
			Data >> Entry.Weight;
			Data >> Entry.Mask;
			Data >> Entry.Type;
			Data >> Entry.Time;
			Data >> Entry.PlayRate;
			Data >> Entry.Loop;
			Data >> Entry.Fixed;
			DataString::Decode( Data, Entry.Animation.Name );
		}
	}

	return Data;
}
