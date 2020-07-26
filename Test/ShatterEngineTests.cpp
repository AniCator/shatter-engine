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

	static uint16_t TabOffset = 0;
	void RecurseJSONObject( JSON::Object* Object )
	{
		if( !Object )
			return;

		std::wstring TabString;
		for( auto Index = 0; Index < TabOffset; Index++ )
		{
			TabString += L"---";
		}

		const std::wstring ObjectType = Object->IsField ? L" (field)" : Object->IsArray ? L" (array)" : L" (object)";
		if( Object->Objects.empty() )
		{
			const std::wstring ObjectKey = TabString + std::wstring( Object->Key.begin(), Object->Key.end() );
			// Logger::WriteMessage( ObjectKey.c_str() );

			const std::wstring ObjectValue = ObjectKey + L" : " + std::wstring( Object->Value.begin(), Object->Value.end() ) + ObjectType;
			Logger::WriteMessage( ObjectValue.c_str() );
		}
		else
		{
			const std::wstring ObjectKey = TabString + std::wstring( Object->Key.begin(), Object->Key.end() ) + L" (" + std::to_wstring( Object->Objects.size() ) + L")" + ObjectType;
			Logger::WriteMessage( ObjectKey.c_str() );

			TabOffset++;
			for(const auto& SubObject: Object->Objects )
			{
				RecurseJSONObject( SubObject );
			}
			TabOffset--;
		}
	}

	JSON::Container GetJSON( const std::string& FileName )
	{
		const std::string Path = "../TestModels/" + FileName;
		CFile JSONFile( Path.c_str() );
		if( JSONFile.Exists() )
		{
			JSONFile.Load();
			return JSON::GenerateTree( JSONFile );
		}

		return JSON::Container();
	}

	JSON::Container GetTestJSON()
	{
		return GetJSON( "ExampleJSON.json" );
	}

	TEST_CLASS( JSONTests )
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
				Success = Test.GetEntities().size() == 3;
				const auto EntityString = L"Found " + std::to_wstring( Test.GetEntities().size() ) + L" entities.";
				Logger::WriteMessage( EntityString.c_str() );
			}

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}

		TEST_METHOD( ParseJSONFile_Dialogue )
		{
			Logger::WriteMessage( "Parsing dialogue JSON." );

			const auto Tree = GetJSON( "GrocerTest.sdscript" );
			Assert::IsTrue( Tree.Tree[0]->Key == "session", L"session key not found" );
		}

		TEST_METHOD( ParseJSONContainerObjects )
		{
			Logger::WriteMessage( "Checking JSON container objects." );
			const auto& Tree = GetTestJSON();

			Assert::IsFalse( Tree.Objects.empty(), L"Object array is empty." );
		}

		TEST_METHOD( ParseJSONTree )
		{
			Logger::WriteMessage( "Checking JSON Tree size." );
			const auto& Tree = GetTestJSON();

			// Output tree debug information.
			for( const auto& Object : Tree.Tree )
			{
				RecurseJSONObject( Object );
			}

			const std::wstring TreeSize = std::wstring( L"Incorrect tree size, expecting 9 top level objects. (found " ) + std::to_wstring( Tree.Tree.size() ) + L")";
			Assert::IsTrue( Tree.Tree.size() == 9, TreeSize.c_str() );
		}

		TEST_METHOD( ParseJSONFile_ExampleJSON )
		{
			Logger::WriteMessage( "Parsing arbitrary JSON file." );

			CFile JSONFile( "../TestModels/ExampleJSON.json" );
			if( JSONFile.Exists() )
			{
				JSONFile.Load();
				const auto Tree = JSON::GenerateTree( JSONFile );

				Assert::IsTrue( Tree.Tree[0]->Key == "topkey", L"Top key entry not found." );

				// Composite object.
				Assert::IsTrue( Tree.Tree[1]->Key == "object1", L"object1 key not found." );
				Assert::IsTrue( Tree.Tree[1]->Objects[0]->Key.empty(), L"object1 first key is not empty." );
				Assert::IsTrue( Tree.Tree[1]->Objects[0]->Value == "keylessvalue", L"object1 keylessvalue not found." );

				// Check the first object array
				Assert::IsTrue( Tree.Tree[2]->Key == "objectarray", L"objectarray key not found." );

				// Reference the objects of the object array for easy access.
				const auto& Objects = Tree.Tree[2]->Objects;
				const std::wstring ArraySize = std::wstring( L"Incorrect first object array size, expecting 4 objects. (found " ) + std::to_wstring( Objects.size() ) + L")";
				Assert::IsTrue( Objects.size() == 4, ArraySize.c_str() );

				// Check array keys.
				const auto& Object0 = Objects[0]->Objects;
				Assert::IsTrue( Object0[0]->Key == "key1", L"key1 key not found (in objectarray)" );

				const auto& Object1 = Objects[1]->Objects;
				Assert::IsTrue( Object1[0]->Key == "key2", L"key2 key not found (in objectarray)" );

				const auto& Object2 = Objects[2]->Objects;
				Assert::IsTrue( Object2[0]->Key == "key3", L"key3 key not found (in objectarray)" );
				Assert::IsTrue( Object2[1]->Key == "key4", L"key4 key not found (in objectarray)" );

				const auto& Object3 = Objects[3]->Objects;
				Assert::IsTrue( Object3[0]->Key == "key5", L"key5 key not found (in objectarray)" );

				// Check array values.
				Assert::IsTrue( Object0[0]->Value == "value1", L"value1 key not found (in objectarray)" );
				Assert::IsTrue( Object1[0]->Value == "value2", L"value2 key not found (in objectarray)" );
				Assert::IsTrue( Object2[0]->Value == "value3", L"value3 key not found (in objectarray)" );
				Assert::IsTrue( Object2[1]->Value == "value4", L"value3 key not found (in objectarray)" );
				Assert::IsTrue( Object3[0]->Value == "value5", L"value3 key not found (in objectarray)" );

				Assert::IsTrue( Tree.Tree[3]->Key == "object2", L"object2 key not found." );
				Assert::IsTrue( Tree.Tree[4]->Key == "object3", L"object3 key not found." );
				Assert::IsTrue( Tree.Tree[5]->Key == "object4", L"object4 key not found." );

				// Check the second object array
				Assert::IsTrue( Tree.Tree[6]->Key == "objectarray2", L"Second objectarray key not found." );

				// Reference the objects of the object array for easy access.
				const auto& Objects2 = Tree.Tree[6]->Objects;
				const std::wstring ArraySize2 = std::wstring( L"Incorrect second object array size, expecting 5 objects. (found " ) + std::to_wstring( Objects.size() ) + L")";
				Assert::IsTrue( Objects2.size() == 4, ArraySize2.c_str() );

				// Check array keys.
				const auto& Object20 = Objects2[0]->Objects;
				Assert::IsTrue( Object20[0]->Key == "key1_new", L"key1_new key not found (in objectarray2)" );

				const auto& Object21 = Objects2[1]->Objects;
				Assert::IsTrue( Object21[0]->Key == "key2", L"key2 key not found (in objectarray2)" );

				const auto& Object22 = Objects2[2]->Objects;
				Assert::IsTrue( Object22[0]->Key == "key3", L"key3 key not found (in objectarray2)" );
				Assert::IsTrue( Object22[1]->Key == "key4", L"key4 key not found (in objectarray2)" );

				const auto& Object23 = Objects2[3]->Objects;
				Assert::IsTrue( Object23[0]->Key == "key5", L"key5 key not found (in objectarray2)" );

				// Check array values.
				Assert::IsTrue( Object20[0]->Value == "value1", L"value1 key not found (in objectarray2)" );
				Assert::IsTrue( Object21[0]->Value == "value2", L"value2 key not found (in objectarray2)" );
				Assert::IsTrue( Object22[0]->Value == "value3", L"value3 key not found (in objectarray2)" );
				Assert::IsTrue( Object22[1]->Value == "value4", L"value3 key not found (in objectarray2)" );
				Assert::IsTrue( Object23[0]->Value == "value5", L"value3 key not found (in objectarray2)" );

				// Check if object5 is still stored correctly.
				Assert::IsTrue( Tree.Tree[7]->Key == "object5", L"object5 key not found." );
				Assert::IsTrue( Tree.Tree[7]->Objects[0]->Key == "key", L"key key not found. (in object5)" );
				Assert::IsTrue( Tree.Tree[7]->Objects[0]->Value == "value", L"value key not found. (in object5)" );

				// Check the nested example dialogue tree.
				Assert::IsTrue( Tree.Tree[8]->Key == "session", L"session key not found." );
				const auto& Session = Tree.Tree[8];
				Assert::IsTrue( Session->Objects[0], L"First cue not found." );
				const auto& Cue = Session->Objects[0];
				const auto& Name = Cue->operator[]("name");
				Assert::IsTrue( Name, L"name key not found." );
				Assert::IsTrue( Name->Value == "Start", L"Start value not found." );

				const auto& Conditions = JSON::Find( Cue->Objects, "conditions" );
				Assert::IsTrue( Conditions, L"conditions array not found." );
				const auto& Memory = Conditions->Objects[0]->GetValue( "type" );
				Assert::IsTrue( Memory == "memory", L"Expected \"memory\" type." );

				Assert::IsTrue( Session->Objects[1], L"Second cue not found." );
				const auto& Cue2 = Session->Objects[1];
				const auto& Choices = JSON::Find( Cue2->Objects, "choices" );
				Assert::IsTrue( Choices, L"choices array not found." );
				Assert::IsTrue( Choices->Objects.size() == 1, L"Expected choices size of 1." );
			}
		}
	};

	TEST_CLASS( UtilityFunctions )
	{
	public:
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
