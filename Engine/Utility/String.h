// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

struct String
{
    /// <summary>
    /// Finds occuranes of a given string and replaces it with another.
    /// </summary>
    /// <param name="Input">The string that will be searched.</param>
    /// <param name="Find">Text we'll be searching for.</param>
    /// <param name="Replace">Text that will replace the found string segments.</param>
    /// <returns></returns>
    static std::string Replace( 
        const std::string& Input, 
        const std::string& Find,
        const std::string& Replace 
    )
    {
        // There is nothing to search for, just return the input string.
        if( Find.empty() )
            return Input;

        std::string Result;
        size_t Start = 0;
        size_t End = Input.find( Find, Start );

        // Find all instances of the search string.
        while( End != std::string::npos ) 
        {
            // Append the input segment, up until the position of the search string, then append the replacement string.
            Result += Input.substr( Start, End - Start ) + Replace;

            // Update the start position to the position of the search string and jump over it.
            Start = End + Find.length();

            // Look for the next occurance of the search string.
            End = Input.find( Find, Start );
        }

        // Append the remainder.
        Result += Input.substr( Start, Input.length() - Start );
        return Result;
    }
};
