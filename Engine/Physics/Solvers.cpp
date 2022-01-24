// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Solvers.h"

#include <Engine/Physics/Body/Body.h>

void Solve::Solve( CBody* Body, const CollisionResponse& Response, const Type& Type )
{
	if( Type == Position )
	{
		SolvePosition( Body, Response );
	}
}

void Solve::SolvePosition( CBody* Body, const CollisionResponse& Response )
{
	if( Response.Distance > 0.0f && !Body->Static && !Body->Stationary )
	{
		const auto Penetration = ( Response.Normal * Response.Distance );

		auto Transform = Body->GetTransform();
		Transform.SetPosition( Transform.GetPosition() - Penetration );
		Body->SetTransform( Transform );

		Body->Normal += Penetration * -1.0f;
	}
}
