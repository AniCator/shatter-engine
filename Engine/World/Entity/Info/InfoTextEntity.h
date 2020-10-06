// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/Logic/LogicPointEntity/LogicPointEntity.h>
#include <Engine/Utility/Math.h>

class CInfoText : public CPointEntity
{
public:
	CInfoText();

	void Tick() override;
	void Load( const JSON::Vector& Objects ) override;

	void Import( CData& Data ) override;
	void Export( CData& Data ) override;

protected:
	std::string Text;
};
