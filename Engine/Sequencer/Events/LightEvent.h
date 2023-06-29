// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/World/Entity/LightEntity/LightEntity.h>
#include <Engine/Sequencer/Sequencer.h>

struct LightEvent : TrackEvent
{
	void Evaluate( const Timecode& Marker ) override;
	void Execute() override;
	void Reset() override {};
	void Context() override;
	const char* GetName() override;
	const char* GetType() const override;
	void Export( CData& Data ) const override;
	void Import( CData& Data ) override;

protected:
	Light Information;
	int32_t LightIndex = -1;
};