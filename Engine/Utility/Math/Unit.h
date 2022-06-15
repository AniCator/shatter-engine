// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

namespace Unit
{
	template<typename T>
	struct Unit
	{
		T MetersPerSecond;
		T KilometersPerHour;
	};

	constexpr double KMHtoMS = 3.6;

	template<typename T>
	constexpr Unit<T> KilometersPerHour( const T& Value )
	{
		return {
			static_cast<T>( static_cast<double>( Value ) / KMHtoMS ),
			Value
		};
	}

	template<typename T>
	constexpr Unit<T> MetersPerSecond( const T& Value )
	{
		return {
			Value,
			static_cast<T>( static_cast<double>( Value ) * KMHtoMS )
		};
	}
}
