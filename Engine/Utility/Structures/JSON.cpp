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

		bool LookingForKeyEntry = true;
		Object* Current = nullptr;
		Object* Parent = nullptr;
		while( !Finished && ( Token < ( Data + Length ) ) )
		{
			if( Token[0] == '{' )
			{
				// Entering object.
				Depth++;

				// Check if the current node is a field.
				const bool IsField = Current && Current->IsField;
				const bool IsArray = Current && Current->IsArray;

				// Check if we're the top-level object and not a field.
				if( Depth > 1 )
				{
					if( !IsField )
					{
						Object* StackParent = Current;
						Object Object;
						Container.Objects.emplace_back( Object );

						// Create a new node for the object.
						Current = &Container.Objects.back();
						Current->IsObject = true;

						if( Parent )
						{
							// Assign the stored parent node as the parent of the new node.
							Current->Parent = Parent;
							Current->Parent->Objects.emplace_back( Current );
						}
						else if( StackParent )
						{
							// Assign the previous current node as the parent of the new node.
							Current->Parent = StackParent;
							Current->Parent->Objects.emplace_back( Current );
						}

						// Parent = Current->Parent;
					}
					
					Parent = Current;
					Current = nullptr;
				}

				LookingForKeyEntry = true;
			}
			else if( Token[0] == '}' )
			{
				// Exiting object.
				CheckForSyntaxErrors( LastSpecialToken, Line, Token );
				Depth--;
				LookingForKeyEntry = true;

				// Close the object.
				if( Current )
				{
					Current->IsField = false;
					Current->IsObject = true;
				}

				// If we have a parent, we want to set our parent to the level above that.
				if( Parent )
				{
					Current = Parent;
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

				if( LookingForKeyEntry && Start && End )
				{
					Object Object;
					Container.Objects.emplace_back( Object );

					Current = &Container.Objects.back();
					if( Current )
					{
						Current->IsField = true;
						
						Current->Key = std::string( Start, End );

						if( Parent )
						{
							Current->Parent = Parent;
							Current->Parent->Objects.emplace_back( Current );
						}
					}
				}
				else
				{
					if( Current )
					{
						if( Current->IsField )
						{
							Current->Value = std::string( Start, End );
							LookingForKeyEntry = true;
						}
					}
				}
			}
			else if( Token[0] == ':' )
			{
				LookingForKeyEntry = false;
			}
			else if( Token[0] == ',' )
			{
				LookingForKeyEntry = true;
			}
			else if( Token[0] == '[' )
			{
				ArrayDepth++;
				LookingForKeyEntry = true;

				if( Current )
				{
					Current->IsArray = true;
					Current->IsField = false;
				}

				Parent = Current;
				Current = nullptr;
			}
			else if( Token[0] == ']' )
			{
				CheckForSyntaxErrors( LastSpecialToken, Line, Token );
				ArrayDepth--;
				LookingForKeyEntry = true;

				if( Parent )
				{
					Current = Parent;
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

		Container.Regenerate();

		return Container;
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

			// uint32_t Iterator = 1;
			/*for( uint32_t Index = 0; Index < Object->Objects.size(); Index++ )
			{
				uint32_t NewOffset = Offset + 1;
				StringifyObject( Stream, Object->Objects[Index], NewOffset, Index == ( Object->Objects.size() - 1 ) );
			}*/

			for( auto& Iterator : Object->Objects )
			{
				const uint32_t NewOffset = Offset + 1;
				StringifyObject( Stream, Iterator, NewOffset, Iterator == Object->Objects.back() );
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
			if( Object->Value.length() > 0 || true )
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

	static uint16_t TabOffset = 0;
	void RecurseJSONObject( JSON::Object* Object )
	{
		if( !Object )
			return;

		std::string TabString;
		for( auto Index = 0; Index < TabOffset; Index++ )
		{
			TabString += "---";
		}

		if( Object->Objects.empty() )
		{
			const std::string ObjectKey = TabString + std::string( Object->Key.begin(), Object->Key.end() );
			Log::Event( "%s\n", ObjectKey.c_str() );

			const std::string ObjectValue = TabString + std::string( Object->Value.begin(), Object->Value.end() );
			Log::Event( "%s\n", ObjectValue.c_str() );
		}
		else
		{
			const std::string ObjectKey = TabString + std::string( Object->Key.begin(), Object->Key.end() ) + " (" + std::to_string( Object->Objects.size() ) + ")";
			Log::Event( "%s\n", ObjectKey.c_str() );

			TabOffset++;
			for( const auto& SubObject : Object->Objects )
			{
				RecurseJSONObject( SubObject );
			}
			TabOffset--;
		}
	}

	void PrintTree( const Container& Tree )
	{
		for( const auto& Object : Tree.Tree )
		{
			RecurseJSONObject( Object );
		}
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

				IntegrateBranch( Container, &Container.Objects.back() );
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

		Regenerate();

		return *this;
	}

	Object* Container::Allocate()
	{
		Object Object;
		Objects.emplace_back( Object );

		return &Objects.back();
	}

	void Container::Add( Object* Parent, Object* Child )
	{
		if( !Parent )
			return;

		// Check if the parent belongs to this container.
		if( !Valid( Parent ) )
			return;


		// Check if the child belongs to this container.
		if( !Valid( Child ) )
			return;

		Child->Parent = Parent;
		Parent->Objects.emplace_back( Child );
		Regenerate();
	}

	Object* Container::Add( Object* Parent, const std::string& Key )
	{
		if( !Parent || Key.length() == 0 )
			return nullptr;

		// Check if the parent belongs to this container.
		if( !Valid( Parent ) )
			return nullptr;

		auto* Child = Allocate();

		Child->Parent = Parent;
		Child->Key = Key;
		Parent->Objects.emplace_back( Child );
		Regenerate();

		return Child;
	}

	void Container::Regenerate()
	{
		Tree.clear();

		for( auto& Object : Objects )
		{
			if( !Object.Parent )
			{
				Tree.emplace_back( &Object );
			}
		}
	}

	std::string Container::Export()
	{
		Regenerate();

		std::stringstream Stream;

		Stream << "{\n";

		for( auto& Iterator : Tree )
		{
			StringifyObject( Stream, Iterator, 1, Iterator == Tree.back() );
		}

		Stream << "}";

		return Stream.str();
	}

	bool Container::Valid( Object* Object ) const
	{
		for( const auto& ContainerObject : Objects )
		{
			if( &ContainerObject == Object )
				return true;
		}

		return false;
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
