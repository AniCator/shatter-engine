// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SoLoudSound.h"

#include <ThirdParty/SoLoud/include/soloud.h>
#include <ThirdParty/SoLoud/include/soloud_wav.h>
#include <ThirdParty/SoLoud/include/soloud_wavstream.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/File.h>

#undef GetCurrentTime
#include <Game/Game.h>

std::deque<FSound> CSoLoudSound::Sounds;
std::vector<SoLoud::Wav*> CSoLoudSound::SoundBuffers;
std::vector<SoLoud::WavStream*> CSoLoudSound::StreamBuffers;
std::vector<FStream> CSoLoudSound::Streams;
float CSoLoudSound::GlobalVolume = 1.0f;

SoLoud::Soloud Engine;

SoundBufferHandle CSoLoudSound::Sound( const std::string& ResourcePath )
{
	if( CFile::Exists( ResourcePath.c_str() ) )
	{
		auto* NewSoundBuffer = new SoLoud::Wav();
		if( NewSoundBuffer->load( ResourcePath.c_str() ) == 0 )
		{
			SoundBuffers.emplace_back( NewSoundBuffer );
			SoundBufferHandle Handle;
			Handle.Handle = SoundBuffers.size() - 1;
			return Handle;
		}
	}
	
	Log::Event( Log::Warning, "Failed to load sound \"%s\"\n", ResourcePath.c_str() );
	return EmptyHandle<SoundBufferHandle>();
}

StreamHandle CSoLoudSound::Music( const std::string& ResourcePath )
{
	if( CFile::Exists( ResourcePath.c_str() ) )
	{
		auto* NewStreamBuffer = new SoLoud::WavStream();
		if( NewStreamBuffer->load( ResourcePath.c_str() ) == 0 )
		{
			StreamBuffers.emplace_back( NewStreamBuffer );
			StreamHandle Handle;
			Handle.Handle = StreamBuffers.size() - 1;
			return Handle;
		}
	}
	
	Log::Event( Log::Warning, "Failed to open stream \"%s\"\n", ResourcePath.c_str() );
	return EmptyHandle<StreamHandle>();
}

SoundHandle CSoLoudSound::Start( SoundBufferHandle Handle )
{
	if( Handle.Handle < SoundBuffers.size() )
	{
		auto& AudioSource = *SoundBuffers[Handle.Handle];

		FSound NewSound;
		NewSound.Voice = Engine.play( AudioSource );
		NewSound.Playing = true;
		NewSound.Buffer = Handle;

		Sounds.emplace_back( NewSound );
		
		SoundHandle Handle;
		Handle.Handle = Sounds.size() - 1;
		return Handle;
	}
	
	return EmptyHandle<SoundHandle>();
}

StreamHandle CSoLoudSound::Start( StreamHandle Handle, const float FadeIn )
{
	if( GlobalVolume == 0.0f )
		return EmptyHandle<StreamHandle>();

	if( Handle.Handle == InvalidHandle )
		return EmptyHandle<StreamHandle>();

	if( Handle.Handle < StreamBuffers.size() )
	{
		auto& AudioSource = *StreamBuffers[Handle.Handle];
		
		FStream NewStream;
		NewStream.Playing = true;
		NewStream.FadeDuration = FadeIn;
		NewStream.FadeIn = FadeIn > 0.0f;
		NewStream.StartTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );
		NewStream.Buffer = Handle;

		NewStream.Stream = Engine.play( AudioSource, NewStream.FadeIn ? 0.0f : -1.0f );
		Streams.emplace_back( NewStream );

		StreamHandle Handle;
		Handle.Handle = Streams.size() - 1;
		return Handle;
	}

	return EmptyHandle<StreamHandle>();
}

void CSoLoudSound::Stop( SoundHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.stop( Sounds[Handle.Handle].Voice );
	}
}

void CSoLoudSound::Stop( StreamHandle Handle, const float FadeOut )
{
	if( Handle.Handle > InvalidHandle )
	{
		if( FadeOut < 0.0f )
		{
			Engine.stop( Streams[Handle.Handle].Stream );
		}
		else
		{
			if( Streams[Handle.Handle].FadeIn )
			{
				Streams[Handle.Handle].Volume = Engine.getVolume( Streams[Handle.Handle].Stream );
			}

			Streams[Handle.Handle].FadeDuration = FadeOut;
			Streams[Handle.Handle].FadeIn = false;
			Streams[Handle.Handle].StartTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );
		}
	}
}

void CSoLoudSound::StopSounds()
{
	for( const auto& Sound : Sounds )
	{
		Engine.stop( Sound.Voice );
	}
}

void CSoLoudSound::StopMusic()
{
	for( const auto& Stream : Streams )
	{
		Engine.stop( Stream.Stream );
	}
}

void CSoLoudSound::StopAll()
{
	StopSounds();
	StopMusic();
}

