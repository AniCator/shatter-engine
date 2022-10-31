// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>

class CLogicSequenceEntity : public CPointEntity
{
public:
	CLogicSequenceEntity();

	void Tick() override;
	void Destroy() override;

	void Load( const JSON::Vector& Objects ) override;
	void Reload() override;

	void Export( CData& Data ) override;
	void Import( CData& Data ) override;

protected:
	std::string SequenceAsset;
	class CSequence* Sequence = nullptr;

	bool IsPlaying = false;
	bool UseTransform = false;
};
