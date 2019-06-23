// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>

class CTriggerProximityEntity : public CPointEntity
{
public:
	CTriggerProximityEntity();

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;
	virtual void Load( const JSON::Vector& Objects ) override;

	virtual void Export( CData& Data ) override;
	virtual void Import( CData& Data ) override;

private:
	float Radius;
	bool Latched;
	int32_t Frequency;
	int32_t TriggerCount;
};
