// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LogicTimerEntity.h"

static CEntityFactory<CLogicTimerEntity> Factory( "logic_timer" );

CLogicTimerEntity::CLogicTimerEntity()
{
	Construct();

	TriggerTime = 1.0f;
	Frequency = -1;
	TriggerCount = 0;

	Inputs["Start"] = [this] { this->Start(); };
	Inputs["Stop"] = [this] { this->Stop(); };
	Inputs["Reset"] = [this] { this->Reset(); };
}

void CLogicTimerEntity::Construct()
{
	
}

void CLogicTimerEntity::Tick()
{
	if( Timer.Enabled() )
	{
		const double ElapsedTime = Timer.GetElapsedTimeSeconds();
		if( ElapsedTime > TriggerTime )
		{
			if( Frequency < 0 || TriggerCount < Frequency )
			{
				Send( "OnTrigger" );
				Timer.Start();
			}

			if( TriggerCount > Frequency )
			{
				Stop();
			}

			TriggerCount++;
		}
	}
}

void CLogicTimerEntity::Destroy()
{
	
}

void CLogicTimerEntity::Load( const JSON::Vector& Objects )
{
	for( auto& Property : Objects )
	{
		if( Property->Key == "triggertime" )
		{
			const double PropertyTriggerTime = ParseDouble( Property->Value.c_str() );
			if( PropertyTriggerTime > 0.0f )
			{
				TriggerTime = PropertyTriggerTime;
			}
		}
		else if( Property->Key == "frequency" )
		{
			const double PropertyFrequency = ParseDouble( Property->Value.c_str() );
			if( PropertyFrequency > 0.0f )
			{
				Frequency = static_cast<int32_t>( PropertyFrequency );
			}
		}
	}
}

void CLogicTimerEntity::Start()
{
	Reset();
	Timer.Start();
}

void CLogicTimerEntity::Stop()
{
	Timer.Stop();
}

void CLogicTimerEntity::Reset()
{
	TriggerCount = 0;
}

void CLogicTimerEntity::Export( CData& Data )
{
	Data << TriggerTime;
	Data << Frequency;
	Data << TriggerCount;
	Data << Timer;
}

void CLogicTimerEntity::Import( CData& Data )
{
	Data >> TriggerTime;
	Data >> Frequency;
	Data >> TriggerCount;
	Data >> Timer;
}
