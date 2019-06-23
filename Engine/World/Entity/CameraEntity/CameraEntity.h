// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Display/Rendering/Camera.h>

class CCameraEntity : public CPointEntity
{
public:
	CCameraEntity();

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;
	virtual void Load( const JSON::Vector& Objects ) override;

	void Activate();
	void Deactivate();

	virtual void Export( CData& Data ) override;
	virtual void Import( CData& Data ) override;

protected:
	CCamera Camera;
	uint32_t Priority;
	bool Active;
};
