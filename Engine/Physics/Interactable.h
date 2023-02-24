// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class Interactable
{
public:
	virtual void Interact( Interactable* Caller ) = 0;
	virtual bool CanInteract( Interactable* Caller ) const = 0;
};