void CSoLoudSound::Loop( SoundHandle Handle, const bool Loop )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.setLooping( Sounds[Handle.Handle].Voice, Loop );
	}
}

void CSoLoudSound::Loop( StreamHandle Handle, const bool Loop )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.setLooping( Streams[Handle.Handle].Stream, Loop );
	}
}

void CSoLoudSound::Rate( SoundHandle Handle, const float Rate )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.setRelativePlaySpeed( Sounds[Handle.Handle].Voice, Rate );
	}
}

void CSoLoudSound::Rate( StreamHandle Handle, const float Rate )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.setRelativePlaySpeed( Streams[Handle.Handle].Stream, Rate );
	}
}

float CSoLoudSound::Time( SoundHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return Engine.getStreamTime( Sounds[Handle.Handle].Voice );
	}
	
	return 0.0f;
}

float CSoLoudSound::Time( StreamHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return Engine.getStreamTime( Streams[Handle.Handle].Stream );
	}
	
	return 0.0f;
}

float CSoLoudSound::Length( SoundHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return SoundBuffers[Sounds[Handle.Handle].Buffer.Handle]->getLength();
	}
	
	return -1.0f;
}

float CSoLoudSound::Length( StreamHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return StreamBuffers[Streams[Handle.Handle].Buffer.Handle]->getLength();
	}
	
	return -1.0f;
}

void CSoLoudSound::Offset( SoundHandle Handle, const float Offset )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.seek( Sounds[Handle.Handle].Voice, Offset );
	}
}

void CSoLoudSound::Offset( StreamHandle Handle, const float Offset )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.seek( Streams[Handle.Handle].Stream, Offset );
	}
}

bool CSoLoudSound::Playing( SoundHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return Engine.isValidVoiceHandle( Sounds[Handle.Handle].Voice );
	}
	
	return false;
}

bool CSoLoudSound::Playing( StreamHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return Engine.isValidVoiceHandle( Streams[Handle.Handle].Stream );
	}
	
	return false;
}

void CSoLoudSound::Volume( SoundHandle Handle, const float Volume )
{
	Sounds[Handle.Handle].Volume = Volume * 0.01f;
}

void CSoLoudSound::Volume( StreamHandle Handle, const float Volume )
{
	Streams[Handle.Handle].Volume = Volume * 0.01f;
}

void CSoLoudSound::Volume( const float GlobalVolumeIn )
{
	Engine.setGlobalVolume( GlobalVolumeIn * 0.01f );
}

void CSoLoudSound::Tick()
{
	Profile( "Sound" );

	CProfiler& Profiler = CProfiler::Get();
	// Profiler.AddCounterEntry( FProfileTimeEntry( "Sound Buffers", Sounds.size() ), true );
	// Profiler.AddCounterEntry( FProfileTimeEntry( "Stream Buffers", Streams.size() ), true );

	const auto CurrentTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );

	size_t ActiveStreams = 0;
	size_t StreamIndex = 0;
	for( auto& Stream : Streams )
	{
		StreamIndex++;
		
		if( Stream.Stream && Stream.Playing )
		{
			ActiveStreams++;
			
			StreamHandle Handle;
			Handle.Handle = StreamIndex - 1;
			Stream.Playing = Playing( Handle );

			if( Stream.FadeDuration > 0.0f )
			{
				if( Stream.FadeIn )
				{
					const float DeltaTime = CurrentTime - Stream.StartTime;
					float Alpha = DeltaTime / Stream.FadeDuration;
					Alpha *= Alpha * Alpha;
					if( Alpha < 1.0f && Alpha > 0.0f )
					{
						Engine.setVolume( Stream.Stream, Alpha * Stream.Volume );
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
						Engine.setVolume( Stream.Stream, Alpha * Stream.Volume );
					}
					else if( Alpha < 0.0f )
					{
						Engine.stop( Stream.Stream );
						Stream.Playing = false;
					}
				}
			}
			else
			{
				Engine.setVolume( Stream.Stream, Stream.Volume );
			}
		}
	}

	size_t ActiveSounds = 0;
	size_t SoundIndex = 0;
	for( auto& Sound : Sounds )
	{
		SoundIndex++;
		if( Sound.Voice && Sound.Playing )
		{
			ActiveSounds++;

			if( ActiveSounds > 200 )
			{
				Engine.stop( Sound.Voice );
				Sound.Playing = false;
				continue;
			}

			SoundHandle Handle;
			Handle.Handle = SoundIndex - 1;
			Sound.Playing = Playing( Handle );
			Engine.setVolume( Sound.Voice, Sound.Volume );
		}
	}

	Profiler.AddCounterEntry( FProfileTimeEntry( "Active Sounds", ActiveSounds ), true );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Active Streams", ActiveStreams ), true );
}

void CSoLoudSound::Initialize()
{
	Engine.init();
}

void CSoLoudSound::Shutdown()
{
	StopAll();
	Sounds.clear();
	Streams.clear();
	Engine.deinit();
}
