// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class IEngineService
{
public:
	// Virtual destructor
	virtual ~IEngineService() = default;

	// Move support
	IEngineService( IEngineService&& ) = default;
	IEngineService& operator=( IEngineService&& ) & = default;

	// Copy support
	IEngineService( const IEngineService& ) = default;
	IEngineService& operator=( const IEngineService& ) & = default;

protected:
	IEngineService() = default;
};
