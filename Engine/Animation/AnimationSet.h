// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Animation/Skeleton.h>
#include <Engine/Utility/Structures/JSON.h>

#include <unordered_map>
#include <string>

struct AnimationSet
{
	static AnimationSet Generate( const std::string& Path )
	{
		CFile File( Path );
		if( File.Exists() )
		{
			File.Load();
			const auto& Data = JSON::Tree( File );
			return Generate( JSON::Find( Data.Tree, "animations" ) );
		}

		return AnimationSet();
	}
	
	static AnimationSet Generate( const JSON::Object* Node )
	{
		AnimationSet Set;		
		if( !Node )
			return Set;

		Set.Set.reserve( Node->Objects.size() );
		
		for( const auto& Object : Node->Objects )
		{
			Set.Set.insert_or_assign( Object->Key, Object->Value );
		}

		return Set;
	}

	const std::string& Lookup( const std::string& Key ) const
	{
		const auto Result = Set.find( Key );
		if( Result != Set.end() )
		{
			return Result->second;
		}

		return Key;
	}

	bool Lookup( const std::string& Key, Animation& Animation ) const
	{
		const auto& Result = Lookup( Key );
		const auto& Iterator = Skeleton.Animations.find( Result );
		if( Iterator != Skeleton.Animations.end() )
		{
			Animation = Iterator->second;
			return true;
		}

		return false;
	}
	
	std::unordered_map<std::string, std::string> Set;
	Skeleton Skeleton = ::Skeleton();
};
