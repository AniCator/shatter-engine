// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/Body/TriggerBody.h>
#include <Engine/World/Interactable.h>
#include <Engine/World/Entity/PointEntity/PointEntity.h>

class CTriggerProximityEntity : public CPointEntity
{
public:
	CTriggerProximityEntity();

	void Construct() override;
	void Tick() override;
	void Destroy() override;
	void Load( const JSON::Vector& Objects ) override;

	void Debug() override;

	void Export( CData& Data ) override;
	void Import( CData& Data ) override;

	void OnEnter( Interactable* Interactable );
	void OnLeave( Interactable* Interactable );

	// Fetches entities from the trigger volume.
	const std::unordered_set<Interactable*>& Fetch() const;

private:
	bool CanTrigger() const;
	bool IsValidEntity( Interactable* Interactable ) const;

	float Radius = 1.0f;
	float RadiusSquared = 1.0f;
	bool Latched = false;
	int32_t Frequency = -1;
	int32_t Count = 0;

	std::string FilterName;

	// When not null, only this entity will cause the trigger event to fire.
	CEntity* Filter = nullptr;

	CTriggerBody<Interactable*>* Volume = nullptr;
	std::unordered_set<Interactable*> LatchedEntities;
};
