// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Animation/Animator.h>
#include <Engine/Utility/Math/Transform.h>

constexpr uint8_t MaximumRecordingStackSize = 64;
struct Recording
{
	struct Sample
	{
		double Time = -1.0;
		FTransform Transform;
		Animator::BlendEntry Stack[MaximumRecordingStackSize];
		uint8_t Entries = 0;
	};

	// Recorded samples.
	std::vector<Sample> Samples;

	void Wipe()
	{
		Samples.clear();
	}

	// Returns true if the recording contains samples.
	bool Taped() const
	{
		return !Samples.empty();
	}

	void Apply( class CMeshEntity* Entity, const double Time );
	void Apply( class CRenderable& Renderable, Animator::Instance& Instance, const double Time );

	friend CData& operator<<( CData& Data, const Recording& Recording );
	friend CData& operator>>( CData& Data, Recording& Recording );
};
