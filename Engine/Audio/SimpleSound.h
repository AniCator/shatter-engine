// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>

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
		Sound = nullptr;
		Volume = 100.0f;
		Playing = false;
	}

	sf::Sound* Sound;
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

class CSimpleSound
{
public:
	static SoundBufferHandle Sound( std::string ResourcePath );
	static StreamHandle Music( std::string ResourcePath );

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
	static std::vector<FSound> Sounds;
	static std::vector<sf::SoundBuffer*> SoundBuffers;
	static std::vector<FStream> Streams;
	static float GlobalVolume;
};
