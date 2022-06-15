// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Sequencer/Sequencer.h>
#include <Engine/Utility/Math/Transform.h>

struct EventEntity : TrackEvent
{
	void Evaluate( const Timecode& Marker ) override;
	void Execute() override;
	void Reset() override;
	void Context() override;

	const char* GetName() override;
	const char* GetType() const override;

	void Export( CData& Data ) const override;
	void Import( CData& Data ) override;

protected:
	void FindEntities();

	char Name[2048]{};
	class CMeshEntity* Entity = nullptr;

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
};
