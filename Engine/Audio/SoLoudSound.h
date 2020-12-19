// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>
#include <deque>

static const int32_t InvalidHandle = -1;

template<typename HandleType>
HandleType EmptyHandle()
{
	HandleType EmptyHandle;
	EmptyHandle.Handle = -1;
	return EmptyHandle;
}

// Matches soloud.h
typedef unsigned int SoLoudHandle;

union SoundHandle
{
	int32_t Handle = -1;
};

union SoundBufferHandle
{
	int32_t Handle = -1;
};

union StreamHandle
{
	int32_t Handle = -1;
};

namespace SoLoud
{
	class Wav;
	class WavStream;
}

struct FSound
{
	SoLoudHandle Voice = 0;
	SoundBufferHandle Buffer;
	float Volume = 1.0f;
	bool Playing = false;
};

struct FStream
{
	SoLoudHandle Stream = 0;
	StreamHandle Buffer;
	float FadeDuration = -1.0f;
	float StartTime = -1.0f;
	bool FadeIn = false;
	float Volume = 1.0f;
	bool Playing = false;
};

class CSoLoudSound
{
public:
	static SoundBufferHandle Sound( const std::string& ResourcePath );
	static StreamHandle Music( const std::string& ResourcePath );

	static SoundHandle Start( SoundBufferHandle Handle );
	static StreamHandle Start( StreamHandle Handle, const float FadeIn = -1.0f );

	static void Stop( SoundHandle Handle );
	static void Stop( StreamHandle Handle, const float FadeOut = -1.0f );

	static void StopSounds();
	static void StopMusic();
	static void StopAll();

	static void Loop( SoundHandle Handle, const bool Loop );
	static void Loop( StreamHandle Handle, const bool Loop );

	static void Rate( SoundHandle Handle, const float Rate );
	static void Rate( StreamHandle Handle, const float Rate );

	static float Time( SoundHandle Handle );
	static float Time( StreamHandle Handle );

	static float Length( SoundHandle Handle );
	static float Length( StreamHandle Handle );

	static void Offset( SoundHandle Handle, const float Offset );
	static void Offset( StreamHandle Handle, const float Offset );

	static bool Playing( SoundHandle Handle );
	static bool Playing( StreamHandle Handle );

	static void Volume( SoundHandle Handle, const float Volume );
	static void Volume( StreamHandle Handle, const float Volume );

	static void Volume( const float GlobalVolume );

	static void Tick();

	static void Initialize();
	static void Shutdown();

private:
	static std::deque<FSound> Sounds;
	static std::vector<SoLoud::Wav*> SoundBuffers;
	static std::vector<SoLoud::WavStream*> StreamBuffers;
	static std::vector<FStream> Streams;
	static float GlobalVolume;
};
