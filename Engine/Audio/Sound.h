// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Audio/SimpleSound.h>

namespace ESoundPlayMode
{
	enum Type
	{
		Unknown = 0,
		Sequential,
		Random
	};
}

class CSound
{
public:
	CSound();
	~CSound();

	bool Load( const char* FileLocation );

	void Clear();

	void Start();
	void Stop();
	void Loop( const bool Loop );
	void Rate( const float Rate );
	bool Playing();
	void Volume( const float Volume );

	void SetPlayMode( ESoundPlayMode::Type NewPlayMode );

private:
	std::vector<SoundBufferHandle> BufferHandles;
	std::vector<SoundHandle> SoundHandles;
	ESoundPlayMode::Type PlayMode;

	SoundBufferHandle Select();
	uint32_t Location;
};
