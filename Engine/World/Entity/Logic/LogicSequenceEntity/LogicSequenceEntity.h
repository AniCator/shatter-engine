// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/Entity.h>

class CLogicSequenceEntity : public CEntity
{
public:
	CLogicSequenceEntity();

	virtual void Destroy() override;

	virtual void Load( const JSON::Vector& Objects ) override;
	virtual void Reload() override;

	virtual void Export( CData& Data ) override;
	virtual void Import( CData& Data ) override;

protected:
	std::string SequenceAsset;
	class CSequence* Sequence = nullptr;
};
