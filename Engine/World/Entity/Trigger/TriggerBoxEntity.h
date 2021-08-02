// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/PhysicsComponent.h>
#include <Engine/World/Entity/PointEntity/PointEntity.h>

class CTriggerBoxEntity : public CPointEntity
{
public:
	CTriggerBoxEntity() = default;

	void Construct() override;
	void Tick() override;
	void Destroy() override;
	void Load( const JSON::Vector& Objects ) override;
	void Reload() override;

	void Debug() override;

	void Export( CData& Data ) override;
	void Import( CData& Data ) override;

protected:
	bool CanTrigger() const;
	
	bool Latched = false;
	int32_t Frequency = -1;
	int32_t Count = 0;

	std::string FilterName;

	// When not null, only this entity will cause the trigger event to fire.
	CEntity* Filter = nullptr;

	const FBounds DefaultBounds = FBounds( Vector3D( -1.0f ), Vector3D( 1.0f ) );
	FBounds Bounds = DefaultBounds;

	CTriggerBody<Interactable*>* Volume = nullptr;
};
