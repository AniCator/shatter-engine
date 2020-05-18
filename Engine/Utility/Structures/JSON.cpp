// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "JSON.h"

#include <Engine/Profiling/Logging.h>

#include <algorithm>

namespace JSON
{
	void PopString( const char*& Token, const char*& Start, const char*& End, size_t& Length )
	{
		Token++;
		Start = Token;
		while( Token[0] != '"' )
		{
			Token++;
		}
		End = Token;
		Length = End - Start;
	}

	static const size_t SpecialTokensCount = 7;
	static const char SpecialTokens[SpecialTokensCount] = {
		'{', '}',
		'[', ']',
		'"', ':', ','
	};

	void UpdateSpecialToken( char& Special, const char* Token)
	{
		for( size_t Index = 0; Index < SpecialTokensCount; Index++ )
		{
			if( Token[0] == SpecialTokens[Index] )
			{
				Special = Token[0];
				return;
			}
		}
	}

	void CheckForSyntaxErrors( char& Special, size_t& Line, const char* Token )
	{
		if( Special == ',' )
		{
			Log::Event( Log::Warning, "Syntax error on line %zu: Too many commas, my friend.\n", Line );
		}
	}

	Container GenerateTree( const CFile& File )
	{
		Container Container;

		const char* Data = File.Fetch<char>();
		size_t Count = 0;
		const size_t Length = File.Size();

		char LastSpecialToken = ' ';
		size_t Line = 0;

		const char* Token = Data;
		bool Finished = false;
		size_t Depth = 0;
		size_t ArrayDepth = 0;

		bool Entry = true;
		Object* Current = nullptr;
		Object* Parent = nullptr;
		while( !Finished )
		{
			if( Token[0] == '{' )
			{
				Depth++;
				Entry = true;

				if( Current )
				{
					Current->IsObject = true;

					if( Depth > 1 )
					{
						Object* StackParent = Current;
						Object Object;
						Container.Objects.push_back( Object );

						Current = &Container.Objects[Container.Objects.size() - 1];

						if( Parent )
						{
							Current->Parent = Parent;
							Current->Parent->Objects.push_back( Current );
						}
						else if( StackParent )
						{
							Current->Parent = StackParent;
							Current->Parent->Objects.push_back( Current );
						}

						Parent = Current;
					}
				}
			}
			else if( Token[0] == '}' )
			{
				CheckForSyntaxErrors( LastSpecialToken, Line, Token );
				Depth--;
				Entry = true;

				if( Parent )
				{
					Parent = Parent->Parent;
				}

				if( Depth == 0 && ArrayDepth == 0 )
				{
					Finished = true;
				}
			}
			else if( Token[0] == '"' )
			{
				const char* Start = nullptr;
				const char* End = nullptr;
				size_t Length = 0;
				PopString( Token, Start, End, Length );

				if( Entry && Start && End )
				{
					Object Object;
					Container.Objects.push_back( Object );

					Current = &Container.Objects[Container.Objects.size() - 1];
					if( Current )
					{
						Current->Key = std::string( Start, End );

						if( Parent )
						{
							Current->Parent = Parent;
							Current->Parent->Objects.push_back( Current );
						}
					}
				}
				else
				{
					if( Current )
					{
						if( !Current->IsObject )
						{
							Current->Value = std::string( Start, End );
						}

						if( Current->Parent )
						{
							Current = Current->Parent;
						}
					}

				}
			}
			else if( Token[0] == ':' )
			{
				Entry = false;
			}
			else if( Token[0] == ',' )
			{
				Entry = true;
			}
			else if( Token[0] == '[' )
			{
				ArrayDepth++;
				Entry = true;

				Parent = Current;
			}
			else if( Token[0] == ']' )
			{
				CheckForSyntaxErrors( LastSpecialToken, Line, Token );
				ArrayDepth--;
				Entry = true;

				if( Parent )
				{
					Parent = Parent->Parent;
				}
			}
			else if( Token[0] == '\n' )
			{
				Line++;
			}

			UpdateSpecialToken( LastSpecialToken, Token );

			Token++;
		}

		RegenerateTree( Container );

		return Container;
	}

