// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Animation/Animator.h>
#include <Engine/Sequencer/Sequencer.h>
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
};

struct EventEntity : TrackEvent
{
	void Evaluate( const Timecode& Marker ) override;
	void Execute() override;
	void Reset() override;
	void Context() override;
	void Visualize() override;

	const char* GetName() override;
	const char* GetType() const override;

	void Export( CData& Data ) const override;
	void Import( CData& Data ) override;

protected:
	void FindEntities();

	void StandardOperation();
	void ReplayOperation();

	void PlayRecording();
	void PerformRecording();
	void ClearRecording();

	char Name[2048]{};
	class CMeshEntity* Entity = nullptr;
	bool UpdateEntities = false;

	char Target[2048]{};
	class CPointEntity* TargetEntity = nullptr;

	bool UseTransform = true;
	bool InterpolateLinear = false;
	FTransform TransformA;
	FTransform TransformB;

	bool OverrideAnimation = false;
	std::string Animation;
	float PlayRate = 1.0f;
	bool LoopAnimation = false;

	bool DisplayGizmo = false;

	// For recording an entity's animation and movement.
	bool Record = false;

	// If a recording is available, this allows us to enable it.
	bool UseRecording = false;

	// Recording data.
	Recording Recording;
};
