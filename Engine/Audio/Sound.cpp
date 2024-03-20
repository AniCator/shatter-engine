// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Sound.h"

#include <Engine/Audio/SoLoudSound.h>
#include <Engine/Audio/SoundInstance.h>
#include <Engine/Utility/Math.h>

SoundInstance::SoundInstance( CSound* Sound )
{
	Asset = Sound;
}

SoundInstance::~SoundInstance()
{
	if( AutoStop )
		Stop();
}

void SoundInstance::Start( const Spatial& Information )
{
	if( !Asset )
		return;

	if( AutoStop )
		Stop();

	Handle = Asset->Start( Information );
	SoundHandle.Handle = Handle;
	StreamHandle.Handle = Handle;
}

void SoundInstance::Stop( const float& FadeOut ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		SoLoudSound::Stop( SoundHandle );
	}
	else
	{
		SoLoudSound::Stop( StreamHandle, FadeOut );
	}
}

void SoundInstance::Pause() const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		SoLoudSound::Pause( SoundHandle );
	}
	else
	{
		SoLoudSound::Pause( StreamHandle );
	}
}

void SoundInstance::Loop( const bool& Loop ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		SoLoudSound::Loop( SoundHandle, Loop );
	}
	else
	{
		SoLoudSound::Loop( StreamHandle, Loop );
	}
}

void SoundInstance::Rate( const float& Rate ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		SoLoudSound::Rate( SoundHandle, Rate );
	}
	else
	{
		SoLoudSound::Rate( StreamHandle, Rate );
	}
}

double SoundInstance::Time() const
{
	if( !Asset )
		return 0.0;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		return SoLoudSound::Time( SoundHandle );
	}

	return SoLoudSound::Time( StreamHandle );
}

double SoundInstance::Length() const
{
	if( !Asset )
		return 0.0;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		return SoLoudSound::Length( SoundHandle );
	}

	return SoLoudSound::Length( StreamHandle );
}

void SoundInstance::Offset( const double& Offset ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		SoLoudSound::Offset( SoundHandle, Offset );
	}
	else
	{
		SoLoudSound::Offset( StreamHandle, Offset );
	}
}

bool SoundInstance::Playing() const
{
	if( !Asset )
		return false;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		return SoLoudSound::Playing( SoundHandle );
	}

	return SoLoudSound::Playing( StreamHandle );
}

void SoundInstance::Volume( const float& Volume ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		SoLoudSound::Volume( SoundHandle, Volume );
	}
	else
	{
		SoLoudSound::Volume( StreamHandle, Volume );
	}
}

void SoundInstance::Fade( const float& Volume, const float& Time ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		SoLoudSound::Fade( SoundHandle, Volume, Time );
	}
	else
	{
		SoLoudSound::Fade( StreamHandle, Volume, Time );
	}
}

void SoundInstance::Update( const Vector3D& Position, const Vector3D& Velocity ) const
{
	if( !Asset || Handle < 0 )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		SoLoudSound::Update( SoundHandle, Position, Velocity );
	}
	else
	{
		SoLoudSound::Update( StreamHandle, Position, Velocity );
	}
}

void SoundInstance::Set( CSound* Sound )
{
	if( !Sound )
		return;

	if( AutoStop )
		Stop();

	Asset = Sound;
}

void SoundInstance::Synchronize( const SoundInstance& Source ) const
{
	if( !Asset || Handle < 0 || !Source.Asset || Source.Handle < 0 )
		return;

	// Only supports streams.
	if( Asset->GetSoundType() != ESoundType::Stream || Source.Asset->GetSoundType() != ESoundType::Stream )
		return;

	SoLoudSound::Synchronize( Source.StreamHandle, StreamHandle );
}

CSound::CSound( ESoundType::Type TypeIn )
{
	SoundType = TypeIn;
	Loaded = false;
}

CSound::~CSound()
{
	Clear();
}

bool CSound::Load( const char* FileLocation )
{
	if( !Loaded )
	{
		if( SoundType == ESoundType::Memory )
		{
			auto Handle = SoLoudSound::Sound( FileLocation );
			if( Handle.Handle > InvalidHandle )
			{
				BufferHandles.emplace_back( Handle );
				Loaded = true;
			}
		}
		else
		{
			auto Handle = SoLoudSound::Stream( FileLocation );
			if( Handle.Handle > InvalidHandle )
			{
				StreamBufferHandles.emplace_back( Handle );
				Loaded = true;
			}
		}
	}

	this->FileLocation = FileLocation;

	return Loaded;
}

