// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Bus.h"
#include "Effect.h"

struct FilterStack
{
	FilterStack()
	{
		Stack.reserve( 8 );
	}

	void Add( SoLoud::Filter* Filter, const std::vector<float>& Parameters, const std::string& Name = "" );
	void Remove( const Effect* Effect );
	void SetForBus( const Bus::Type& Bus );
	void UpdateForBus( const Bus::Type& Bus );

	const std::vector<Effect>& Get() const
	{
		return Stack;
	}

private:
	std::vector<Effect> Stack;
	Bus::Type LatestBus = Bus::Maximum;
};