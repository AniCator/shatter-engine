// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SimpleSound.h"

#include <ThirdParty/SFML-2.5.1/include/SFML/Audio.hpp>

#include <Engine/Profiling/Logging.h>
#include <Game/Game.h>

std::vector<sf::Sound*> CSimpleSound::Sounds;
std::vector<sf::SoundBuffer*> CSimpleSound::SoundBuffers;
std::vector<FStream> CSimpleSound::Streams;

SoundHandle CSimpleSound::Sound( std::string ResourcePath )
{
	Log::Event( "Loading sound \"%s\"\n", ResourcePath.c_str() );
	sf::SoundBuffer* NewSoundBuffer = new sf::SoundBuffer();
	NewSoundBuffer->loadFromFile( ResourcePath );
	sf::Sound* NewSound = new sf::Sound( *NewSoundBuffer );

	SoundBuffers.push_back( NewSoundBuffer );
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

	FStream Stream;
	Stream.Stream = NewMusic;
	Streams.push_back( Stream );

	MusicHandle Handle;
	Handle.Handle = Streams.size() - 1;
	return Handle;
}

void CSimpleSound::Start( SoundHandle Handle )
{
	if( Handle.Handle > -1 )
		Sounds[Handle.Handle]->play();
}

void CSimpleSound::Start( MusicHandle Handle, const float FadeIn )
{
	if( Handle.Handle < 0 )
		return;

	if( FadeIn > 0.0f )
	{
		Streams[Handle.Handle].Stream->setVolume( 0.0f );
	}

	Streams[Handle.Handle].Stream->play();
	Streams[Handle.Handle].FadeDuration = FadeIn;
	Streams[Handle.Handle].StartTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );
}

void CSimpleSound::Stop( SoundHandle Handle )
{
	if( Handle.Handle > -1 )
		Sounds[Handle.Handle]->stop();
}

void CSimpleSound::Stop( MusicHandle Handle )
{
	if( Handle.Handle > -1 )
		Streams[Handle.Handle].Stream->stop();
}

void CSimpleSound::StopSounds()
{
	for( auto Sound : Sounds )
	{
		Sound->stop();
	}
}

void CSimpleSound::StopMusic()
{
	for( auto Stream : Streams )
	{
		Stream.Stream->stop();
	}
}

void CSimpleSound::StopAll()
{
	StopSounds();
	StopMusic();
}

void CSimpleSound::Loop( SoundHandle Handle, const bool Loop )
{
	Sounds[Handle.Handle]->setLoop(Loop);
}

void CSimpleSound::Loop( MusicHandle Handle, const bool Loop )
{
	Streams[Handle.Handle].Stream->setLoop( Loop );
}

void CSimpleSound::Rate( SoundHandle Handle, const float Rate )
{
	Sounds[Handle.Handle]->setPitch( Rate );
}

void CSimpleSound::Rate( MusicHandle Handle, const float Rate )
{
	Streams[Handle.Handle].Stream->setPitch( Rate );
}

bool CSimpleSound::Playing( SoundHandle Handle )
{
	return Sounds[Handle.Handle]->getStatus() == sf::SoundSource::Status::Playing;
}

bool CSimpleSound::Playing( MusicHandle Handle )
{
	return Streams[Handle.Handle].Stream->getStatus() == sf::SoundSource::Status::Playing;
}

void CSimpleSound::Volume( SoundHandle Handle, const float Volume )
{
	Sounds[Handle.Handle]->setVolume( Volume );
}

void CSimpleSound::Volume( MusicHandle Handle, const float Volume )
{
	Streams[Handle.Handle].Stream->setVolume( Volume );
}

void CSimpleSound::Tick()
{
	const float CurrentTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );
	for( auto Stream : Streams )
	{
		if( Stream.FadeDuration > 0.0f )
		{
			const float DeltaTime = CurrentTime - Stream.StartTime;
			float Alpha = DeltaTime / Stream.FadeDuration;
			Alpha *= Alpha * Alpha;
			if( Alpha < 100.0f && Alpha > 0.0f )
			{
				Stream.Stream->setVolume( Alpha * 100.0f );
			}
		}
	}
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
		delete Stream.Stream;
	}

	Streams.clear();
}
