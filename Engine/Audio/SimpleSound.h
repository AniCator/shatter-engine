// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>

union SoundHandle
{
	size_t Handle;
};

union MusicHandle
{
	size_t Handle;
};

namespace sf
{
	class Sound;
	class SoundBuffer;
	class Music;
}

class CSimpleSound
{
public:
	static SoundHandle Sound( std::string ResourcePath );
	static MusicHandle Music( std::string ResourcePath );

	static void Start( SoundHandle Handle );
	static void Start( MusicHandle Handle );

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

	static void Shutdown();

private:
	static std::vector<sf::Sound*> Sounds;
	static std::vector<sf::SoundBuffer*> SoundBuffers;
	static std::vector<sf::Music*> Streams;
};
