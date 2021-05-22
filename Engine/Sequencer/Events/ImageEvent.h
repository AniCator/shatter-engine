// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Sequencer/Sequencer.h>

struct FEventImage : FTrackEvent
{
	void Execute() override;
	void Reset() override {};
	void Context() override;
	const char* GetName() override;
	const char* GetType() override;
	void Export( CData& Data ) override;
	void Import( CData& Data ) override;

protected:
	class CTexture* Texture = nullptr;
	std::string Name = "Image";
};