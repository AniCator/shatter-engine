// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "JSON.h"

#include <Engine/Profiling/Logging.h>

#include <algorithm>

namespace JSON
{
	void Object::SetValue( const std::string& Key, const std::string& NewValue )
	{
		if( this->Key == Key )
		{
			Value = NewValue;
			return;
		}

		if( Object* Object = Find( Key ) )
		{
			Object->Value = NewValue;
		}
	}

	const std::string& Object::GetValue( const std::string& Key ) const
	{
		if( this->Key == Key )
		{
			return Value;
		}

		if( const auto* Object = Find( Key ) )
		{
			return Object->Value;
		}

		const static std::string EmptyString;
		return EmptyString;
	}

	Object& Object::Add()
	{
		auto* Object = Owner->Allocate();
		Owner->Add( this, Object );
		return *Object;
	}

	Object* Object::operator[]( const std::string& Key ) const
	{
		return Find( Key );
	}

	Object& Object::operator[]( const std::string& Key )
	{
		const auto Result = Find( Key );

		// Entry doesn't exist, create it.
		if( !Result )
		{
			return *Owner->Add( this, Key );
		}

		return *Result;
	}

	Object& Object::operator[]( const Object& Object )
	{
		// Copy the object into this one.
		auto& New = Add();
		New = Object;
		return New;
	}

	Object& Object::operator=( const char* Value )
	{
		this->Value = Value;
		return *this;
	}

	Object& Object::operator=( const std::string& Value )
	{
		this->Value = Value;
		return *this;
	}

	void CopyTree( Object* Source, Object* Target );
	Object& Object::operator=( const Container& Container )
	{
		for( auto& Branch : Container.Tree )
		{
			CopyTree( Branch, this );
		}

		return *this;
	}

	Object* Object::Find( const std::string& Key ) const
	{
		const auto Result = std::find_if( Objects.begin(), Objects.end(), [&Key]( Object* Item ) -> bool
		{
			return Item->Key == Key;
		} );

		if( Result != Objects.end() )
		{
			return *Result;
		}

		return nullptr;
	}

	void PopString( const char*& Token, const char*& Start, const char*& End, size_t& Length )
	{
		Token++;
		Start = Token;
		while( Token[0] != '"' )
		{
			if( Token[0] == '\\' ) // Escape character detected.
				Token++; // Hop over an additional token.
			
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

	Container Tree( const char* Data, const size_t& Length )
	{
		Container Container;

		// const char* Data = File.Fetch<char>();
		size_t Count = 0;

		char LastSpecialToken = ' ';
		size_t Line = 0;

		const char* Token = Data;
		const char* End = Data + Length;
		bool Finished = false;
		size_t Depth = 0;
		size_t ArrayDepth = 0;

		bool LookingForKeyEntry = true;
		Object* Current = nullptr;
		Object* Parent = nullptr;
		while( !Finished && ( Token < End ) )
		{
			switch( Token[0] )
			{
			case '{':
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
						Object.Owner = &Container;
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
				break;
			}
			case '}':
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

				break;
			}
			case '"':
			{
				const char* Start = nullptr;
				const char* End = nullptr;
				size_t Length = 0;
				PopString( Token, Start, End, Length );

				if( LookingForKeyEntry && Start && End )
				{
					Object Object;
					Object.Owner = &Container;
					Container.Objects.emplace_back( Object );

					Current = &Container.Objects.back();
					if( Current )
					{
						Current->IsField = true;

						Current->Key = std::string( Start, End );
						Current->Key = String::Unescape( Current->Key );

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
							Current->Value = String::Unescape( Current->Value );
							LookingForKeyEntry = true;
						}
					}
				}
				break;
			}
			case ':':
			{
				LookingForKeyEntry = false;
				break;
			}
			case ',':
			{
				LookingForKeyEntry = true;
				break;
			}
			case '[':
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
				break;
			}
			case ']':
			{
				CheckForSyntaxErrors( LastSpecialToken, Line, Token );
				ArrayDepth--;
				LookingForKeyEntry = true;

				if( Parent )
				{
					Current = Parent;
					Parent = Parent->Parent;
				}
				break;
			}
			case '\n':
			{
				Line++;
				break;
			}
			case '/': // Support comments because, why not.
			{
				bool IsComment = false;
				if( ( Token + 1 ) < End )
				{
					IsComment = Token[1] == '/';
				}

				if( IsComment )
				{
					// Flush line.
					bool Flushed = false;
					while( !Flushed && Token < End )
					{
						if( Token[0] != '\n' )
						{
							Token++;
						}
						else
						{
							// Back up one token so the parsing loop can handle it.
							Token--;
							Flushed = true;
						}
					}
				}
			}
			default:
			{
				break;
			}
			}

