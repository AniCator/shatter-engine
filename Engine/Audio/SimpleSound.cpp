// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SimpleSound.h"

#include <ThirdParty/SFML-2.5.1/include/SFML/Audio.hpp>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Game/Game.h>

std::vector<FSound> CSimpleSound::Sounds;
std::vector<sf::SoundBuffer*> CSimpleSound::SoundBuffers;
std::vector<FStream> CSimpleSound::Streams;
float CSimpleSound::GlobalVolume = 1.0f;

SoundBufferHandle CSimpleSound::Sound( std::string ResourcePath )
{
	Log::Event( "Loading sound \"%s\"\n", ResourcePath.c_str() );
	sf::SoundBuffer* NewSoundBuffer = new sf::SoundBuffer();
	NewSoundBuffer->loadFromFile( ResourcePath );
	// sf::Sound* NewSound = new sf::Sound( *NewSoundBuffer );

	SoundBuffers.push_back( NewSoundBuffer );
	// Sounds.push_back( NewSound );

	SoundBufferHandle Handle;
	Handle.Handle = SoundBuffers.size() - 1;
	return Handle;
}

StreamHandle CSimpleSound::Music( std::string ResourcePath )
{
	Log::Event( "Opening stream \"%s\"\n", ResourcePath.c_str() );
	sf::Music* NewMusic = new sf::Music();
	NewMusic->openFromFile( ResourcePath );

	FStream Stream;
	Stream.Stream = NewMusic;
	Streams.push_back( Stream );

	StreamHandle Handle;
	Handle.Handle = Streams.size() - 1;
	return Handle;
}

SoundHandle CSimpleSound::Start( SoundBufferHandle Handle )
{
	if( Handle.Handle > -1 )
	{
		FSound NewSound;
		NewSound.Sound = new sf::Sound( *SoundBuffers[Handle.Handle] );
		Sounds.push_back( NewSound );
		NewSound.Sound->setVolume( NewSound.Volume * GlobalVolume );
		NewSound.Sound->play();

		SoundHandle NewHandle;
		NewHandle.Handle = Sounds.size() - 1;
		return NewHandle;
	}

	SoundHandle EmptyHandle;
	EmptyHandle.Handle = -1;
	return EmptyHandle;
}

void CSimpleSound::Start( StreamHandle Handle, const float FadeIn )
{
	if( Handle.Handle < 0 )
		return;

	if( FadeIn > 0.0f )
	{
		Streams[Handle.Handle].Stream->setVolume( 0.0f );
	}
	else
	{
		Streams[Handle.Handle].Stream->setVolume( Streams[Handle.Handle].Volume * GlobalVolume );
	}

	Streams[Handle.Handle].Stream->play();
	Streams[Handle.Handle].FadeDuration = FadeIn;
	Streams[Handle.Handle].FadeIn = true;
	Streams[Handle.Handle].StartTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );
}

void CSimpleSound::Stop( SoundHandle Handle )
{
	if( Handle.Handle > -1 )
		Sounds[Handle.Handle].Sound->stop();
}

void CSimpleSound::Stop( StreamHandle Handle, const float FadeOut )
{
	if( Handle.Handle > -1 )
	{
		if( FadeOut < 0.0f )
		{
			Streams[Handle.Handle].Stream->stop();
		}
		else
		{
			Streams[Handle.Handle].FadeDuration = FadeOut;
			Streams[Handle.Handle].FadeIn = false;
			Streams[Handle.Handle].StartTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );
		}
	}
}

void CSimpleSound::StopSounds()
{
	for( auto Sound : Sounds )
	{
		Sound.Sound->stop();
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
	Sounds[Handle.Handle].Sound->setLoop(Loop);
}

void CSimpleSound::Loop( StreamHandle Handle, const bool Loop )
{
	Streams[Handle.Handle].Stream->setLoop( Loop );
}

void CSimpleSound::Rate( SoundHandle Handle, const float Rate )
{
	Sounds[Handle.Handle].Sound->setPitch( Rate );
}

void CSimpleSound::Rate( StreamHandle Handle, const float Rate )
{
	Streams[Handle.Handle].Stream->setPitch( Rate );
}

bool CSimpleSound::Playing( SoundHandle Handle )
{
	return Sounds[Handle.Handle].Sound->getStatus() == sf::SoundSource::Status::Playing;
}

bool CSimpleSound::Playing( StreamHandle Handle )
{
	return Streams[Handle.Handle].Stream->getStatus() == sf::SoundSource::Status::Playing;
}

void CSimpleSound::Volume( SoundHandle Handle, const float Volume )
{
	Sounds[Handle.Handle].Volume = Volume;
}

void CSimpleSound::Volume( StreamHandle Handle, const float Volume )
{
	Streams[Handle.Handle].Volume = Volume;
}

void CSimpleSound::Volume( const float GlobalVolumeIn )
{
	GlobalVolume = GlobalVolumeIn * 0.01f;
}

void CSimpleSound::Tick()
{
	Profile( "Sound" );

	const float CurrentTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );
	for( auto Stream : Streams )
	{
		if( Stream.FadeDuration > 0.0f )
		{
			if( Stream.FadeIn )
			{
				const float DeltaTime = CurrentTime - Stream.StartTime;
				float Alpha = DeltaTime / Stream.FadeDuration;
				Alpha *= Alpha * Alpha;
				if( Alpha < 1.0f && Alpha > 0.0f )
				{
					Stream.Stream->setVolume( Alpha * Stream.Volume * GlobalVolume );
				}
			}
			else
			{
				const float DeltaTime = CurrentTime - Stream.StartTime;
				float Alpha = DeltaTime / Stream.FadeDuration;
				// Alpha *= Alpha * Alpha;
				Alpha = 1.0f - Alpha;
				if( Alpha < 1.0f && Alpha > 0.0f )
				{
					Stream.Stream->setVolume( Alpha * Stream.Volume * GlobalVolume );
				}
				else if( Alpha < 0.0f )
				{
					Stream.Stream->stop();
				}
			}
		}
		else
		{
			Stream.Stream->setVolume( Stream.Volume * GlobalVolume );
		}
	}

	for( auto Sound : Sounds )
	{
		Sound.Sound->setVolume( Sound.Volume * GlobalVolume );
	}
}

void CSimpleSound::Shutdown()
{
	for( auto Sound : Sounds )
	{
		delete Sound.Sound;
	}

	Sounds.clear();

	for( auto Stream : Streams )
	{
		delete Stream.Stream;
	}

	Streams.clear();
}
