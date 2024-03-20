// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SoLoudSound.h"

#include <stack>

#include <ThirdParty/SoLoud/include/soloud.h>
#include <ThirdParty/SoLoud/include/soloud_wav.h>
#include <ThirdParty/SoLoud/include/soloud_wavstream.h>
#include <ThirdParty/SoLoud/include/soloud_speech.h>
#include <ThirdParty/SoLoud/include/soloud_freeverbfilter.h>
#include <ThirdParty/SoLoud/include/soloud_echofilter.h>
#include <ThirdParty/SoLoud/include/soloud_lofifilter.h>
#include <ThirdParty/SoLoud/include/soloud_limiter.h>

#include <Engine/Audio/SoLoud/EffectStack.h>
#include <Engine/Audio/SoundQueue.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#undef GetCurrentTime
#include <Game/Game.h>

#include "Sound.h"

std::deque<FSound> SoLoudSound::Sounds;
std::vector<SoLoud::Wav*> SoLoudSound::SoundBuffers;
std::vector<SoLoud::WavStream*> SoLoudSound::StreamBuffers;
std::vector<FStream> SoLoudSound::Streams;

float SoLoudSound::GlobalVolume = 1.0f;

SoLoud::Soloud Engine;
SoLoud::FreeverbFilter ReverbEarlyReflection;
SoLoud::FreeverbFilter ReverbTail;
SoLoud::EchoFilter Echo;
SoLoud::LofiFilter Lofi;

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

void FilterStack::Add( SoLoud::Filter* Filter, const std::vector<float>& Parameters, const std::string& Name )
{
	Effect NewEffect;
	NewEffect.Filter = Filter;
	NewEffect.Parameters = Parameters;
	NewEffect.Name = Name;
	
	Stack.emplace_back( NewEffect );
}

void FilterStack::Toggle( const Effect* Marked )
{
	if( !Marked )
		return;
	
	const auto Iterator = std::find_if( Stack.begin(), Stack.end(), [Marked] ( const Effect& Effect )
		{
			return &Effect == Marked;
		}
	);

	if( Iterator->Filter )
	{
		Iterator->Inactive = Iterator->Filter;
		Iterator->Filter = nullptr;
	}
	else if ( Iterator->Inactive )
	{
		Iterator->Filter = Iterator->Inactive;
		Iterator->Inactive = nullptr;
	}

	SetForBus( LatestBus );
	// Stack.erase( Iterator );
}

void FilterStack::SetForBus( const Bus::Type& Bus )
{
	LatestBus = Bus;
	
	if( Stack.empty() )
		return;
	
	uint32_t FilterID = 1;
	for( const auto& Effect : Stack )
	{
		if( Bus == Bus::Maximum )
		{
			Engine.setGlobalFilter( FilterID, Effect.Filter );
		}
		else
		{
			Mixer[Bus].setFilter( FilterID, Effect.Filter );
		}
		
		FilterID++;
	}

	UpdateForBus( Bus );
}

void FilterStack::UpdateForBus( const Bus::Type& Bus )
{
	if( Stack.empty() )
		return;
	
	uint32_t FilterID = 1;
	for( const auto& Effect : Stack )
	{
		uint32_t AttributeID = 0;
		for( const auto& Parameter : Effect.Parameters )
		{
			if( Bus == Bus::Maximum )
			{
				Engine.setFilterParameter( 0, FilterID, AttributeID, Parameter );
			}
			else
			{
				Engine.setFilterParameter( MixerHandles[Bus], FilterID, AttributeID, Parameter );
			}

			AttributeID++;
		}
		
		FilterID++;
	}
}

FilterStack Stacks[Bus::Maximum + 1];

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
	const float Volume = FadeIn ? 0.0f : Information.Volume * 0.01f;
	
	if( Information.Is3D )
	{
		Handle = Mixer[SelectedBus].play3d( AudioSource,
				Information.Position.X, Information.Position.Y, Information.Position.Z,
				Information.Velocity.X, Information.Velocity.Y, Information.Velocity.Z,
			0.0f, Information.StartPaused );

		Configure3DSound( Handle, Information );

		if( Information.DelayByDistance )
		{
			AudioSource.set3dDistanceDelay( true );
		}
	}
	else
	{
		Handle = Mixer[SelectedBus].play( AudioSource, 0.0f, 0.0f, Information.StartPaused );
	}

	Engine.setRelativePlaySpeed( Handle, Information.Rate );

	return Handle;
}

