// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class CEntity
{
public:
	virtual ~CEntity() = 0;

	virtual void Construct() = 0;
	virtual void Tick() = 0;
	virtual void Destroy() = 0;
};
