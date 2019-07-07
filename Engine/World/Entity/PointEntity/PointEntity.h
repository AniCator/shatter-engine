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

	virtual const FTransform& GetTransform();
	virtual const FTransform& GetLocalTransform();
	virtual void SetTransform(const FTransform& Transform );
	virtual void Load( const JSON::Vector& Objects ) override;

	virtual void Debug() override;

	virtual void Import( CData& Data ) override;
	virtual void Export( CData& Data ) override;

protected:
	FTransform Transform;
	FTransform WorldTransform;
	bool ShouldUpdateTransform;
};
