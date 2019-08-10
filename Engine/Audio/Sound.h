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

namespace ESoundType
{
	enum Type
	{
		Unknown = 0,
		Memory,
		Stream
	};
}

class CSound
{
public:
	CSound( ESoundType::Type Type );
	~CSound();

	bool Load( const char* FileLocation );
	bool Load( const std::vector<std::string> Locations );

	void Clear();

	void Start( const float FadeIn = -1.0f );
	void Stop( const float FadeOut = -1.0f );
	void Loop( const bool Loop );
	void Rate( const float Rate );
	bool Playing();
	void Volume( const float Volume );

	void SetPlayMode( ESoundPlayMode::Type NewPlayMode );

private:
	std::vector<SoundBufferHandle> BufferHandles;
	std::vector<SoundHandle> SoundHandles;
	std::vector<StreamHandle> StreamHandles;

	ESoundPlayMode::Type PlayMode;
	ESoundType::Type SoundType;

	SoundBufferHandle Select();
	uint32_t Location;

	bool Loaded;
};