	void RegenerateTree( Container& Tree )
	{
		Tree.Tree.clear();

		for( auto& Object : Tree.Objects )
		{
			if( !Object.Parent )
			{
				Tree.Tree.push_back( &Object );
			}
		}
	}

	void StringifyObject( std::stringstream& Stream, Object* Object, const uint32_t Offset, const bool Last )
	{
		std::string OffsetString;

		for( uint32_t Index = 0; Index < Offset; Index++ )
		{
			OffsetString += "\t";
		}

		if( Object->Objects.size() > 0 )
		{
			const bool HasKey = Object->Key.length() != 0;

			if( HasKey )
			{
				Stream << OffsetString << "\"" << Object->Key << "\" : ";
				Stream << "[\n";
			}
			else
			{
				Stream << OffsetString << "{\n";
			}

			uint32_t Iterator = 1;
			for( uint32_t Index = 0; Index < Object->Objects.size(); Index++ )
			{
				uint32_t NewOffset = Offset + 1;
				StringifyObject( Stream, Object->Objects[Index], NewOffset, Index == ( Object->Objects.size() - 1 ) );
			}

			if( HasKey )
			{
				Stream << OffsetString << "]\n";
			}
			else
			{
				if( Last )
				{
					Stream << OffsetString << "}\n";
				}
				else
				{
					Stream << OffsetString << "},\n";
				}
			}
		}
		else
		{
			if( Object->Value.length() > 0 )
			{
				Stream << OffsetString << "\"" << Object->Key << "\" : \"" << Object->Value;
			}
			else
			{
				Stream << OffsetString << "\"" << Object->Key;
			}

			if( Last )
			{
				Stream << "\"\n";
			}
			else
			{
				Stream << "\",\n";
			}
			
		}
	}

	std::string ExportTree( Container& Tree )
	{
		RegenerateTree( Tree );

		std::stringstream Stream;

		Stream << "{\n";

		for( uint32_t Index = 0; Index < Tree.Tree.size(); Index++ )
		{
			StringifyObject( Stream, Tree.Tree[Index], 1, Index == ( Tree.Tree.size() - 1 ) );
		}

		Stream << "}";

		return Stream.str();
	}

	void IntegrateBranch( Container& Container, Object* Branch )
	{
		if( Branch->Objects.size() > 0 )
		{
			for( auto& Twig : Branch->Objects )
			{
				Object NewObject = *Twig;
				NewObject.Parent = nullptr;
				NewObject.Objects.clear();
				Container.Objects.emplace_back( NewObject );

				IntegrateBranch( Container, &Container.Objects[Container.Objects.size() - 1] );
			}
		}
		else
		{
			Object NewObject = *Branch;
			NewObject.Parent = nullptr;
			Container.Objects.emplace_back( NewObject );
		}
	}

	Container& Container::operator+=( const Container& Container )
	{
		for( auto& Branch : Container.Tree )
		{
			IntegrateBranch( *this, Branch );
		}

		RegenerateTree( *this );

		return *this;
	}

	void CopyTree( Object* Source, Object* Target )
	{
		Target->Key = Source->Key;
		Target->Value = Source->Value;
		Target->IsObject = Source->IsObject;
		Target->Objects.clear();

		if( Source->Objects.size() > 0 )
		{
			for( auto& SourceObject : Source->Objects )
			{
				Object TargetObject;
				CopyTree( SourceObject, &TargetObject );

				TargetObject.Parent = Target;
				// Target->Objects.emplace_back( TargetObject );
				// TODO: Container isn't known and the container stores all the objects.
			}
		}
	}

	Object& Object::operator=( const Container& Container )
	{
		for( auto& Branch : Container.Tree )
		{
			CopyTree( Branch, this );
		}

		return *this;
	}
}
