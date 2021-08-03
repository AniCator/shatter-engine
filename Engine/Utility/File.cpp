// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "File.h"
#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Math.h>

#include <filesystem>
#include <fstream>
#include <algorithm>

#pragma warning(disable:4996)

CFile::CFile( const char* FileLocationIn )
{
	Data = nullptr;
	FileLocation = FileLocationIn;
	Binary = false;

	FileExtension = FileLocation;
	const size_t ExtensionIndex = FileExtension.rfind( '.' );
	if( ExtensionIndex != std::string::npos )
	{
		FileLocationStripped = FileExtension.substr( 0, ExtensionIndex );
		FileExtension = FileExtension.substr( ExtensionIndex + 1 );
		std::transform( FileExtension.begin(), FileExtension.end(), FileExtension.begin(), ::tolower );
	}
	else
	{
		FileExtension = "";
	}

	Statistics = struct stat();
}

CFile::~CFile()
{
	if( Data )
	{
		delete[] Data;
		Data = nullptr;
	}
}

bool CFile::Load( bool InBinary )
{
	if( Data )
	{
		delete [] Data;
		Data = nullptr;
	}

	Binary = InBinary;

	std::ifstream FileStream;
	
	if( Binary )
	{
		FileStream.open( FileLocation.c_str(), std::ios::in | std::ios::binary );
	}
	else
	{
		FileStream.open( FileLocation.c_str(), std::ios::in );
	}
	
	if( !FileStream.fail() )
	{
		// Move to the end of the file and fetch the file size
		FileStream.seekg( 0, std::ios::end );
		FileSize = FileStream.tellg();

		FileStream.seekg( 0, std::ios::beg );

		if( FileSize > 0 )
		{
			Data = new char[FileSize];

			if( Binary )
			{
				FileStream.read( Data, FileSize );
			}
			else
			{
				FileStream.getline( Data, FileSize, '\0' );
				Data[FileSize - 1] = '\0';
			}
		}

		FileStream.close();

		stat( FileLocation.c_str(), &Statistics );

		return true;
	}

	Log::Event( Log::Warning, "Failed to load file \"%s\".\n", FileLocation.c_str() );

	return false;
}

bool CFile::Load( char* DataSource, const size_t SizeIn )
{
	if( Data )
	{
		delete[] Data;
		Data = nullptr;
	}

	Binary = true;
	Data = DataSource;
	FileSize = SizeIn;

	return true;
}

bool CFile::Load( const std::string& DataIn )
{
	if( Data )
	{
		delete[] Data;
		Data = nullptr;
	}

	const size_t Characters = DataIn.length();

	Binary = false;
	FileSize = Characters * sizeof( char );
	if( FileSize > 0 )
	{
		Data = new char[Characters];
		DataIn.copy( Data, Characters );
	}

	return true;
}

bool CFile::Load( CData& Data )
{
	const size_t Size = Data.Size();
	char* Buffer = new char[Size];

	Data.Store( Buffer, Size );

	return Load( Buffer, Size );
}

bool CFile::Save( const bool& CreateDirectory )
{
	if( Data )
	{
		std::ofstream FileStream;

		if( CreateDirectory )
		{
			const auto Path = std::experimental::filesystem::path( FileLocation );
			if( !exists( Path.parent_path() ) )
			{
				create_directories( Path.parent_path() );
			}
		}

		if( Binary )
		{
			FileStream.open( FileLocation.c_str(), std::ios::out | std::ios::binary );
		}
		else
		{
			FileStream.open( FileLocation.c_str(), std::ios::out );
		}

		if( !FileStream.fail() )
		{
			if( Binary )
			{
				FileStream.write( Data, FileSize );
			}
			else
			{
				FileStream.write( Data, FileSize );
			}

			FileStream.close();

			return true;
		}
	}

	Log::Event( "Failed to save file \"%s\".\n", FileLocation.c_str() );

	return false;
}

bool CFile::Exists() const
{
	return Exists( FileLocation.c_str() );
}

bool CFile::Exists( const char* FileLocation )
{
	struct stat Buffer;
	const bool Exists = stat( FileLocation, &Buffer ) == 0 || errno == 132;

	if( !Exists )
	{
		Log::Event( Log::Warning, "Could not find file at location \"%s\"\n", FileLocation );
	}

	return Exists;
}