bool CSound::Load( const std::vector<std::string>& Locations )
{
	if( !Loaded )
	{
		for( const auto& FileLocation : Locations )
		{
			if( SoundType == ESoundType::Memory )
			{
				auto Handle = SoLoudSound::Sound( FileLocation );
				if( Handle.Handle > InvalidHandle )
				{
					BufferHandles.emplace_back( Handle );
					Loaded = true;
				}
			}
			else
			{
				auto Handle = SoLoudSound::Stream( FileLocation );
				if( Handle.Handle > InvalidHandle )
				{
					StreamBufferHandles.emplace_back( Handle );
					Loaded = true;
				}
			}
		}
	}

	if( !Locations.empty() )
	{
		FileLocation = Locations[0];
	}

	return Loaded;
}

void CSound::Clear( const bool& Unload )
{
	Stop();
	Location = 0;
	
	SoundHandles.clear();
	StreamHandles.clear();

	if( Unload )
	{
		Loaded = false;

		BufferHandles.clear();
		StreamBufferHandles.clear();
	}
}

int32_t CSound::Start( const Spatial& Information )
{
	if( SoundType == ESoundType::Memory )
	{
		const auto Buffer = Select();
		auto Handle = SoLoudSound::Start( Buffer, Information );
		if( Handle.Handle > InvalidHandle )
		{
			SoundHandles.emplace_back( Handle );
			return Handle.Handle;
		}
	}
	else if( !StreamBufferHandles.empty() )
	{
		auto Handle = SoLoudSound::Start( StreamBufferHandles[0], Information );
		if( Handle.Handle > InvalidHandle )
		{
			StreamHandles.emplace_back( Handle );
			return Handle.Handle;
		}
	}

	return -1;
}

void CSound::Stop( const float& FadeOut )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			SoLoudSound::Stop( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			SoLoudSound::Stop( Handle, FadeOut );
		}
	}
}

void CSound::Loop( const bool& Loop )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			SoLoudSound::Loop( Handle, Loop );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			SoLoudSound::Loop( Handle, Loop );
		}
	}
}

void CSound::Rate( const float& Rate )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			SoLoudSound::Rate( Handle, Rate );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			SoLoudSound::Rate( Handle, Rate );
		}
	}
}

double CSound::Time() const
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			return SoLoudSound::Time( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			return SoLoudSound::Time( Handle );
		}
	}

	return 0.0;
}

double CSound::Length() const
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			return SoLoudSound::Length( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			return SoLoudSound::Length( Handle );
		}
	}
	
	return -1.0;
}

void CSound::Offset( const double& Offset )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			SoLoudSound::Offset( Handle, Offset );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			SoLoudSound::Offset( Handle, Offset );
		}
	}
}

bool CSound::Playing()
{
	bool IsPlaying = false;

	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			if( SoLoudSound::Playing( Handle ) )
			{
				IsPlaying = true;
				break;
			}
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			if( SoLoudSound::Playing( Handle ) )
			{
				IsPlaying = true;
				break;
			}
		}
	}

	return IsPlaying;
}

void CSound::Volume( const float& Volume )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			SoLoudSound::Volume( Handle, Volume );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			SoLoudSound::Volume( Handle, Volume );
		}
	}
}

void CSound::Fade( const float& Volume, const float& Time )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			SoLoudSound::Fade( Handle, Volume, Time );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			SoLoudSound::Fade( Handle, Volume, Time );
		}
	}
}

void CSound::SetPlayMode( const ESoundPlayMode::Type& NewPlayMode )
{
	PlayMode = NewPlayMode;
}

void CSound::Update( const int32_t& HandleIn, const Vector3D& Position, const Vector3D& Velocity )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			if( Handle.Handle == HandleIn )
			{
				SoLoudSound::Update( Handle, Position, Velocity );
			}
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			if( Handle.Handle == HandleIn )
			{
				SoLoudSound::Update( Handle, Position, Velocity );
			}
		}
	}
}

SoundBufferHandle CSound::Select()
{
	if( BufferHandles.empty() )
		return EmptyHandle<SoundBufferHandle>();

	if( PlayMode == ESoundPlayMode::Unknown || PlayMode == ESoundPlayMode::Sequential )
	{
		if( Location >= BufferHandles.size() )
		{
			Location = 0;
		}

		return BufferHandles[Location++];
	}
	else if( PlayMode == ESoundPlayMode::Random )
	{
		uint32_t NewLocation = static_cast<uint32_t>( Math::RandomRangeInteger( 0, BufferHandles.size() - 1 ) );
		while( NewLocation == Location )
		{
			// NewLocation = ( NewLocation + 1 ) % BufferHandles.size();
			NewLocation = static_cast<uint32_t>( Math::RandomRangeInteger( 0, BufferHandles.size() - 1 ) );
		}

		Location = NewLocation;
		return BufferHandles[Location];
	}

	return EmptyHandle<SoundBufferHandle>();
}

