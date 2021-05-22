// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Audio/SoLoudSound.h>

class SoundInstance
{
public:
	SoundInstance() = default;
	SoundInstance( class CSound* Sound );
	~SoundInstance();

	void Start( const Spatial Information = Spatial() );
	void Stop( const float FadeOut = -1.0f ) const;
	void Loop( const bool Loop ) const;
	void Rate( const float Rate ) const;
	float Time() const;
	float Length() const;
	void Offset( const float Offset ) const;
	bool Playing() const;
	void Volume( const float Volume ) const;
	void Fade( const float Volume, const float Time ) const;
	void Update( const Vector3D& Position, const Vector3D& Velocity ) const;

protected:
	class CSound* Asset = nullptr;
	int32_t Handle = -1;
	SoundHandle SoundHandle;
	StreamHandle StreamHandle;
};