bool CFile::Modified() const
{
	struct stat Buffer;
	stat( FileLocation.c_str(), &Buffer );
	return Statistics.st_mtime > Buffer.st_mtime;
}

time_t CFile::ModificationDate() const
{
	struct stat Buffer;
	stat( FileLocation.c_str(), &Buffer );
	return Buffer.st_mtime;
}

const char* GetLine( const char*& Start, const char*& End )
{
	const char* Token = Start;
	End = Start;

	while( Start )
	{
		const bool EndOfLine = Token[0] == std::streambuf::traits_type::eof() || Token[0] == '\0' || Token[0] == '\n' || ( Token[0] == '\r' && Token[1] == '\n' );
		if( EndOfLine )
		{
			End = Token + 1;

			return Start;
		}

		Token++;
		End = Token;

		if( Token[0] == std::streambuf::traits_type::eof() || Token[0] == '\0' )
		{
			End = Token + 1;

			return nullptr;
		}
	}

	return nullptr;
}

std::istream& GetLineStream( std::istream& is, std::string& t )
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se( is, true );
	std::streambuf* sb = is.rdbuf();

	for( ;;) {
		int c = sb->sbumpc();
		switch( c ) {
			case '\n':
				return is;
			case '\r':
				if( sb->sgetc() == '\n' )
					sb->sbumpc();
				return is;
			case std::streambuf::traits_type::eof():
				// Also handle the case when the last line has no line ending
				if( t.empty() )
					is.setstate( std::ios::eofbit );
				return is;
			default:
				t += (char) c;
		}
	}
}

double ParseDouble( const char* p )
{
	if( !*p || *p == '?' )
		return -0x7FFFFFFF;
	int s = 1;
	while( *p == ' ' ) p++;

	if( *p == '-' ) {
		s = -1; p++;
	}

	double acc = 0;
	while( *p >= '0' && *p <= '9' )
		acc = acc * 10 + *p++ - '0';

	if( *p == '.' ) {
		double k = 0.1;
		p++;
		while( *p >= '0' && *p <= '9' ) {
			acc += ( *p++ - '0' ) * k;
			k *= 0.1;
		}
	}
	if( *p )
	{
		Log::Event( Log::Error, "Invalid numeric format.\n" );
	}
	return s * acc;
}

float ParseFloat( const char* p )
{
	return StaticCast<float>( ParseDouble( p ) );
}

void Extract( const std::string& String, float& Out )
{
	Extract( String.c_str(), Out );
}

void Extract( const std::string& String, int32_t& Out )
{
	Extract( String.c_str(), Out );
}

void Extract( const std::string& String, uint32_t& Out )
{
	Extract( String.c_str(), Out );
}

void Extract( const std::string& String, Vector3D& Out )
{
	Extract( String.c_str(), Out );
}

void Extract( const std::string& String, Vector4D& Out )
{
	Extract( String.c_str(), Out );
}

void Extract( const char* Start, float& Out )
{
	size_t OutTokenCount = 0;
	auto* TokenDistance = ExtractTokensFloat( Start, ' ', OutTokenCount, 1 );
	if( OutTokenCount == 1 )
	{
		Out = TokenDistance[0];
	}
}

void Extract( const std::string& Value, bool& Out )
{
	if( Value == "0" )
	{
		Out = false;
	}
	else
	{
		Out = true;
	}
}

void Extract( const char* Start, int32_t& Out )
{
	size_t OutTokenCount = 0;
	auto* TokenDistance = ExtractTokensInteger( Start, ' ', OutTokenCount, 1 );
	if( OutTokenCount == 1 )
	{
		Out = TokenDistance[0];
	}
}

void Extract( const char* Start, uint32_t& Out )
{
	size_t OutTokenCount = 0;
	auto* TokenDistance = ExtractTokensInteger( Start, ' ', OutTokenCount, 1 );
	if( OutTokenCount == 1 )
	{
		Out = StaticCast<uint32_t>( TokenDistance[0] );
	}
}

