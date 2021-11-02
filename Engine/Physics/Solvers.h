// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class CBody;
struct CollisionResponse;

namespace Solve
{
	enum Type
	{
		Position
	};

	void Solve( CBody* Body, const CollisionResponse& Response, const Type& Type );

	// Solvers
	void SolvePosition( CBody* Body, const CollisionResponse& Response );
}