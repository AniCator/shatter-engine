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

union SoundHandle
{
	int32_t Handle;
};

union SoundBufferHandle
{
	int32_t Handle;
};

union StreamHandle
{
	int32_t Handle;
};

namespace sf
{
	class Sound;
	class SoundBuffer;
	class Music;
}

struct FSound
{
	FSound()
	{
		Voice = nullptr;
		Volume = 100.0f;
		Playing = false;
	}

	sf::Sound* Voice;
	float Volume;
	bool Playing;
};

struct FStream
{
	FStream()
	{
		Stream = nullptr;
		FadeDuration = -1.0f;
		StartTime = -1.0f;
		FadeIn = false;
		Volume = 100.0f;
		Playing = false;
	}

	sf::Music* Stream;
	float FadeDuration;
	float StartTime;
	bool FadeIn;
	float Volume;
	bool Playing;
};

static const size_t MaximumVoices = 256;
struct FVoice
{
	FVoice()
	{
		Voice = nullptr;
		User = nullptr;
		Priority = 0;
		StartTime = 0.0f;
	}

	sf::Sound* Voice;
	FSound* User;
	uint32_t Priority;
	float StartTime;
};

class CSimpleSound
{
public:
	static SoundBufferHandle Sound( const std::string& ResourcePath );
	static StreamHandle Music( const std::string& ResourcePath );

	static SoundHandle Start( SoundBufferHandle Handle );
	static void Start( StreamHandle Handle, const float FadeIn = -1.0f );

	static void Stop( SoundHandle Handle );
	static void Stop( StreamHandle Handle, const float FadeOut = -1.0f );

	static void StopSounds();
	static void StopMusic();
	static void StopAll();

	static void Loop( SoundHandle Handle, const bool Loop );
	static void Loop( StreamHandle Handle, const bool Loop );

	static void Rate( SoundHandle Handle, const float Rate );
	static void Rate( StreamHandle Handle, const float Rate );

	static bool Playing( SoundHandle Handle );
	static bool Playing( StreamHandle Handle );

	static void Volume( SoundHandle Handle, const float Volume );
	static void Volume( StreamHandle Handle, const float Volume );

	static void Volume( const float GlobalVolume );

	static void Tick();

	static void Shutdown();

private:
	static FVoice* GetVoice();
	static FVoice Voices[MaximumVoices];

	static std::deque<FSound> Sounds;
	static std::vector<sf::SoundBuffer*> SoundBuffers;
	static std::vector<FStream> Streams;
	static float GlobalVolume;
};
