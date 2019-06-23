// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/Entity.h>

class CLogicStartEntity : public CEntity
{
public:
	CLogicStartEntity();

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;
	virtual void Load( const JSON::Vector& Objects ) override;

	virtual void Export( CData& Data ) override;
	virtual void Import( CData& Data ) override;

private:
	bool HasStarted;
};
