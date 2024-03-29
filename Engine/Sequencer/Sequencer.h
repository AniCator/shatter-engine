// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>
#include <Engine/Utility/Data.h>

typedef uint64_t Timecode;
static const Timecode Timebase = 1536;

double MarkerToTime( const Timecode& Marker );
double MarkerRangeToTime( const Timecode& StartMarker, const Timecode& EndMarker );

constexpr size_t InvalidTrackIndex = -1;
struct TrackEvent
{
	virtual void Evaluate( const Timecode& Marker );
	virtual void Execute() = 0;
	virtual void Reset() = 0;
	virtual void Context();
	virtual void Visualize() {};

	template<typename T>
	static T* Create()
	{
		T* Event = new T();
		return Event;
	}

	static void AddType( const std::string& Name, const std::function<TrackEvent* ( )>& Generator );

	virtual const char* GetName() = 0;
	virtual const char* GetType() const = 0;

	Timecode Start = 0;
	Timecode Length = 0;

	// The marker position relative to the event.
	Timecode Offset = 0;
	Timecode PreviousOffset = 0;
	Timecode StoredMarker = 0;
	size_t TrackIndex = InvalidTrackIndex;
	class CTimeline* Timeline = nullptr;

	void UpdateInternalMarkers( const Timecode& Marker );

	bool Frozen() const
	{
		return PreviousOffset == Offset;
	}

	bool HasStarted( const Timecode& Marker ) const
	{
		return Marker >= Start;
	}

	bool HasEnded( const Timecode& Marker ) const
	{
		return Marker > ( Start + Length );
	}

	bool InRange( const Timecode& Marker ) const
	{
		return Marker >= Start && Marker < ( Start + Length );
	}

	virtual void Export( CData& Data ) const;
	virtual void Import( CData& Data );

	friend CData& operator<<( CData& Data, const TrackEvent* Track )
	{
		Track->Export( Data );
		return Data;
	}

	friend CData& operator>>( CData& Data, TrackEvent* Track )
	{
		Track->Import( Data );
		return Data;
	}
};

template<typename T>
TrackEvent* CreateTrack()
{
	return TrackEvent::Create<T>();
}

class CSequence
{
public:
	CSequence();
	~CSequence();

	bool Load( const char* FileLocation );
	void Save( const char* FileLocation = nullptr );
	std::string Location() const;

	void Play();
	void Pause();
	void Stop();

	bool Playing() const;
	bool Stopped() const;

	void Step();
	void GoTo( const Timecode& Marker );
	Timecode CurrentMarker() const;
	Timecode Size() const;

	double Time() const;
	double Length() const;

	// TODO: Move this elsewhere.
	void Frame();
	void Draw();
	void SetTransform( struct FTransform& Transform );

	template<typename T>
	static void AddType()
	{
		auto* Temporary = CreateTrack<T>();
		TrackEvent::AddType( Temporary->GetType(), CreateTrack<T> );
		delete Temporary;
	}

public:
	friend CData& operator<<( CData& Data, const CSequence& Sequence );
	friend CData& operator>>( CData& Data, CSequence& Sequence );

private:
	class CTimeline* Timeline;
};