void Extract( const char* Start, Vector3D& Out )
{
	size_t OutTokenCount = 0;
	auto* TokenDistance = ExtractTokensFloat( Start, ' ', OutTokenCount, 3 );
	if (OutTokenCount == 3)
	{
		Out = Vector3D( TokenDistance[0], TokenDistance[1], TokenDistance[2] );
	}
}

void Extract( const char* Start, Vector4D& Out )
{
	size_t OutTokenCount = 0;
	auto* TokenDistance = ExtractTokensFloat( Start, ' ', OutTokenCount, 4 );
	if (OutTokenCount == 4)
	{
		Out = Vector4D( TokenDistance[0], TokenDistance[1], TokenDistance[2], TokenDistance[3] );
	}
}

std::vector<std::string> ExtractTokens( const char* Start, char Delimiter, const size_t ExpectedTokens /*= 3 */ )
{
	std::vector<std::string> Tokens;
	Tokens.resize( ExpectedTokens );
	size_t TokenCount = 0;

	const char* Token = Start;
	const char* End = Start;

	static const size_t BufferSize = 256;
	char Buffer[BufferSize];
	while( true )
	{
		const bool EndOfLine = Token[0] == '\0' || Token[0] == '\n' || ( Token[0] == '\r' && Token[1] == '\n' );
		if( Token[0] == Delimiter || EndOfLine )
		{
			const size_t Length = End - Start;
			if( Length < BufferSize - 1 )
			{
				strncpy_s( Buffer, Start, Length );
				Buffer[Length + 1] = '\0';

				if( TokenCount < ExpectedTokens )
				{
					Tokens[TokenCount++] = Buffer;
				}
			}

			Start = Token + 1;
			End = Start;
		}

		Token++;
		End = Token;

		if( EndOfLine )
		{
			return Tokens;
		}
	}
}

static const size_t TokenBufferSize = 1024;
float FloatBuffer[TokenBufferSize];
float* ExtractTokensFloat( const char* Start, char Delimiter, size_t& OutTokenCount, const size_t ExpectedTokens /*= 3 */ )
{
	size_t Location = 0;

	const char* Token = Start;
	const char* End = Start;

	static const size_t BufferSize = 256;
	char Buffer[BufferSize];
	while( true )
	{
		const bool EndOfLine = Token[0] == '\0' || Token[0] == '\n' || ( Token[0] == '\r' && Token[1] == '\n' );
		if( Token[0] == Delimiter || EndOfLine )
		{
			const size_t Length = End - Start;
			if( Length < BufferSize - 1 )
			{
				strncpy_s( Buffer, Start, Length );
				Buffer[Length + 1] = '\0';

				if( Location < TokenBufferSize )
				{
					FloatBuffer[Location++] = static_cast<float>( ParseDouble( Buffer ) );
				}
			}

			Start = Token + 1;
			End = Start;
		}

		Token++;
		End = Token;

		if( EndOfLine )
		{
			if( Location <= ExpectedTokens )
			{
				OutTokenCount = Location;
				return FloatBuffer;
			}
			else
			{
				OutTokenCount = 0;
				return nullptr;
			}
		}
	}
}

int IntegerBuffer[TokenBufferSize];
int* ExtractTokensInteger( const char* Start, char Delimiter, size_t& OutTokenCount, const size_t ExpectedTokens /*= 3 */ )
{
	size_t Location = 0;

	const char* Token = Start;
	const char* End = Start;

	static const size_t BufferSize = 256;
	char Buffer[BufferSize];
	while( true )
	{
		const bool EndOfLine = Token[0] == '\0' || Token[0] == '\n' || ( Token[0] == '\r' && Token[1] == '\n' );
		if( Token[0] == Delimiter || EndOfLine )
		{
			const size_t Length = End - Start;
			if( Length < BufferSize - 1 )
			{
				strncpy_s( Buffer, Start, Length );
				Buffer[Length + 1] = '\0';

				if( Location < TokenBufferSize )
				{
					IntegerBuffer[Location++] = Math::Integer( Buffer );
				}
			}

			Start = Token + 1;
			End = Start;
		}

		Token++;
		End = Token;

		if( EndOfLine )
		{
			if( Location <= ExpectedTokens )
			{
				OutTokenCount = Location;
				return IntegerBuffer;
			}
			else
			{
				OutTokenCount = 0;
				return nullptr;
			}
		}
	}
}
