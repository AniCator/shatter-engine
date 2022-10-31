// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Game/Game.h>
#include <Engine/Sequencer/Sequencer.h>
#include <Engine/Utility/Math/Transform.h>

#include <ThirdParty/imgui-1.70/imgui.h>

#include <set>

constexpr Timecode InvalidTimecode = -1;

enum class SequenceStatus : uint8_t
{
	Stopped = 0,
	Paused,
	Playing
};

struct FTrack
{
	void AddEvent( TrackEvent* Event );
	void Evaluate( const Timecode& Marker );
	void Reset();

	Timecode Start;
	Timecode Length;

	std::vector<TrackEvent*> Events;

	friend CData& operator<<( CData& Data, const FTrack& Track );
	friend CData& operator>>( CData& Data, FTrack& Track );
};

class CTimeline
{
public:
	CTimeline( Timecode Start = 0, Timecode End = 0 );
	~CTimeline() = default;

	bool Load( const char* FileLocation = nullptr );
	void Save( const char* FileLocation = nullptr ) const;

	void Play();
	void Pause();
	void Stop();

	bool Playing() const;
	bool Stopped() const;

	void Step();

	void GoTo( const Timecode& MarkerLocation );

	Timecode CurrentMarker() const;
	Timecode Size() const;

	double Time() const;
	double Length() const;

	void Feed();
	void Frame();

	/// <summary>
	/// Assigns timeline reference and track index to events.
	/// </summary>
	void ConfigureEvents();

	void Draw();
	void DisplayTimeline();

	void CreateTrack();
	void AdjustTrack( TrackEvent* Event );

	void EventDuplicate( TrackEvent* Source );
	void EventSplit( TrackEvent* Source );
	void EventSplitAtMarker( TrackEvent* Source );

	void Stretch( const float& Factor );

	void DeleteActiveEvents();
	void ClearActiveEvents();

	friend CData& operator<<( CData& Data, const CTimeline& Timeline );
	friend CData& operator>>( CData& Data, CTimeline& Timeline );

	bool DrawTimeline;
	double StartTime = -1.0;
	double PlayRate = 1.0;

	class CCamera* ActiveCamera = nullptr;
	FTrack* ActiveTrack = nullptr;
	size_t ActiveTrackIndex = InvalidTrackIndex;
	std::set<TrackEvent*> ActiveEvents;

	std::string Location;
	Timecode Marker;

	Timecode StartMarker;
	Timecode EndMarker;

	SequenceStatus Status;

	std::vector<FTrack> Tracks;

	/// State tracking
	//---
	double PreviousTime = 0.0;

	// True if the mouse is held down while dragging on the timeline bar.
	bool Scrubbing = false;
	//---

	struct TrackState
	{
		TrackEvent* Event = nullptr;
		bool Stretch = false;

		// For stretching, this indicates it is the left handle.
		bool Left = false;

		size_t TrackIndex = 0;

		// In screen space.
		ImVec2 CursorPosition = ImVec2( 0.0f, 0.0f );
	} LastDrag;

	// Automatically fit the scale of the timeline to what's in the tracks.
	bool AutoFit = false;

	// Snap events to each other when stretching.
	bool SnapToHandles = false;

	// Enable visualization for all tracks.
	bool Visualize = false;

	// Enables marker tracking.
	bool AutoScroll = true;
	Timecode ScrollMarker = InvalidTimecode;

	// True if the sequencer is skipping through the sequence and only wants to fire important events.
	bool Skipping = false;

	// Determines if camera events of this timeline should be active.
	bool EnableCamera = true;

	// Determines if a sequence should play once or multiple times.
	enum PlaybackMode
	{
		Once,
		Repeat
	} PlaybackMode = Once;

	Timecode LoopStart = InvalidTimecode;
	Timecode LoopEnd = InvalidTimecode;

	FTransform Transform;
};