			UpdateSpecialToken( LastSpecialToken, Token );

			Token++;
		}

		Container.Regenerate();

		return Container;
	}

	Container Tree( const CFile& File )
	{
		return Tree( File.Fetch<char>(), File.Size() );
	}

	Container Tree( const std::string& Data )
	{
		return Tree( Data.c_str(), Data.size() );
	}

	void Stringify( std::stringstream& Stream, const Object* Object, const uint32_t Offset, const bool Last )
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
				Stream << OffsetString << "\"" << String::Escape( Object->Key ) << "\" : ";
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
				Stringify( Stream, Iterator, NewOffset, Iterator == Object->Objects.back() );
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
				// std::string Value = Object->Value;
				// Value.replace( "\"", "\\\"" );
				Stream << OffsetString << "\"" << String::Escape( Object->Key ) << "\" : \"" << String::Escape( Object->Value );
			}
			else
			{
				Stream << OffsetString << "\"" << String::Escape( Object->Key );
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
				NewObject.Owner = &Container;
				NewObject.Parent = nullptr;
				NewObject.Objects.clear();
				Container.Objects.emplace_back( NewObject );

				IntegrateBranch( Container, &Container.Objects.back() );
			}
		}
		else
		{
			Object NewObject = *Branch;
			NewObject.Owner = &Container;
			NewObject.Parent = nullptr;
			Container.Objects.emplace_back( NewObject );
		}
	}

	std::string Stringify( const JSON::Object* Object )
	{
		std::stringstream Stream;
		Stringify( Stream, Object, 1, true );

		return Stream.str();
	}

	Object& Container::operator[]( const std::string& Key )
	{
		const auto Result = std::find_if( Tree.begin(), Tree.end(), [&Key]( Object* Item ) -> bool
		{
			return Item->Key == Key;
		}
		);

		if( Result == Tree.end() )
		{
			Object Object;
			Object.Owner = this;
			Object.Key = Key;
			Objects.emplace_back( Object );

			Regenerate();
			return Objects.back();
		}

		return **Result;
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
		Object.Owner = this;
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

		for( const auto& Iterator : Tree )
		{
			Stringify( Stream, Iterator, 1, Iterator == Tree.back() );
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

		if( Source->Objects.empty() )
			return;

		for( auto* SourceObject : Source->Objects )
		{
			Object TargetObject;
			CopyTree( SourceObject, &TargetObject );

			TargetObject.Parent = Target;
			// Target->Objects.emplace_back( TargetObject );
			// TODO: Container isn't known and the container stores all the objects.
		}
	}

	void CopyObject( Container& Target, Object* Source, Object* Parent = nullptr )
	{
		auto* Child = Target.Allocate();

		// Copy the information from the source object to the new child.
		Child->Parent = Parent;
		Child->Key = Source->Key;
		Child->Value = Source->Value;
		Child->IsObject = Source->IsObject;
		Child->Objects.clear();

		// Check if we have a parent.
		if( Parent )
		{
			// Add the child to the parent.
			Parent->Objects.emplace_back( Child );
		}

		// Recursively copy any children.
		for( auto* SourceObject : Source->Objects )
		{
			CopyObject( Target, SourceObject, Child );
		}
	}

	Container::Container( Container const& Source )
	{
		if( this == &Source )
			return;

		Objects.clear();
		Tree.clear();

		for( auto& Branch : Source.Tree )
		{
			CopyObject( *this, Branch );
		}

		Regenerate();
	}

	Container& Container::operator=( Container const& Source )
	{
		if( this == &Source )
			return *this;

		Objects.clear();
		Tree.clear();

		for( auto& Branch : Source.Tree )
		{
			CopyObject( *this, Branch );
		}

		Regenerate();

		return *this;
	}
}
