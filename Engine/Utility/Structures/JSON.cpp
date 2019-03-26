// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "JSON.h"

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

	Container GenerateTree( const CFile& File )
	{
		Container Container;

		const char* Data = File.Fetch<char>();
		size_t Count = 0;
		const size_t Length = File.Size();

		const char* Token = Data;
		bool Finished = false;
		size_t Depth = 0;
		size_t ArrayDepth = 0;

		bool Entry = true;
		Object* Current = nullptr;
		Object* PreviousArrayParent = nullptr;
		Object* PreviousObjectParent = nullptr;
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
						Object Object;
						Container.Objects.push_back( Object );

						Current = &Container.Objects[Container.Objects.size() - 1];

						if( Parent )
						{
							Current->Parent = Parent;
							Current->Parent->Objects.push_back( Current );
						}

						PreviousObjectParent = Parent;
						Parent = Current;
					}
				}
			}
			else if( Token[0] == '}' )
			{
				Depth--;
				Entry = true;

				Parent = PreviousObjectParent;

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

				PreviousArrayParent = Parent;
				Parent = Current;
			}
			else if( Token[0] == ']' )
			{
				ArrayDepth--;
				Entry = true;

				Parent = PreviousArrayParent;
			}

			Token++;
		}

		for( auto& Object : Container.Objects )
		{
			if( !Object.Parent )
			{
				Container.Tree.push_back( &Object );
			}
		}

		return Container;
	}
}
