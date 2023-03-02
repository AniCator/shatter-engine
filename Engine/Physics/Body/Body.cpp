// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Body.h"

#include <Engine/Utility/TranslationTable.h>

static auto TranslateBodyType = Translate<std::string, BodyType>( {
			{ "triangle", BodyType::TriangleMesh },
			{ "plane", BodyType::Plane },
			{ "aabb", BodyType::AABB },
			{ "sphere", BodyType::Sphere }
	} );

BodyType ToBodyType( const std::string& Type )
{
	return TranslateBodyType.To( Type );
}

std::string FromBodyType( const BodyType& Type )
{
	return TranslateBodyType.From( Type );
}