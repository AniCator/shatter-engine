// Copyright © 2017, Christiaan Bakker, All rights reserved.
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
        Output.reserve( Input.size() );

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

    static std::string Escape(
        const std::string& Input
    )
    {
        std::string Output;
        Output.reserve( Input.size() );

        // Handles backslashes first.
        Output = Replace( Input, "\\", "\\\\" );

        // Replace all other escape cases.
        Output = Replace( Output, "\'", "\\\'" );
        Output = Replace( Output, "\"", "\\\"" );
        Output = Replace( Output, "\t", "\\\t" );
        Output = Replace( Output, "\r", "\\\r" );
        Output = Replace( Output, "\n", "\\\n" );

        return Output;
    }

    static std::string Unescape(
        const std::string& Input
    )
    {
        std::string Output;
        Output.reserve( Input.size() );

        // Replace all other escape cases, reverse order of String::Escape.
        Output = Replace(  Input, "\\\n", "\n" );
        Output = Replace( Output, "\\\r", "\r" );
        Output = Replace( Output, "\\\t", "\t" );
        Output = Replace( Output, "\\\"", "\"" );
        Output = Replace( Output, "\\\'", "\'" );

        // Handles backslashes last.
        Output = Replace( Output, "\\\\", "\\" );

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

    static std::vector<std::string> Segment(
        const std::string& Input,
        const char Delimiter
    )
    {
        size_t StartIndex = 0;
        size_t Index = Input.find( Delimiter );

        // Delimiter wasn't found.
        if( Index == std::string::npos )
            return { Input };

        // Check if the index isn't at the end of the line.
        const auto Length = Input.length();
        if( Index >= Length )
            return { Input };

        std::vector<std::string> Entries;

        while( Index != std::string::npos && StartIndex != Index )
        {
            std::string Entry = Input.substr( StartIndex, Index - StartIndex );
            Entries.emplace_back( Entry );

            StartIndex = Index + 1;
            Index = Input.find( Delimiter, StartIndex );
        }

        if( Index < Length )
        {
            std::string Remainder = Input.substr( Index + 1, Length - Index );
            if( !Remainder.empty() )
            {
                Entries.emplace_back( Remainder );
            }
        }

        return Entries;
    }

    static std::string TrimL(
        const std::string& Input,
        const char Trim
    )
    {
        bool Active = true;
        std::string Output;

        auto Iterator = Input.begin();
        while( Iterator != Input.end() )
        {
            const char Character = *Iterator;
            Iterator++;

            if( Character != Trim )
                Active = false; // We've hit a character we don't want to trim, deactivate the trimmer.

            // Skip over characters we'd like to trim.
            if( Active && Character == Trim )
                continue;

            Output += Character;
        }

        return Output;
    }

    static std::string TrimR(
        const std::string& Input,
        const char Trim
    )
    {
        bool Active = true;
        std::string Output;

        auto Iterator = Input.rbegin();
        while( Iterator != Input.rend() )
        {
            const char Character = *Iterator;
            Iterator++;

            if( Character != Trim )
                Active = false; // We've hit a character we don't want to trim, deactivate the trimmer.

            // Skip over characters we'd like to trim.
            if( Active && Character == Trim )
                continue;

            Output += Character;
        }

        // Flip the string back around.
        std::reverse( Output.begin(), Output.end() );

        return Output;
    }

    static std::string LowerCase(
        const std::string& Input
    )
    {
        std::string Lower = Input;
        std::transform( Lower.begin(), Lower.end(), Lower.begin(), ::tolower );
        return Lower;
    }

    static std::string UpperCase(
        const std::string& Input
    )
    {
        std::string Upper = Input;
        std::transform( Upper.begin(), Upper.end(), Upper.begin(), ::toupper );
        return Upper;
    }
};