SoundBufferHandle SoLoudSound::Sound( const std::string& ResourcePath )
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

StreamHandle SoLoudSound::Stream( const std::string& ResourcePath )
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
void SoLoudSound::Speak( const std::string& Sentence, const Spatial Information )
{
	Speech.setText( Sentence.c_str() );
	Speech.setParams( Math::RandomRangeInteger( 2000, 3000 ), 5.0f );
	Speech.setVolume( Information.Volume * 0.01f );

	unsigned int SelectedBus = Information.Bus;
	if( SelectedBus >= Bus::Maximum )
	{
		SelectedBus = Bus::SFX;
	}

	if( !Information.Is3D )
	{
		Mixer[SelectedBus].play( Speech, 1.0f );
	}
	else
	{
		Mixer[SelectedBus].play3d( Speech, 
			Information.Position.X, Information.Position.Y, Information.Position.Z, 
			Information.Velocity.X, Information.Velocity.Y, Information.Velocity.Z,
			1.0f );
	}
}

SoundHandle SoLoudSound::Start( SoundBufferHandle Handle, const Spatial Information )
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

		// Kill inaudible sounds.
		Engine.setInaudibleBehavior( NewSound.Voice, false, true );
		
		SoundHandle Handle;
		Handle.Handle = Sounds.size() - 1;

		if( FadeIn )
		{
			Fade( Handle, Information.Volume, Information.FadeIn );
		}
		else
		{
			Volume( Handle, Information.Volume );
		}
		
		return Handle;
	}
	
	return EmptyHandle<SoundHandle>();
}

StreamHandle SoLoudSound::Start( StreamHandle BufferHandle, const Spatial Information )
{
	if( GlobalVolume == 0.0f )
		return EmptyHandle<StreamHandle>();

	if( BufferHandle.Handle == InvalidHandle )
		return EmptyHandle<StreamHandle>();

	if( BufferHandle.Handle < StreamBuffers.size() )
	{
		auto& AudioSource = *StreamBuffers[BufferHandle.Handle];
		
		FStream NewStream;
		NewStream.Playing = true;
		NewStream.FadeDuration = Information.FadeIn;
		NewStream.FadeIn = Information.FadeIn > 0.0f;
		NewStream.StartTime = GameLayersInstance->GetRealTime();
		NewStream.Buffer = BufferHandle;

		NewStream.Stream = PlaySound( AudioSource, Information );

		Streams.emplace_back( NewStream );

		// Always tick inaudible streams.
		Engine.setInaudibleBehavior( NewStream.Stream, true, false );

		StreamHandle Handle;
		Handle.Handle = Streams.size() - 1;

		if( NewStream.FadeIn )
		{
			Engine.setVolume( NewStream.Stream, 0.0f );
			Engine.fadeVolume( NewStream.Stream, Information.Volume * 0.01f, Information.FadeIn );
		}
		else
		{
			Volume( Handle, Information.Volume );
		}
		
		return Handle;
	}

	return EmptyHandle<StreamHandle>();
}

void SoLoudSound::Stop( SoundHandle Handle )
{
	if( Sounds.empty() )
		return;
	
	if( Handle.Handle > InvalidHandle )
	{
		Engine.stop( Sounds[Handle.Handle].Voice );
	}
}

void SoLoudSound::Stop( StreamHandle Handle, const float FadeOut )
{
	if( Streams.empty() )
		return;
	
	if( Handle.Handle > InvalidHandle )
	{
		if( FadeOut < 0.0f )
		{
			Engine.stop( Streams[Handle.Handle].Stream );

			// Invalidate the stream handle.
			Streams[Handle.Handle].Stream = 0;
		}
		else
		{
			if( Streams[Handle.Handle].FadeIn )
			{
				Streams[Handle.Handle].Volume = Engine.getVolume( Streams[Handle.Handle].Stream );
			}

			Streams[Handle.Handle].FadeDuration = FadeOut;
			Streams[Handle.Handle].FadeIn = false;
			Streams[Handle.Handle].StartTime = GameLayersInstance->GetRealTime();

			Fade( Handle, 0.0f, FadeOut );

			// Invalidate the stream handle.
			Streams[Handle.Handle].Stream = 0;
		}
	}
}

