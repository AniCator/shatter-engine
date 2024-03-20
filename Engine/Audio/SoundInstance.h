// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Audio/SoLoudSound.h>

class SoundInstance
{
public:
	SoundInstance() = default;
	SoundInstance( class CSound* Sound );
	~SoundInstance();

	void Start( const Spatial& Information = Spatial() );
	void Stop( const float& FadeOut = -1.0f ) const;
	void Pause() const;
	void Loop( const bool& Loop ) const;
	void Rate( const float& Rate ) const;
	double Time() const;
	double Length() const;
	void Offset( const double& Offset ) const;
	bool Playing() const;
	void Volume( const float& Volume ) const;
	void Fade( const float& Volume, const float& Time ) const;
	void Update( const Vector3D& Position, const Vector3D& Velocity ) const;

	void Set( class CSound* Sound );
	void Synchronize( const SoundInstance& Source ) const;

	explicit operator bool() const
	{
		return Asset != nullptr;
	}

	bool AutoStop = true;

	SoundHandle SoundHandle;
	StreamHandle StreamHandle;
protected:
	class CSound* Asset = nullptr;

	int32_t Handle = -1;
};