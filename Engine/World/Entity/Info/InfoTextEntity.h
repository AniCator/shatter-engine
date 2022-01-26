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

	void Debug() override;

protected:
	float Distance = 10.0f;
	std::string Title;
	std::string Text;

	// Should we only show the text once?
	bool Once = false;

	// Indicates whether we have displayed the text.
	bool Expired = false;

	enum Mode
	{
		World,
		Screen
	};

	Mode Style = World;
};