void SoLoudSound::Pause( SoundHandle Handle )
{
	if( Sounds.empty() )
		return;

	if( Handle.Handle > InvalidHandle )
	{
		Engine.setPause( Sounds[Handle.Handle].Voice, true );
	}
}

void SoLoudSound::Pause( StreamHandle Handle )
{
	if( Sounds.empty() )
		return;

	if( Handle.Handle > InvalidHandle )
	{
		Engine.setPause( Streams[Handle.Handle].Stream, true );
	}
}

void SoLoudSound::StopSounds()
{
	for( const auto& Sound : Sounds )
	{
		Engine.stop( Sound.Voice );
	}
}

void SoLoudSound::StopStreams()
{
	for( const auto& Stream : Streams )
	{
		Engine.stop( Stream.Stream );
	}
}

void SoLoudSound::StopAll()
{
	StopSounds();
	StopStreams();
}

void SoLoudSound::Loop( SoundHandle Handle, const bool Loop )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.setLooping( Sounds[Handle.Handle].Voice, Loop );
	}
}

void SoLoudSound::Loop( StreamHandle Handle, const bool Loop )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.setLooping( Streams[Handle.Handle].Stream, Loop );
	}
}

void SoLoudSound::Rate( SoundHandle Handle, const float Rate )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.setRelativePlaySpeed( Sounds[Handle.Handle].Voice, Rate );
	}
}

void SoLoudSound::Rate( StreamHandle Handle, const float Rate )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.setRelativePlaySpeed( Streams[Handle.Handle].Stream, Rate );
	}
}

double SoLoudSound::Time( SoundHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return Engine.getStreamPosition( Sounds[Handle.Handle].Voice );
	}
	
	return 0.0;
}

double SoLoudSound::Time( StreamHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		const auto& Stream = Streams[Handle.Handle];
		return Engine.getStreamPosition( Stream.Stream );;
	}
	
	return 0.0;
}

double SoLoudSound::Length( SoundHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return SoundBuffers[Sounds[Handle.Handle].Buffer.Handle]->getLength();
	}
	
	return -1.0;
}

double SoLoudSound::Length( StreamHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return StreamBuffers[Streams[Handle.Handle].Buffer.Handle]->getLength();
	}
	
	return -1.0;
}

void SoLoudSound::Offset( SoundHandle Handle, const double& Offset )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.seek( Sounds[Handle.Handle].Voice, Offset );
	}
}

void SoLoudSound::Offset( StreamHandle Handle, const double& Offset )
{
	if( Handle.Handle > InvalidHandle )
	{
		Engine.seek( Streams[Handle.Handle].Stream, Offset );
	}
}

void SoLoudSound::Synchronize(const StreamHandle& Source, const StreamHandle& Target )
{
	if( Source.Handle > InvalidHandle && Target.Handle > InvalidHandle )
	{
		const auto& SourceStream = Streams[Source.Handle];
		auto& TargetStream = Streams[Target.Handle];

		Engine.sync( SourceStream.Stream, TargetStream.Stream );
	}
}

bool SoLoudSound::Playing( SoundHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return Engine.isValidVoiceHandle( Sounds[Handle.Handle].Voice );
	}
	
	return false;
}

bool SoLoudSound::Playing( StreamHandle Handle )
{
	if( Handle.Handle > InvalidHandle )
	{
		return Engine.isValidVoiceHandle( Streams[Handle.Handle].Stream );
	}
	
	return false;
}

void SoLoudSound::Volume( SoundHandle Handle, const float Volume )
{
	if( Handle.Handle == InvalidHandle )
		return;
	
	Sounds[Handle.Handle].Volume = Volume * 0.01f;
	Engine.setVolume( Sounds[Handle.Handle].Voice, Volume * 0.01f );
}

void SoLoudSound::Volume( StreamHandle Handle, const float Volume )
{
	if( Handle.Handle == InvalidHandle )
		return;
	
	Streams[Handle.Handle].Volume = Volume * 0.01f;
	Engine.setVolume( Streams[Handle.Handle].Stream, Volume * 0.01f );
}

