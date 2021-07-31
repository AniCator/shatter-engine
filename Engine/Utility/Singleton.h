// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

template<class T>
class Singleton
{
public:
	static T& Get()
	{
		static T StaticInstance;
		return StaticInstance;
	}

	Singleton( Singleton const& ) = delete;
	void operator=( Singleton const& ) = delete;
protected:
	Singleton() = default;
};