// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class CEntity
{
public:
	CEntity() {};
	virtual ~CEntity() {};

	virtual void Construct() {};
	virtual void Tick() {};
	virtual void Destroy() {};
};
