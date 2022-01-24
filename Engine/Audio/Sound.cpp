// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Sound.h"

#include <Engine/Audio/SoLoudSound.h>
#include <Engine/Audio/SoundInstance.h>
#include <Engine/Utility/Math.h>

SoundInstance::SoundInstance( CSound* Sound )
{
	Asset = Sound;

	if( !Asset )
		return;
}

SoundInstance::~SoundInstance()
{
	Stop();
}

void SoundInstance::Start( const Spatial Information )
{
	if( !Asset )
		return;

	Stop();

	Handle = Asset->Start( Information );
	SoundHandle.Handle = Handle;
	StreamHandle.Handle = Handle;
}

void SoundInstance::Stop( const float FadeOut ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		CSoLoudSound::Stop( SoundHandle );
	}
	else
	{
		CSoLoudSound::Stop( StreamHandle, FadeOut );
	}
}

void SoundInstance::Loop( const bool Loop ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		CSoLoudSound::Loop( SoundHandle, Loop );
	}
	else
	{
		CSoLoudSound::Loop( StreamHandle, Loop );
	}
}

void SoundInstance::Rate( const float Rate ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		CSoLoudSound::Rate( SoundHandle, Rate );
	}
	else
	{
		CSoLoudSound::Rate( StreamHandle, Rate );
	}
}

float SoundInstance::Time() const
{
	if( !Asset )
		return 0.0f;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		return CSoLoudSound::Time( SoundHandle );
	}

	return CSoLoudSound::Time( StreamHandle );
}

float SoundInstance::Length() const
{
	if( !Asset )
		return 0.0f;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		return CSoLoudSound::Length( SoundHandle );
	}

	return CSoLoudSound::Length( StreamHandle );
}

void SoundInstance::Offset( const float Offset ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		CSoLoudSound::Offset( SoundHandle, Offset );
	}
	else
	{
		CSoLoudSound::Offset( StreamHandle, Offset );
	}
}

bool SoundInstance::Playing() const
{
	if( !Asset )
		return false;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		return CSoLoudSound::Playing( SoundHandle );
	}

	return CSoLoudSound::Playing( StreamHandle );
}

void SoundInstance::Volume( const float Volume ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		CSoLoudSound::Volume( SoundHandle, Volume );
	}
	else
	{
		CSoLoudSound::Volume( StreamHandle, Volume );
	}
}

void SoundInstance::Fade( const float Volume, const float Time ) const
{
	if( !Asset )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		CSoLoudSound::Fade( SoundHandle, Volume, Time );
	}
	else
	{
		CSoLoudSound::Fade( StreamHandle, Volume, Time );
	}
}

void SoundInstance::Update( const Vector3D& Position, const Vector3D& Velocity ) const
{
	if( !Asset || Handle < 0 )
		return;

	if( Asset->GetSoundType() == ESoundType::Memory )
	{
		CSoLoudSound::Update( SoundHandle, Position, Velocity );
	}
	else
	{
		CSoLoudSound::Update( StreamHandle, Position, Velocity );
	}
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
			auto Handle = CSoLoudSound::Sound( FileLocation );
			if( Handle.Handle > InvalidHandle )
			{
				BufferHandles.emplace_back( Handle );
				Loaded = true;
			}
		}
		else
		{
			auto Handle = CSoLoudSound::Music( FileLocation );
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
				auto Handle = CSoLoudSound::Sound( FileLocation );
				if( Handle.Handle > InvalidHandle )
				{
					BufferHandles.emplace_back( Handle );
					Loaded = true;
				}
			}
			else
			{
				auto Handle = CSoLoudSound::Music( FileLocation );
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

int32_t CSound::Start( const Spatial Information )
{
	if( SoundType == ESoundType::Memory )
	{
		const auto Buffer = Select();
		auto Handle = CSoLoudSound::Start( Buffer, Information );
		if( Handle.Handle > InvalidHandle )
		{
			SoundHandles.emplace_back( Handle );
			return Handle.Handle;
		}
	}
	else if( !StreamBufferHandles.empty() )
	{
		auto Handle = CSoLoudSound::Start( StreamBufferHandles[0], Information );
		if( Handle.Handle > InvalidHandle )
		{
			StreamHandles.emplace_back( Handle );
			return Handle.Handle;
		}
	}

	return -1;
}

void CSound::Stop( const float FadeOut )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Stop( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Stop( Handle, FadeOut );
		}
	}
}

void CSound::Loop( const bool Loop )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Loop( Handle, Loop );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Loop( Handle, Loop );
		}
	}
}

void CSound::Rate( const float Rate )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Rate( Handle, Rate );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Rate( Handle, Rate );
		}
	}
}

float CSound::Time() const
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			return CSoLoudSound::Time( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			return CSoLoudSound::Time( Handle );
		}
	}

	return 0.0f;
}

float CSound::Length() const
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			return CSoLoudSound::Length( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			return CSoLoudSound::Length( Handle );
		}
	}
	
	return -1.0f;
}

void CSound::Offset( const float Offset )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Offset( Handle, Offset );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Offset( Handle, Offset );
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
			if( CSoLoudSound::Playing( Handle ) )
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
			if( CSoLoudSound::Playing( Handle ) )
			{
				IsPlaying = true;
				break;
			}
		}
	}

	return IsPlaying;
}

void CSound::Volume( const float Volume )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Volume( Handle, Volume );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Volume( Handle, Volume );
		}
	}
}

void CSound::Fade( const float Volume, const float Time )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Fade( Handle, Volume, Time );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Fade( Handle, Volume, Time );
		}
	}
}

void CSound::SetPlayMode( ESoundPlayMode::Type NewPlayMode )
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
				CSoLoudSound::Update( Handle, Position, Velocity );
			}
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			if( Handle.Handle == HandleIn )
			{
				CSoLoudSound::Update( Handle, Position, Velocity );
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

