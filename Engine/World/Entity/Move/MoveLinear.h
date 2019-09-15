// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

class CMoveLinearEntity : public CMeshEntity
{
public:
	CMoveLinearEntity();

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;
	virtual void Load( const JSON::Vector& Objects ) override;

	virtual void Export( CData& Data ) override;
	virtual void Import( CData& Data ) override;

private:
	Vector3D Start;
	Vector3D Direction;
	float Distance;
	float Speed;
	bool MoveTo;
};
