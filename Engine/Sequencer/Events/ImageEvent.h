// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Sequencer/Sequencer.h>

struct FEventImage : TrackEvent
{
	void Execute() override;
	void Reset() override {};
	void Context() override;
	const char* GetName() override;
	const char* GetType() const override;
	void Export( CData& Data ) const override;
	void Import( CData& Data ) override;

protected:
	class CTexture* Texture = nullptr;
	std::string Name = "Image";
};