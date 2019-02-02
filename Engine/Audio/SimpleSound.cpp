// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SimpleSound.h"

#include <ThirdParty/SFML-2.5.1/include/SFML/Audio.hpp>

#include <Engine/Profiling/Logging.h>

std::vector<sf::Sound*> CSimpleSound::Sounds;
std::vector<sf::Music*> CSimpleSound::Streams;

SoundHandle CSimpleSound::Sound( std::string ResourcePath )
{
	Log::Event( "Loading sound \"%s\"", ResourcePath.c_str() );
	sf::SoundBuffer NewSoundBuffer;
	NewSoundBuffer.loadFromFile( ResourcePath );
	sf::Sound* NewSound = new sf::Sound( NewSoundBuffer );

	Sounds.push_back( NewSound );

	SoundHandle Handle;
	Handle.Handle = Sounds.size() - 1;
	return Handle;
}

MusicHandle CSimpleSound::Music( std::string ResourcePath )
{
	Log::Event( "Opening stream \"%s\"\n", ResourcePath.c_str() );
	sf::Music* NewMusic = new sf::Music();
	NewMusic->openFromFile( ResourcePath );

	Streams.push_back( NewMusic );

	MusicHandle Handle;
	Handle.Handle = Streams.size() - 1;
	return Handle;
}

void CSimpleSound::Start( SoundHandle Handle )
{
	Sounds[Handle.Handle]->play();
}

void CSimpleSound::Start( MusicHandle Handle )
{
	Streams[Handle.Handle]->play();
}

void CSimpleSound::Stop( SoundHandle Handle )
{
	Sounds[Handle.Handle]->stop();
}

void CSimpleSound::Stop( MusicHandle Handle )
{
	Streams[Handle.Handle]->stop();
}

void CSimpleSound::Loop( SoundHandle Handle, const bool Loop )
{
	Sounds[Handle.Handle]->setLoop(Loop);
}

void CSimpleSound::Loop( MusicHandle Handle, const bool Loop )
{
	Streams[Handle.Handle]->setLoop( Loop );
}

void CSimpleSound::Rate( SoundHandle Handle, const float Rate )
{
	Sounds[Handle.Handle]->setPitch( Rate );
}

void CSimpleSound::Rate( MusicHandle Handle, const float Rate )
{
	Streams[Handle.Handle]->setPitch( Rate );
}

void CSimpleSound::Shutdown()
{
	for( auto Sound : Sounds )
	{
		delete Sound;
	}

	Sounds.clear();

	for( auto Stream : Streams )
	{
		delete Stream;
	}

	Streams.clear();
}
