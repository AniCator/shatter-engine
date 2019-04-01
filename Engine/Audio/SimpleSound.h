// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>

union SoundHandle
{
	int32_t Handle;
};

union MusicHandle
{
	int32_t Handle;
};

namespace sf
{
	class Sound;
	class SoundBuffer;
	class Music;
}

struct FStream
{
	sf::Music* Stream;
	float FadeDuration;
	float StartTime;
};

class CSimpleSound
{
public:
	static SoundHandle Sound( std::string ResourcePath );
	static MusicHandle Music( std::string ResourcePath );

	static void Start( SoundHandle Handle );
	static void Start( MusicHandle Handle, const float FadeIn = -1.0f );

	static void Stop( SoundHandle Handle );
	static void Stop( MusicHandle Handle );

	static void StopSounds();
	static void StopMusic();
	static void StopAll();

	static void Loop( SoundHandle Handle, const bool Loop );
	static void Loop( MusicHandle Handle, const bool Loop );

	static void Rate( SoundHandle Handle, const float Rate );
	static void Rate( MusicHandle Handle, const float Rate );

	static bool Playing( SoundHandle Handle );
	static bool Playing( MusicHandle Handle );

	static void Volume( SoundHandle Handle, const float Volume );
	static void Volume( MusicHandle Handle, const float Volume );

	static void Tick();

	static void Shutdown();

private:
	static std::vector<sf::Sound*> Sounds;
	static std::vector<sf::SoundBuffer*> SoundBuffers;
	static std::vector<FStream> Streams;
};
