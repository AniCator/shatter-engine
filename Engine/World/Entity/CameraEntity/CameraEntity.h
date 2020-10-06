// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Display/Rendering/Camera.h>

struct CameraKey
{
	FCameraSetup Setup;
	float Time;
};

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
	std::vector<CameraKey> Keys;

	bool Active = false;
	float StartTime = 0.0f;
	
	int Target = -1;
	uint32_t Priority = 501;
};
