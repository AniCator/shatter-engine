// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SoLoudSound.h"

#include <ThirdParty/SoLoud/include/soloud.h>
#include <ThirdParty/SoLoud/include/soloud_wav.h>
#include <ThirdParty/SoLoud/include/soloud_wavstream.h>
#include <ThirdParty/SoLoud/include/soloud_speech.h>
#include <ThirdParty/SoLoud/include/soloud_freeverbfilter.h>
#include <ThirdParty/SoLoud/include/soloud_echofilter.h>

#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#include <Engine/Physics/Body/Body.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>

#undef GetCurrentTime
#include <Game/Game.h>

std::deque<FSound> CSoLoudSound::Sounds;
std::vector<SoLoud::Wav*> CSoLoudSound::SoundBuffers;
std::vector<SoLoud::WavStream*> CSoLoudSound::StreamBuffers;
std::vector<FStream> CSoLoudSound::Streams;
float CSoLoudSound::GlobalVolume = 1.0f;

SoLoud::Soloud Engine;
SoLoud::FreeverbFilter ReverbEarlyReflection;
SoLoud::FreeverbFilter ReverbTail;
SoLoud::EchoFilter Echo;

SoLoud::Bus Mixer[Bus::Maximum];
SoLoud::handle MixerHandles[Bus::Maximum];

struct ReverbParameters
{
	float Freeze = 0.0f;
	float RoomSize = 0.5f;
	float Dampening = 0.5f;
	float Width = 1.0f;
	float Wet = 1.0f;
};

struct DelayParameters
{
	float Wet = 1.0f;
	float Delay = 0.01f;
	float Decay = 0.3f;
	float Filter = 0.1f;
};

Spatial Spatial::Create( CMeshEntity* Entity )
{
	Spatial Information;
	if( !Entity )
		return Information;

	const auto* Body = Entity->GetBody();
	if( !Body )
		return Information;

	Information = Create( Body->GetTransform().GetPosition(), Body->Velocity );
	return Information;
}

void Configure3DSound( SoLoud::handle Handle, const Spatial& Information )
{
	Engine.set3dSourceAttenuation( Handle, Information.Attenuation, Information.Rolloff );
	Engine.set3dSourceMinMaxDistance( Handle, Information.MinimumDistance, Information.MaximumDistance );
	Engine.set3dSourceDopplerFactor( Handle, Information.Doppler );
	Engine.setInaudibleBehavior( Handle, false, true );
}

SoLoud::handle PlaySound( SoLoud::AudioSource& AudioSource, const Spatial& Information )
{
	SoLoud::handle Handle;
	unsigned int SelectedBus = Information.Bus;
	if( SelectedBus >= Bus::Maximum )
	{
		SelectedBus = Bus::SFX;
	}

	const bool FadeIn = Information.FadeIn > 0.0f;
	
	if( Information.Is3D )
	{
		Handle = Mixer[SelectedBus].play3d( AudioSource,
				Information.Position.X, Information.Position.Y, Information.Position.Z,
				Information.Velocity.X, Information.Velocity.Y, Information.Velocity.Z,
				FadeIn ? 0.0f : Information.Volume * 0.01f, Information.StartPaused );

		Configure3DSound( Handle, Information );

		if( Information.DelayByDistance )
		{
			AudioSource.set3dDistanceDelay( true );
		}
	}
	else
	{
		Handle = Mixer[SelectedBus].play( AudioSource, Information.Volume * 0.01f, 0.0f, Information.StartPaused );
	}

	Engine.setRelativePlaySpeed( Handle, Information.Rate );

	return Handle;
}

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

SoLoud::Speech Speech;
void CSoLoudSound::Speak( const std::string& Sentence, const Spatial Information )
{
	Speech.setText( Sentence.c_str() );
	Speech.setParams( Math::RandomRangeInteger( 2000, 3000 ), 5.0f );

	unsigned int SelectedBus = Information.Bus;
	if( SelectedBus >= Bus::Maximum )
	{
		SelectedBus = Bus::SFX;
	}
	
	Mixer[SelectedBus].play( Speech, 2.0f );
}

