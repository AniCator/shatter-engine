// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

struct Date
{
	size_t Year = 0;
	size_t Month = 0;
	size_t Day = 0;

	size_t Hour = 0;
	size_t Minute = 0;
	size_t Second = 0;
};

inline Date GetDate()
{
	Date Date;
	Date.Year = 
		( __DATE__[7] - '0' ) * 1000 +
		( __DATE__[8] - '0' ) * 100 +
		( __DATE__[9] - '0' ) * 10 +
		( __DATE__[10] - '0' );

	Date.Month = 
		( __DATE__[0] == 'J' ) && ( __DATE__[1] == 'a' ) && ( __DATE__[2] == 'n' ) ? 1 :
		( __DATE__[0] == 'F' ) && ( __DATE__[1] == 'e' ) && ( __DATE__[2] == 'b' ) ? 2 :
		( __DATE__[0] == 'M' ) && ( __DATE__[1] == 'a' ) && ( __DATE__[2] == 'r' ) ? 3 :
		( __DATE__[0] == 'A' ) && ( __DATE__[1] == 'p' ) && ( __DATE__[2] == 'r' ) ? 4 :
		( __DATE__[0] == 'M' ) && ( __DATE__[1] == 'a' ) && ( __DATE__[2] == 'y' ) ? 5 :
		( __DATE__[0] == 'J' ) && ( __DATE__[1] == 'u' ) && ( __DATE__[2] == 'n' ) ? 6 :
		( __DATE__[0] == 'J' ) && ( __DATE__[1] == 'u' ) && ( __DATE__[2] == 'l' ) ? 7 :
		( __DATE__[0] == 'A' ) && ( __DATE__[1] == 'u' ) && ( __DATE__[2] == 'g' ) ? 8 :
		( __DATE__[0] == 'S' ) && ( __DATE__[1] == 'e' ) && ( __DATE__[2] == 'p' ) ? 9 :
		( __DATE__[0] == 'O' ) && ( __DATE__[1] == 'c' ) && ( __DATE__[2] == 't' ) ? 10 :
		( __DATE__[0] == 'N' ) && ( __DATE__[1] == 'o' ) && ( __DATE__[2] == 'v' ) ? 11 :
		( __DATE__[0] == 'D' ) && ( __DATE__[1] == 'e' ) && ( __DATE__[2] == 'c' ) ? 12 : 0;

	Date.Day = 
		( ( __DATE__[4] >= '0' ? __DATE__[4] : '0' ) - '0' ) * 10 +
		( __DATE__[5] - '0' );

	Date.Hour = 
		( __TIME__[0] - '0' ) * 10 +
		( __TIME__[1] - '0' );

	Date.Minute = 
		( __TIME__[3] - '0' ) * 10 +
		( __TIME__[4] - '0' );

	Date.Second = 
		( __TIME__[6] - '0' ) * 10 +
		( __TIME__[7] - '0' );

	return Date;
}

inline std::string GetBuildDate()
{
	const auto Date = GetDate();
	return std::to_string( Date.Year ) + "-" + std::to_string( Date.Month ) + "-" + std::to_string( Date.Day );
}

inline std::string GetBuildTime()
{
	const auto Date = GetDate();
	return std::to_string( Date.Hour ) + ":" + std::to_string( Date.Minute ) + ":" + std::to_string( Date.Second );
}

inline std::string GetBuildDateTime()
{
	return GetBuildDate() + "T" + GetBuildTime();
}