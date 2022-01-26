// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>
#include <deque>

#include <Engine/Audio/SoLoud/Bus.h>
#include <Engine/Utility/Math/Vector.h>

static const int32_t InvalidHandle = -1;

template<typename HandleType>
HandleType EmptyHandle()
{
	HandleType EmptyHandle;
	EmptyHandle.Handle = -1;
	return EmptyHandle;
}

// Matches soloud.h
typedef unsigned int SoLoudHandle;

union SoundHandle
{
	int32_t Handle = -1;
};

union SoundBufferHandle
{
	int32_t Handle = -1;
};

union StreamHandle
{
	int32_t Handle = -1;
};

namespace SoLoud
{
	class Wav;
	class WavStream;
	class Queue;
}

struct FSound
{
	SoLoudHandle Voice = 0;
	SoundBufferHandle Buffer;
	float Volume = 1.0f;
	bool Playing = false;
};

struct FStream
{
	SoLoudHandle Stream = 0;
	StreamHandle Buffer;
	float FadeDuration = -1.0f;
	float StartTime = -1.0f;
	bool FadeIn = false;
	float Volume = 1.0f;
	bool Playing = false;
};

namespace Attenuation
{
	enum Type
	{
		// No attenuation
		Off = 0,
		// Inverse distance attenuation model
		Inverse = 1,
		// Linear distance attenuation model
		Linear = 2,
		// Exponential distance attenuation model
		Exponential = 3
	};
}

// Meta-data for 3D sounds.
struct Spatial
{
	bool Is3D = false;
	Vector3D Position = Vector3D::Zero;
	Vector3D Velocity = Vector3D::Zero;
	
	float MinimumDistance = 5.0f;
	float MaximumDistance = 10000.0f;
	bool DelayByDistance = false;

	Attenuation::Type Attenuation = Attenuation::Inverse;
	float Rolloff = 2.0f;
	float Doppler = 3.43f;

	Bus::Type Bus = Bus::SFX;
	bool StartPaused = false;
	float FadeIn = -1.0f;
	float Volume = 100.0f;
	float Rate = 1.0f;

	static Spatial Create()
	{
		return {};
	}

	static Spatial CreateDialogue()
	{
		Spatial Information;
		Information.Bus = Bus::Dialogue;
		return Information;
	}

	static Spatial CreateMusic()
	{
		Spatial Information;
		Information.Bus = Bus::Music;
		return Information;
	}

	static Spatial CreateUI()
	{
		Spatial Information;
		Information.Bus = Bus::UI;
		return Information;
	}

	static Spatial Create(
		const Vector3D& Position, 
		const Vector3D& Velocity
	)
	{
		Spatial Information;
		Information.Is3D = true;
		Information.Position = Position;
		Information.Velocity = Velocity;
		return Information;
	}

	static Spatial Create( class CMeshEntity* Entity );
};

class SoLoudSound
{
public:
	static SoundBufferHandle Sound( const std::string& ResourcePath );
	static StreamHandle Stream( const std::string& ResourcePath );
	static void Speak( const std::string& Sentence, const Spatial Information = Spatial() );

	static SoundHandle Start( SoundBufferHandle Handle, const Spatial Information = Spatial() );
	static StreamHandle Start( StreamHandle Handle, const Spatial Information = Spatial() );

	static void Stop( SoundHandle Handle );
	static void Stop( StreamHandle Handle, const float FadeOut = -1.0f );

	static void StopSounds();
	static void StopStreams();
	static void StopAll();

	static void Loop( SoundHandle Handle, const bool Loop );
	static void Loop( StreamHandle Handle, const bool Loop );

	static void Rate( SoundHandle Handle, const float Rate );
	static void Rate( StreamHandle Handle, const float Rate );

	static double Time( SoundHandle Handle );
	static double Time( StreamHandle Handle );

	static double Length( SoundHandle Handle );
	static double Length( StreamHandle Handle );

	static void Offset( SoundHandle Handle, const float Offset );
	static void Offset( StreamHandle Handle, const float Offset );

	static bool Playing( SoundHandle Handle );
	static bool Playing( StreamHandle Handle );

	static void Volume( SoundHandle Handle, const float Volume );
	static void Volume( StreamHandle Handle, const float Volume );

	static void Volume( const float GlobalVolume );

	static void Fade( SoundHandle Handle, const float Volume, const float Time );
	static void Fade( StreamHandle Handle, const float Volume, const float Time );

	static void GroupPause( const std::vector<StreamHandle>& Handles, const bool State );
	static void GroupProtect( const std::vector<StreamHandle>& Handles, const bool State );

	static void SetListenerPosition( const Vector3D& Position );
	static void SetListenerDirection( const Vector3D& Direction );
	static void SetListenerUpDirection( const Vector3D& Direction );
	static void SetListenerVelocity( const Vector3D& Velocity );

	static void Update( SoundHandle Handle, const Vector3D& Position, const Vector3D& Velocity );
	static void Update( StreamHandle Handle, const Vector3D& Position, const Vector3D& Velocity );

	static Bus::Volume GetBusOutput( const Bus::Type& Bus );
	static float* GetBusFFT( const Bus::Type& Bus );
	static float Volume( const Bus::Type& Bus );
	static void Volume( const Bus::Type& Bus, const float& Volume );

	static struct FilterStack& GetBusStack( const Bus::Type& Bus );

	static void Tick();

	static void Initialize();
	static void Shutdown();

private:
	static std::deque<FSound> Sounds;
	static std::vector<SoLoud::Wav*> SoundBuffers;
	static std::vector<SoLoud::WavStream*> StreamBuffers;
	static std::vector<FStream> Streams;

	static float GlobalVolume;
};
