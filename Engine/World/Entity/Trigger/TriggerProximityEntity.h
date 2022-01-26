// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/PhysicsComponent.h>
#include <Engine/World/Entity/PointEntity/PointEntity.h>

class CTriggerProximityEntity : public CPointEntity
{
public:
	CTriggerProximityEntity();

	virtual void Construct() override;
	virtual void Tick() override;
	void Destroy() override;
	virtual void Load( const JSON::Vector& Objects ) override;

	virtual void Export( CData& Data ) override;
	virtual void Import( CData& Data ) override;

	void Debug() override;

private:
	bool CanTrigger() const;

	float Radius = 1.0f;
	bool Latched = false;
	int32_t Frequency = -1;
	int32_t Count = 0;

	CTriggerBody<Interactable*>* Volume = nullptr;
};
