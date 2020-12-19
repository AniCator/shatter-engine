// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Audio/SoLoudSound.h>

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

namespace EFalloff
{
	enum Type
	{
		None = 0,
		Linear,
		InverseSquare
	};
}

class CSound
{
public:
	CSound( ESoundType::Type Type );
	~CSound();

	bool Load( const char* FileLocation );
	bool Load( const std::vector<std::string>& Locations );

	void Clear();

	void Start( const float FadeIn = -1.0f );
	void Stop( const float FadeOut = -1.0f );
	void Loop( const bool Loop );
	void Rate( const float Rate );
	float Time() const;
	float Length() const;
	void Offset( const float Offset );
	bool Playing();
	void Volume( const float Volume );

	void SetPlayMode( ESoundPlayMode::Type NewPlayMode );

private:
	std::vector<SoundBufferHandle> BufferHandles;
	std::vector<StreamHandle> StreamBufferHandles;
	std::vector<SoundHandle> SoundHandles;
	std::vector<StreamHandle> StreamHandles;

	ESoundPlayMode::Type PlayMode;
	ESoundType::Type SoundType;

	SoundBufferHandle Select();
	uint32_t Location;

	bool Loaded;
};
