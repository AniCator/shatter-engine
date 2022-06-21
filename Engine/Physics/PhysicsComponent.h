// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class Interactable
{
public:
	virtual void Interact( Interactable* Caller ) = 0;
};
