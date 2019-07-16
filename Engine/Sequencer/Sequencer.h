// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>

typedef uint64_t Timecode;

class CSequence
{
public:
	CSequence();
	~CSequence();

	bool Load( const char* FileLocation );
	void Save( const char* FileLocation = nullptr );

	void Play();
	void Pause();
	void Stop();

	bool Playing() const;
	bool Stopped() const;

	void Step();
	void GoTo( const Timecode Marker );
	Timecode CurrentMarker() const;
	Timecode Size() const;

	float Time() const;
	float Length() const;

	// TODO: Move this elsewhere.
	void Frame();
	void Draw();

private:
	class CTimeline* Timeline;
};
