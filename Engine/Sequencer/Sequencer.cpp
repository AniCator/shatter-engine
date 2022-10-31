// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Sequencer.h"

#include <Engine/Sequencer/Timeline.h>

CSequence::CSequence()
{
	Timeline = new CTimeline();
}

CSequence::~CSequence()
{
	delete Timeline;
	Timeline = nullptr;
}

bool CSequence::Load( const char* FileLocation )
{
	return Timeline->Load( FileLocation );
}

void CSequence::Save( const char* FileLocation )
{
	Timeline->Save( FileLocation );
}

std::string CSequence::Location() const
{
	return Timeline->Location;
}

void CSequence::Play()
{
	Timeline->Play();
}

void CSequence::Pause()
{
	Timeline->Pause();
}

void CSequence::Stop()
{
	Timeline->Stop();
}

bool CSequence::Playing() const
{
	return Timeline->Playing();
}

bool CSequence::Stopped() const
{
	return Timeline->Stopped();
}

void CSequence::Step()
{
	Timeline->Step();
}

void CSequence::GoTo( const Timecode& Marker )
{
	Timeline->GoTo( Marker );
}

Timecode CSequence::CurrentMarker() const
{
	return Timeline->CurrentMarker();
}

Timecode CSequence::Size() const
{
	return Timeline->Size();
}

double CSequence::Time() const
{
	return Timeline->Time();
}

double CSequence::Length() const
{
	return Timeline->Length();
}

void CSequence::Frame()
{
	Timeline->Frame();
}

void CSequence::Draw()
{
	Timeline->Draw();
}

void CSequence::SetTransform( FTransform& Transform )
{
	Timeline->Transform = Transform;
}

CData& operator<<( CData& Data, const CSequence& Sequence )
{
	Data << *Sequence.Timeline;
	return Data;
}

CData& operator>>( CData& Data, CSequence& Sequence )
{
	Data >> *Sequence.Timeline;
	return Data;
}
