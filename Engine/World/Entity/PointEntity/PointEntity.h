// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/Entity.h>
#include <Engine/Utility/Math.h>

class CPointEntity : public CEntity
{
public:
	CPointEntity();
	CPointEntity( const FTransform& Transform );
	virtual ~CPointEntity() override;

	virtual const FTransform& GetTransform() const { return Transform; };
	virtual void Load( const JSON::Vector& Objects ) override;

	virtual void Debug() override;

protected:
	FTransform Transform;
};