void SoLoudSound::Fade( SoundHandle Handle, const float Volume, const float Time )
{
	if( Handle.Handle == InvalidHandle )
		return;
	
	Engine.fadeVolume( Sounds[Handle.Handle].Voice, Volume * 0.01f, Time );
}

void SoLoudSound::Fade( StreamHandle Handle, const float Volume, const float Time )
{
	if( Handle.Handle == InvalidHandle )
		return;
	
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

void SoLoudSound::GroupPause( const std::vector<StreamHandle>& Handles, const bool State )
{
	const auto PauseGroup = CreateGroup( Handles );
	Engine.setPause( PauseGroup, State );
	DeleteGroup( PauseGroup );
}

void SoLoudSound::GroupProtect( const std::vector<StreamHandle>& Handles, const bool State )
{
	const auto ProtectGroup = CreateGroup( Handles );
	Engine.setProtectVoice( ProtectGroup, State );
	DeleteGroup( ProtectGroup );
}

void SoLoudSound::SetListenerPosition( const Vector3D& Position )
{
	Engine.set3dListenerPosition( Position.X, Position.Y, Position.Z );
}

void SoLoudSound::SetListenerDirection( const Vector3D& Direction )
{
	Engine.set3dListenerAt( Direction.X, Direction.Y, Direction.Z );
}

void SoLoudSound::SetListenerUpDirection( const Vector3D& Direction )
{
	Engine.set3dListenerUp( Direction.X, Direction.Y, Direction.Z );
}

void SoLoudSound::SetListenerVelocity( const Vector3D& Velocity )
{
	Engine.set3dListenerVelocity( Velocity.X, Velocity.Y, Velocity.Z );
}

void SoLoudSound::Update( SoundHandle Handle, const Vector3D& Position, const Vector3D& Velocity )
{
	Engine.set3dSourceParameters( Sounds[Handle.Handle].Voice,
		Position.X, Position.Y, Position.Z,
		Velocity.X, Velocity.Y, Velocity.Z
	);
}

void SoLoudSound::Update( StreamHandle Handle, const Vector3D& Position, const Vector3D& Velocity )
{
	Engine.set3dSourceParameters( Streams[Handle.Handle].Stream,
		Position.X, Position.Y, Position.Z,
		Velocity.X, Velocity.Y, Velocity.Z
	);
}

Bus::Volume SoLoudSound::GetBusOutput( const Bus::Type& Bus )
{
	Bus::Volume Volume;
	if( Bus == Bus::Master )
	{
		Volume.Left = Engine.getApproximateVolume( 0 );
		Volume.Right = Engine.getApproximateVolume( 1 );
	}
	else
	{
		Volume.Left = Mixer[Bus].getApproximateVolume( 0 );
		Volume.Right = Mixer[Bus].getApproximateVolume( 1 );
	}

	return Volume;
}

float* SoLoudSound::GetBusFFT( const Bus::Type& Bus )
{
	if( Bus == Bus::Master )
	{
		return Engine.calcFFT();
	}

	return Mixer[Bus].calcFFT();
}

float SoLoudSound::Volume( const Bus::Type& Bus )
{
	if( Bus == Bus::Master )
	{
		return Engine.getGlobalVolume();
	}
	
	return Mixer[Bus].getVolume();
}

void SoLoudSound::Volume( const Bus::Type& Bus, const float& Volume )
{
	if( Bus == Bus::Master )
	{
		Engine.setGlobalVolume( Volume );
	}
	
	Mixer[Bus].setVolume( Volume );
}

FilterStack& SoLoudSound::GetBusStack( const Bus::Type& Bus )
{
	return Stacks[Bus];
}

void SoLoudSound::Volume( const float GlobalVolumeIn )
{
	Engine.setGlobalVolume( GlobalVolumeIn * 0.01f );
}

void SoLoudSound::Tick()
{
	Profile( "Sound" );
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

	CProfiler& Profiler = CProfiler::Get();
	const auto StreamEntry = ProfileTimeEntry( "Active Streams", ActiveStreams );
	Profiler.AddCounterEntry( StreamEntry, false, true );

	const auto VoiceEntry = ProfileTimeEntry( "Active Voices", Engine.getActiveVoiceCount() );
	Profiler.AddCounterEntry( VoiceEntry, false, true );

	Engine.update3dAudio();
}

void SoLoudSound::Initialize()
{
	constexpr unsigned int Flags = 0;
	constexpr unsigned int Backend = 0;
	constexpr unsigned int SampleRate = 44100;
	constexpr unsigned int BufferSize = 1 << 9;
	constexpr unsigned int Channels = 2;

	Engine.init( Flags, Backend, SampleRate, BufferSize, Channels );

	Engine.setMaxActiveVoiceCount( 128 );
	Engine.setVisualizationEnable( true );

	for( SoLoud::handle Handle = 0; Handle < Bus::Maximum; Handle++ )
	{
		MixerHandles[Handle] = Engine.play( Mixer[Handle], 1.0f );
		Mixer[Handle].setVisualizationEnable( true );
	}

	Stacks[Bus::SFX].Add( &Echo, {
		0.12f, // Wet
		0.343f, // Delay
		0.25f, // Decay
		0.1f, // Filter
		},
		"Echo" );

	Stacks[Bus::SFX].Add( &ReverbEarlyReflection, {
		0.03f, // Wet
		0.0f, // Freeze
		0.2f, // RoomSize
		0.7f, // Dampening
		0.5f // Width
		},
		"Reverb Early Reflection" );
	
	Stacks[Bus::SFX].Add( &ReverbTail, {
		0.01f, // Wet
		0.0f, // Freeze
		1.0f, // RoomSize
		0.0f, // Dampening
		1.0f // Width
		},
		"Reverb Tail" );

	Stacks[Bus::Auxilery7].Add( &ReverbTail, {
		0.314f, // Wet
		0.0f, // Freeze
		1.0f, // RoomSize
		0.0f, // Dampening
		1.0f // Width
		},
		"Mega Tail" );

	Stacks[Bus::Auxilery7].Add( &Lofi, {
		0.85f, // Wet
		2756.0f, // Sample Rate
		16.0f // Bit Depth
		},
		"Lofi" );

	static SoLoud::Limiter Limiter;

	Stacks[Bus::Master].Add( &Limiter, {
		2.0f, // Pre-Gain
		0.95f, // Post-Gain
		0.01f, // Release
		1.0f // Oof owie my ears clipper
		},
		"Limiter" );

	// Apply effects stack.
	for( uint64_t Handle = 0; Handle <= Bus::Maximum; Handle++ )
	{
		Stacks[Handle].SetForBus( StaticCast<Bus::Type>( Handle ) );
	}

	StopAll();
}

void SoLoudSound::Shutdown()
{
	Sounds.clear();
	Streams.clear();

	// TODO: This seems to freeze the application in some cases, causing it to still run in the background, waiting eternally.
	// while( Engine.mInsideAudioThreadMutex )
	{
		// Wait for SoLoud to finish processing to avoid a potential crash on exit.
	}

	Engine.deinit();
}

SoLoud::Wav* SoLoudSound::GetSound( const SoundHandle& Handle )
{
	if( Handle.Handle == InvalidHandle )
		return nullptr;

	return SoundBuffers[Handle.Handle];
}

SoLoud::WavStream* SoLoudSound::GetStream( const StreamHandle& Handle )
{
	if( Handle.Handle == InvalidHandle )
		return nullptr;

	return StreamBuffers[Handle.Handle];
}

void SoundQueue::Construct()
{
	Instance = new SoLoud::Queue();
	Mixer[Bus::Music].play( *Instance );
}

void SoundQueue::Destroy()
{
	delete Instance;
}

// TODO: This doesn't seem to work. No sound is produced.
void SoundQueue::Add( const CSound* Sound )
{
	if( !Sound )
		return;

	const auto& Handles = Sound->GetStreamBufferHandles();
	if( Handles.empty() )
		return;

	if( !Instance )
		Construct();

	Instance->play( *SoLoudSound::GetStream( Handles[0] ) );
	Instance->setVolume( 10.0f );
}

uint32_t SoundQueue::Count() const
{
	if( !Instance )
		return 0;

	return Instance->getQueueCount();
}

bool SoundQueue::Playing() const
{
	if( !Instance )
		return false;

	return Instance->getQueueCount() > 0;
}
