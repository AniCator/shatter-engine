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

	void Construct() override;
	void Tick() override;
	void Destroy() override;
	void Load( const JSON::Vector& Objects ) override;
	void Debug() override;

	void Activate();
	void Deactivate();

	void Export( CData& Data ) override;
	void Import( CData& Data ) override;

protected:
	CCamera Camera;
	std::vector<CameraKey> Keys;

	bool Active = false;
	float StartTime = 0.0f;
	
	int Target = -1;
	uint32_t Priority = 501;
};
