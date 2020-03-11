#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <Engine/Utility/MeshBuilder.h>
#include <Engine/World/World.h>
#include <Engine/Utility/Chunk.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>
#include <string>
#include <vector>

#include <direct.h>

namespace EngineTest
{
	TEST_CLASS( PrimitiveTests )
	{
	public:
		TEST_METHOD( GenerateTriangle )
		{
			Logger::WriteMessage( "Generating a triangle primitive." );
			FPrimitive Primitive;
			MeshBuilder::Triangle( Primitive, 1.0f );

			const uint32_t ValidCount = 3;
			Assert::AreEqual( ValidCount, Primitive.VertexCount );
		}

		TEST_METHOD( GeneratePlane )
		{
			Logger::WriteMessage( "Generating a plane primitive." );
			FPrimitive Primitive;
			MeshBuilder::Plane( Primitive, 1.0f );

			const uint32_t ValidCount = 4;
			Assert::AreEqual( ValidCount, Primitive.VertexCount );
		}

		TEST_METHOD( GenerateCube )
		{
			Logger::WriteMessage( "Generating a cube primitive." );
			FPrimitive Primitive;
			MeshBuilder::Cube( Primitive, 1.0f );

			const uint32_t ValidVertices = 8;
			const uint32_t ValidIndices = 36;
			Assert::AreEqual( ValidVertices, Primitive.VertexCount  );
			Assert::AreEqual( ValidIndices, Primitive.IndexCount  );
		}
	};

	TEST_CLASS( UtilityFunctions )
	{
	public:
		TEST_METHOD( ParseLevelJSON )
		{
			Logger::WriteMessage( "Parsing level JSON." );
			bool Success = false;

			CFile LevelFile( "../TestModels/Island.sls" );
			if( LevelFile.Exists() )
			{
				LevelFile.Load();
				CLevel Test;
				Test.Load( LevelFile );
				Success = Test.GetEntities().size() > 0;
			}

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}

		TEST_METHOD( ParseStringTokens )
		{
			Logger::WriteMessage( "Parsing line of tokens." );
			bool Success = false;

			std::string String = "-0.173 0.251 -0.704\r\n";
			std::vector<std::string> Tokens = ExtractTokens( String.c_str(), ' ', 3 );

			if( Tokens.size() == 3 )
			{
				if( Tokens[0] == "-0.173" && Tokens[1] == "0.251" && Tokens[2] == "-0.704" )
				{
					Success = true;
				}
			}

			for( auto& Token : Tokens )
			{
				Logger::WriteMessage( Token.c_str() );
			}

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}

		TEST_METHOD( ParseOBJ )
		{
			Logger::WriteMessage( "Parsing OBJ." );
			bool Success = false;

			char Path[FILENAME_MAX];
			_getcwd( Path, sizeof( Path ) );
			Logger::WriteMessage( Path );
			CFile File( "../TestModels/LoftyLagoonTestModel.obj" );
			if( File.Exists() )
			{
				File.Load();
				FPrimitive Primitive;

				CTimer Timer;
				Timer.Start();
				MeshBuilder::OBJ( Primitive, File );
				Timer.Stop();
				if( Primitive.VertexCount > 0 )
				{
					if( Primitive.Vertices[0].Position[0] )
					{
						char buffer[64];
						int ret = snprintf( buffer, sizeof buffer, "%f", Primitive.Vertices[0].Position[0] );
						std::string Buffer = buffer;
						if( Buffer == "-0.704569" )
						{
							Success = true;
						}
					}
				}

				char buffer[256];
				int ret = snprintf( buffer, sizeof buffer, "Parse time: %llims", Timer.GetElapsedTimeMilliseconds() );
				Logger::WriteMessage( buffer );
			}

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}

		TEST_METHOD( SerializeData )
		{
			bool Success = false;

			uint32_t Write = 500;
			CData Data;
			Data << Write;

			uint32_t Read = 0;
			Data >> Read;

			Success = Read == Write;

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}

		TEST_METHOD( SerializeChunk )
		{
			bool Success = false;

			uint32_t Write = 500;
			CData Data;

			Chunk WriteChunk("TSTDT");
			WriteChunk.Data << Write;

			Data << WriteChunk;

			uint32_t Read = 0;
			Chunk ReadChunk( "TSTDT" );
			Data >> ReadChunk;
			ReadChunk.Data >> Read;

			Success = Read == Write;

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}

		TEST_METHOD( SerializeChunkToDisk )
		{
			bool Success = false;

			uint32_t Write = 500;
			CData WriteData;

			Chunk WriteChunk( "TSTDT" );
			WriteChunk.Data << Write;

			WriteData << WriteChunk;

			CFile WriteFile( "TestData.tst" );
			WriteFile.Load( WriteData );
			WriteFile.Save();

			CFile ReadFile( "TestData.tst" );
			ReadFile.Load( true );

			uint32_t Read = 0;
			Chunk ReadChunk( "TSTDT" );
			ReadFile.Extract( ReadChunk );
			ReadChunk.Data >> Read;

			Success = Read == Write;

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}
	};
}