SoundHandle CSoLoudSound::Start( SoundBufferHandle Handle, const Spatial Information )
{
	if( Handle.Handle < SoundBuffers.size() )
	{
		auto& AudioSource = *SoundBuffers[Handle.Handle];

		unsigned int SelectedBus = Information.Bus;
		if( SelectedBus >= Bus::Maximum )
		{
			SelectedBus = Bus::SFX;
		}

		const bool FadeIn = Information.FadeIn > 0.0f;
		FSound NewSound;
		NewSound.Voice = PlaySound( AudioSource, Information );
		
		NewSound.Playing = true;
		NewSound.Buffer = Handle;

		Sounds.emplace_back( NewSound );
		
		SoundHandle Handle;
		Handle.Handle = Sounds.size() - 1;

		if( FadeIn )
		{
			Fade( Handle, 100.0f, Information.FadeIn );
		}
		
		return Handle;
	}
	
	return EmptyHandle<SoundHandle>();
}

StreamHandle CSoLoudSound::Start( StreamHandle Handle, const Spatial Information )
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
		NewStream.FadeDuration = Information.FadeIn;
		NewStream.FadeIn = Information.FadeIn > 0.0f;
		NewStream.StartTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );
		NewStream.Buffer = Handle;

		NewStream.Stream = PlaySound( AudioSource, Information );

		Streams.emplace_back( NewStream );

		// Always tick inaudible streams.
		Engine.setInaudibleBehavior( NewStream.Stream, true, false );

		StreamHandle Handle;
		Handle.Handle = Streams.size() - 1;

		if( NewStream.FadeIn )
		{
			Fade( Handle, 100.0f, Information.FadeIn );
		}
		
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

			Fade( Handle, 0.0f, FadeOut );
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
	Engine.setVolume( Sounds[Handle.Handle].Voice, Volume * 0.01f );
}

void CSoLoudSound::Volume( StreamHandle Handle, const float Volume )
{
	Streams[Handle.Handle].Volume = Volume * 0.01f;
	Engine.setVolume( Streams[Handle.Handle].Stream, Volume * 0.01f );
}

void CSoLoudSound::Fade( SoundHandle Handle, const float Volume, const float Time )
{
	Engine.fadeVolume( Sounds[Handle.Handle].Voice, Volume * 0.01f, Time );
}

void CSoLoudSound::Fade( StreamHandle Handle, const float Volume, const float Time )
{
	Engine.fadeVolume( Streams[Handle.Handle].Stream, Volume * 0.01f, Time );
}

SoLoud::handle CreateGroup( const std::vector<StreamHandle>& Handles )
{
	const auto VoiceGroup = Engine.createVoiceGroup();
	for( const auto& Handle : Handles )
	{
		Engine.addVoiceToGroup( VoiceGroup, Handle.Handle );
	}

	return VoiceGroup;
}

void DeleteGroup( const SoLoud::handle& VoiceGroup )
{
	Engine.destroyVoiceGroup( VoiceGroup );
}

void CSoLoudSound::GroupPause( const std::vector<StreamHandle>& Handles, const bool State )
{
	const auto PauseGroup = CreateGroup( Handles );
	Engine.setPause( PauseGroup, State );
	DeleteGroup( PauseGroup );
}

void CSoLoudSound::GroupProtect( const std::vector<StreamHandle>& Handles, const bool State )
{
	const auto ProtectGroup = CreateGroup( Handles );
	Engine.setProtectVoice( ProtectGroup, State );
	DeleteGroup( ProtectGroup );
}

void CSoLoudSound::SetListenerPosition( const Vector3D& Position )
{
	Engine.set3dListenerPosition( Position.X, Position.Y, Position.Z );
}

void CSoLoudSound::SetListenerDirection( const Vector3D& Direction )
{
	Engine.set3dListenerAt( Direction.X, Direction.Y, Direction.Z );
}

void CSoLoudSound::SetListenerUpDirection( const Vector3D& Direction )
{
	Engine.set3dListenerUp( Direction.X, Direction.Y, Direction.Z );
}

void CSoLoudSound::SetListenerVelocity( const Vector3D& Velocity )
{
	Engine.set3dListenerVelocity( Velocity.X, Velocity.Y, Velocity.Z );
}

void CSoLoudSound::Update( SoundHandle Handle, const Vector3D& Position, const Vector3D& Velocity )
{
	Engine.set3dSourceParameters( Sounds[Handle.Handle].Voice,
		Position.X, Position.Y, Position.Z,
		Velocity.X, Velocity.Y, Velocity.Z
	);
}

