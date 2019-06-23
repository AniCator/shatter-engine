// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/Entity.h>
#include <Engine/Utility/Timer.h>

class CLogicTimerEntity : public CEntity
{
public:
	CLogicTimerEntity();

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;
	virtual void Load( const JSON::Vector& Objects ) override;

	void Start();
	void Stop();
	void Reset();

	double TriggerTime;
	int32_t Frequency;

	virtual void Export( CData& Data ) override;
	virtual void Import( CData& Data ) override;
private:
	CTimer Timer;

	int32_t TriggerCount;
};
