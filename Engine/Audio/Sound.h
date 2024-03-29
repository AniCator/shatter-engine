// Copyright � 2017, Christiaan Bakker, All rights reserved.
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

	void Clear( const bool& Unload = false );

	int32_t Start( const Spatial& Information = Spatial() );
	void Stop( const float& FadeOut = -1.0f );
	void Loop( const bool& Loop );
	void Rate( const float& Rate );
	double Time() const;
	double Length() const;
	void Offset( const double& Offset );
	bool Playing();
	void Volume( const float& Volume );
	void Fade( const float& Volume, const float& Time );

	void SetPlayMode( const ESoundPlayMode::Type& NewPlayMode );

	void Update( const int32_t& Handle, const Vector3D& Position, const Vector3D& Velocity );

	SoundBufferHandle GetSoundHandle()
	{
		return Select();
	}

	StreamHandle GetStreamHandle()
	{
		auto Handle = EmptyHandle<StreamHandle>();
		return StreamHandles.empty() ? Handle : StreamHandles[0];
	}

	std::vector<SoundBufferHandle> GetSoundBufferHandles() const
	{
		return BufferHandles;
	}

	std::vector<StreamHandle> GetStreamBufferHandles() const
	{
		return StreamBufferHandles;
	}

	std::vector<SoundHandle> GetSoundHandles() const
	{
		return SoundHandles;
	}
	
	std::vector<StreamHandle> GetStreamHandles() const
	{
		return StreamHandles;
	}

	ESoundType::Type GetSoundType() const
	{
		return SoundType;
	}

	std::string FileLocation;
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