void CSoLoudSound::Update( StreamHandle Handle, const Vector3D& Position, const Vector3D& Velocity )
{
	Engine.set3dSourceParameters( Streams[Handle.Handle].Stream,
		Position.X, Position.Y, Position.Z,
		Velocity.X, Velocity.Y, Velocity.Z
	);
}

void CSoLoudSound::Volume( const float GlobalVolumeIn )
{
	Engine.setGlobalVolume( GlobalVolumeIn * 0.01f );
}

void CSoLoudSound::Tick()
{
	Profile( "Sound" );

	CProfiler& Profiler = CProfiler::Get();
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

	DelayParameters DelayParameter;
	DelayParameter.Wet = 0.12f;
	DelayParameter.Delay = 0.343f;
	DelayParameter.Decay = 0.25f;
	DelayParameter.Filter = 0.1f;

	Echo.getParamName( 0 );
	
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 1, 0, DelayParameter.Wet );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 1, 1, DelayParameter.Delay );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 1, 2, DelayParameter.Decay );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 1, 3, DelayParameter.Filter );

	ReverbParameters Parameters;
	Parameters.Wet = 0.03f;
	Parameters.Freeze = 0.0f;
	Parameters.RoomSize = 0.2f;
	Parameters.Dampening = 0.7f;
	Parameters.Width = 0.5f;
	
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 0, Parameters.Wet );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 1, Parameters.Freeze );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 2, Parameters.RoomSize );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 3, Parameters.Dampening );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 4, Parameters.Width );

	Parameters.Wet = 0.003f;
	Parameters.Freeze = 0.0f;
	Parameters.RoomSize = 1.0f;
	Parameters.Dampening = 0.0f;
	Parameters.Width = 1.0f;

	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 0, Parameters.Wet );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 1, Parameters.Freeze );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 2, Parameters.RoomSize );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 3, Parameters.Dampening );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 4, Parameters.Width );

	Engine.update3dAudio();

	const auto SoundEntry = FProfileTimeEntry( "Active Sounds", ActiveSounds );
	Profiler.AddCounterEntry( SoundEntry, true );

	const auto StreamEntry = FProfileTimeEntry( "Active Streams", ActiveStreams );
	Profiler.AddCounterEntry( StreamEntry, true );

	const auto VoiceEntry = FProfileTimeEntry( "Active Voices", Engine.getActiveVoiceCount() );
	Profiler.AddCounterEntry( VoiceEntry, true );
}

void CSoLoudSound::Initialize()
{
	Engine.init();

	for( SoLoud::handle Handle = 0; Handle < Bus::Maximum; Handle++ )
	{
		MixerHandles[Handle] = Engine.play( Mixer[Handle] );
	}

	Mixer[Bus::SFX].setFilter( 1, &Echo );

	DelayParameters DelayParameter;
	DelayParameter.Wet = 0.12f;
	DelayParameter.Delay = 0.343f;
	DelayParameter.Decay = 0.25f;
	DelayParameter.Filter = 0.1f;
	
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 1, 0, DelayParameter.Wet );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 1, 1, DelayParameter.Delay );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 1, 2, DelayParameter.Decay );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 1, 3, DelayParameter.Filter );

	Mixer[Bus::SFX].setFilter( 2, &ReverbEarlyReflection );

	ReverbParameters Parameters;
	Parameters.Wet = 0.03f;
	Parameters.Freeze = 0.0f;
	Parameters.RoomSize = 0.2f;
	Parameters.Dampening = 0.7f;
	Parameters.Width = 0.5f;

	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 0, Parameters.Wet );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 1, Parameters.Freeze );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 2, Parameters.RoomSize );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 3, Parameters.Dampening );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 2, 4, Parameters.Width );

	Mixer[Bus::SFX].setFilter( 3, &ReverbTail );

	Parameters.Wet = 0.003f;
	Parameters.Freeze = 0.0f;
	Parameters.RoomSize = 1.0f;
	Parameters.Dampening = 0.0f;
	Parameters.Width = 1.0f;

	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 0, Parameters.Wet );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 1, Parameters.Freeze );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 2, Parameters.RoomSize );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 3, Parameters.Dampening );
	Engine.setFilterParameter( MixerHandles[Bus::SFX], 3, 4, Parameters.Width );
}

void CSoLoudSound::Shutdown()
{
	StopAll();
	Sounds.clear();
	Streams.clear();
	Engine.deinit();
}
