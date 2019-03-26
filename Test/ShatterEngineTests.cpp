#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <Engine/Utility/MeshBuilder.h>
#include <Engine/World/World.h>
#include <Engine/Utility/File.h>
#include <string>
#include <vector>

#include <direct.h>

namespace Test
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

		TEST_METHOD( GenerateCircle )
		{
			Logger::WriteMessage( "Generating a circle primitive with 32 segments." );
			FPrimitive Primitive;
			MeshBuilder::Circle( Primitive, 1.0f, 32 );

			const uint32_t ValidCount = 32;
			Assert::AreEqual( ValidCount, Primitive.VertexCount );
		}

		TEST_METHOD( GenerateSphere )
		{
			Logger::WriteMessage( "Generating a sphere primitive with 32 segments and 16 rings." );
			FPrimitive Primitive;
			MeshBuilder::Sphere( Primitive, 1.0f, 32, 16 );

			const uint32_t ValidCount = 482;
			Assert::AreEqual( ValidCount, Primitive.VertexCount );
		}

		TEST_METHOD( GenerateCone )
		{
			Logger::WriteMessage( "Generating a cone primitive with 4 sides." );
			FPrimitive Primitive;
			MeshBuilder::Cone( Primitive, 1.0f, 4 );

			const uint32_t ValidCount = 5;
			Assert::AreEqual( ValidCount, Primitive.VertexCount );
		}

		TEST_METHOD( GenerateTorus )
		{
			Logger::WriteMessage( "Generating a torus primitive with 48 major segments and 12 minor segments." );
			FPrimitive Primitive;
			MeshBuilder::Torus( Primitive, 1.0f, 48, 12 );

			const uint32_t ValidCount = 576;
			Assert::AreEqual( ValidCount, Primitive.VertexCount );
		}

		TEST_METHOD( GenerateGrid )
		{
			Logger::WriteMessage( "Generating a grid primitive with 10 subdivisions on the X-axis and 10 subdivisions on the Y-axis." );
			FPrimitive Primitive;
			MeshBuilder::Grid( Primitive, 1.0f, 10, 10 );

			const uint32_t ValidCount = 100;
			Assert::AreEqual( ValidCount, Primitive.VertexCount );
		}
	};

	TEST_CLASS( UtilityFunctions )
	{
	public:
		TEST_METHOD( ParseLevelJSON )
		{
			Logger::WriteMessage( "Parsing level JSON." );
			bool bSuccess = false;

			CFile LevelFile( "../TestModels/Island.sls" );
			if( LevelFile.Exists() )
			{
				LevelFile.Load();
				FLevel Test;
				Test.Level.Load( LevelFile );
				bSuccess = Test.Level.GetEntities().size() > 0;
			}

			Assert::AreEqual( 1, bSuccess ? 1 : 0 );
		}

		TEST_METHOD( ParseStringTokens )
		{
			Logger::WriteMessage( "Parsing line of tokens." );
			bool bSuccess = false;

			std::string String = "-0.173 0.251 -0.704\r\n";
			std::vector<std::string> Tokens = ExtractTokens( String, ' ', 3 );

			if( Tokens.size() == 3 )
			{
				if( Tokens[0] == "-0.173" && Tokens[1] == "0.251" && Tokens[2] == "-0.704" )
				{
					bSuccess = true;
				}
			}

			for( auto& Token : Tokens )
			{
				Logger::WriteMessage( Token.c_str() );
			}

			Assert::AreEqual( 1, bSuccess ? 1 : 0 );
		}

		TEST_METHOD( ParseOBJ )
		{
			Logger::WriteMessage( "Parsing OBJ." );
			bool bSuccess = false;

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
							bSuccess = true;
						}
					}
				}

				char buffer[256];
				int ret = snprintf( buffer, sizeof buffer, "Parse time: %llims", Timer.GetElapsedTimeMilliseconds() );
				Logger::WriteMessage( buffer );
			}

			Assert::AreEqual( 1, bSuccess ? 1 : 0 );
		}
	};
}
