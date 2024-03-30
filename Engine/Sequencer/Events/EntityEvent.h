// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Sequencer/Sequencer.h>
#include <Engine/Sequencer/Recording.h>
#include <Engine/Utility/Math/Transform.h>

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

	const Recording& GetRecording() const;

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

	// Look up animations for the playback stack.
	bool LookupAnimationStack = false;
};
