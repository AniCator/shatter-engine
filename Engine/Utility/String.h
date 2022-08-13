// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

struct String
{
    /// <summary>
    /// Finds occurances of a given string and replaces it with another.
    /// </summary>
    /// <param name="Input">The string that will be searched.</param>
    /// <param name="Find">Text we'll be searching for.</param>
    /// <param name="Replace">Text that will replace the found string segments.</param>
    /// <returns>String with all instances of the Find string replaced with the Replace string.</returns>
    static std::string Replace( 
        const std::string& Input, 
        const std::string& Find,
        const std::string& Replace 
    )
    {
        // There is nothing to search for, just return the input string.
        if( Find.empty() )
            return Input;

        size_t Start = 0;
        size_t End = Input.find( Find, Start );
        std::string Output;

        // Find all instances of the search string.
        while( End != std::string::npos ) 
        {
            // Append the input segment, up until the position of the search string, then append the replacement string.
            Output += Input.substr( Start, End - Start ) + Replace;

            // Update the start position to the position of the search string and jump over it.
            Start = End + Find.length();

            // Look for the next occurance of the search string.
            End = Input.find( Find, Start );
        }

        // Append the remaining string data.
        Output += Input.substr( Start, Input.length() - Start );
        return Output;
    }

    static std::pair<std::string, std::string> Split(
        const std::string& Input,
        const char Delimiter
    )
    {
        const size_t Index = Input.find( Delimiter );

        // Delimiter wasn't found.
        if( Index == std::string::npos )
            return std::make_pair( Input, "" );

        // Check if the index isn't at the end of the line.
        const auto Length = Input.length();
        if( Index >= Input.length() )
            return std::make_pair( Input, "" );

        std::string Left = Input.substr( 0, Index );
        std::string Right = Input.substr( Index + 1, Length - Index );
        return std::make_pair( Left, Right );
    }
};
