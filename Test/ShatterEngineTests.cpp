#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <Engine/Display/UserInterface.h>
#include <Engine/Resource/AssetPool.h>
#include <Engine/Utility/MeshBuilder.h>
#include <Engine/World/World.h>
#include <Engine/Utility/Chunk.h>
#include <Engine/Utility/Container.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/RunLengthEncoding.h>
#include <Engine/Utility/LoftyMeshInterface.h>
#include <string>
#include <vector>

#include <direct.h>

namespace General
{
	std::wstring ToString( const Vector3D& In )
	{
		return L"{" + 
			std::to_wstring( In.X ) + L", " + 
			std::to_wstring( In.Y ) + L", " + 
			std::to_wstring( In.Z ) + L"}";
	}

	std::wstring ToString( const Vector4D& In )
	{
		return L"{" +
			std::to_wstring( In.X ) + L", " +
			std::to_wstring( In.Y ) + L", " +
			std::to_wstring( In.Z ) + L",";
			std::to_wstring( In.W ) + L"}";
	}

	TEST_CLASS( PrimitiveTests )
	{
	public:
		TEST_METHOD( GenerateTriangle )
		{
			// Logger::WriteMessage( "Generating a triangle primitive." );
			FPrimitive Primitive;
			MeshBuilder::Triangle( Primitive, 1.0f );

			const uint32_t ValidCount = 3;
			Assert::AreEqual( ValidCount, Primitive.VertexCount );
		}

		TEST_METHOD( GeneratePlane )
		{
			// Logger::WriteMessage( "Generating a plane primitive." );
			FPrimitive Primitive;
			MeshBuilder::Plane( Primitive, 1.0f );

			const uint32_t ValidCount = 4;
			Assert::AreEqual( ValidCount, Primitive.VertexCount );
		}

		TEST_METHOD( GenerateCube )
		{
			// Logger::WriteMessage( "Generating a cube primitive." );
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
			// Logger::WriteMessage( ObjectValue.c_str() );
		}
		else
		{
			const std::wstring ObjectKey = TabString + std::wstring( Object->Key.begin(), Object->Key.end() ) + L" (" + std::to_wstring( Object->Objects.size() ) + L")" + ObjectType;
			// Logger::WriteMessage( ObjectKey.c_str() );

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
			return JSON::Tree( JSONFile );
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
			// Logger::WriteMessage( "Parsing level JSON." );
			bool Success = false;

			CFile LevelFile( "../TestModels/Island.sls" );
			if( LevelFile.Exists() )
			{
				LevelFile.Load();
				CLevel Test;
				Test.Load( LevelFile );
				Test.Construct();
				Success = Test.GetEntities().size() == 3;
				const auto EntityString = L"Found " + std::to_wstring( Test.GetEntities().size() ) + L" entities.";
				// Logger::WriteMessage( EntityString.c_str() );
			}

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}

		TEST_METHOD( ParseJSONFile_Dialogue )
		{
			// Logger::WriteMessage( "Parsing dialogue JSON." );

			const auto Tree = GetJSON( "GrocerTest.sdscript" );
			Assert::IsTrue( JSON::Find( Tree.Tree, "session" ), L"session key not found" );
		}

		TEST_METHOD( ParseJSONContainerObjects )
		{
			// Logger::WriteMessage( "Checking JSON container objects." );
			const auto& Tree = GetTestJSON();

			Assert::IsFalse( Tree.Objects.empty(), L"Object array is empty." );
		}

		TEST_METHOD( ParseJSONTree )
		{
			// Logger::WriteMessage( "Checking JSON Tree size." );
			const auto& Tree = GetTestJSON();

			// Output tree debug information.
			for( const auto& Object : Tree.Tree )
			{
				RecurseJSONObject( Object );
			}

			const std::wstring TreeSize = std::wstring( L"Incorrect tree size, expecting 9 top level objects. (found " ) + std::to_wstring( Tree.Tree.size() ) + L")";
			Assert::IsTrue( Tree.Tree.size() == 9, TreeSize.c_str() );
		}

		//TEST_METHOD( ParseJSONFile_ExampleJSON )
		//{
		//	Logger::WriteMessage( "Parsing arbitrary JSON file." );

		//	CFile JSONFile( "../TestModels/ExampleJSON.json" );
		//	if( JSONFile.Exists() )
		//	{
		//		JSONFile.Load();
		//		const auto Tree = JSON::GenerateTree( JSONFile );

		//		Assert::IsTrue( Tree.Tree[0]->Key == "topkey", L"Top key entry not found." );

		//		// Composite object.
		//		Assert::IsTrue( Tree.Tree[1]->Key == "object1", L"object1 key not found." );
		//		Assert::IsTrue( Tree.Tree[1]->Objects[0]->Key.empty(), L"object1 first key is not empty." );
		//		Assert::IsTrue( Tree.Tree[1]->Objects[0]->Value == "keylessvalue", L"object1 keylessvalue not found." );

		//		// Check the first object array
		//		Assert::IsTrue( Tree.Tree[2]->Key == "objectarray", L"objectarray key not found." );

		//		// Reference the objects of the object array for easy access.
		//		const auto& Objects = Tree.Tree[2]->Objects;
		//		const std::wstring ArraySize = std::wstring( L"Incorrect first object array size, expecting 4 objects. (found " ) + std::to_wstring( Objects.size() ) + L")";
		//		Assert::IsTrue( Objects.size() == 4, ArraySize.c_str() );

		//		// Check array keys.
		//		const auto& Object0 = Objects[0]->Objects;
		//		Assert::IsTrue( Object0[0]->Key == "key1", L"key1 key not found (in objectarray)" );

		//		const auto& Object1 = Objects[1]->Objects;
		//		Assert::IsTrue( Object1[0]->Key == "key2", L"key2 key not found (in objectarray)" );

		//		const auto& Object2 = Objects[2]->Objects;
		//		Assert::IsTrue( Object2[0]->Key == "key3", L"key3 key not found (in objectarray)" );
		//		Assert::IsTrue( Object2[1]->Key == "key4", L"key4 key not found (in objectarray)" );

		//		const auto& Object3 = Objects[3]->Objects;
		//		Assert::IsTrue( Object3[0]->Key == "key5", L"key5 key not found (in objectarray)" );

		//		// Check array values.
		//		Assert::IsTrue( Object0[0]->Value == "value1", L"value1 key not found (in objectarray)" );
		//		Assert::IsTrue( Object1[0]->Value == "value2", L"value2 key not found (in objectarray)" );
		//		Assert::IsTrue( Object2[0]->Value == "value3", L"value3 key not found (in objectarray)" );
		//		Assert::IsTrue( Object2[1]->Value == "value4", L"value3 key not found (in objectarray)" );
		//		Assert::IsTrue( Object3[0]->Value == "value5", L"value3 key not found (in objectarray)" );

		//		Assert::IsTrue( Tree.Tree[3]->Key == "object2", L"object2 key not found." );
		//		Assert::IsTrue( Tree.Tree[4]->Key == "object3", L"object3 key not found." );
		//		Assert::IsTrue( Tree.Tree[5]->Key == "object4", L"object4 key not found." );

		//		// Check the second object array
		//		Assert::IsTrue( Tree.Tree[6]->Key == "objectarray2", L"Second objectarray key not found." );

		//		// Reference the objects of the object array for easy access.
		//		const auto& Objects2 = Tree.Tree[6]->Objects;
		//		const std::wstring ArraySize2 = std::wstring( L"Incorrect second object array size, expecting 5 objects. (found " ) + std::to_wstring( Objects.size() ) + L")";
		//		Assert::IsTrue( Objects2.size() == 4, ArraySize2.c_str() );

		//		// Check array keys.
		//		const auto& Object20 = Objects2[0]->Objects;
		//		Assert::IsTrue( Object20[0]->Key == "key1_new", L"key1_new key not found (in objectarray2)" );

		//		const auto& Object21 = Objects2[1]->Objects;
		//		Assert::IsTrue( Object21[0]->Key == "key2", L"key2 key not found (in objectarray2)" );

		//		const auto& Object22 = Objects2[2]->Objects;
		//		Assert::IsTrue( Object22[0]->Key == "key3", L"key3 key not found (in objectarray2)" );
		//		Assert::IsTrue( Object22[1]->Key == "key4", L"key4 key not found (in objectarray2)" );

		//		const auto& Object23 = Objects2[3]->Objects;
		//		Assert::IsTrue( Object23[0]->Key == "key5", L"key5 key not found (in objectarray2)" );

		//		// Check array values.
		//		Assert::IsTrue( Object20[0]->Value == "value1", L"value1 key not found (in objectarray2)" );
		//		Assert::IsTrue( Object21[0]->Value == "value2", L"value2 key not found (in objectarray2)" );
		//		Assert::IsTrue( Object22[0]->Value == "value3", L"value3 key not found (in objectarray2)" );
		//		Assert::IsTrue( Object22[1]->Value == "value4", L"value3 key not found (in objectarray2)" );
		//		Assert::IsTrue( Object23[0]->Value == "value5", L"value3 key not found (in objectarray2)" );

		//		// Check if object5 is still stored correctly.
		//		Assert::IsTrue( Tree.Tree[7]->Key == "object5", L"object5 key not found." );
		//		Assert::IsTrue( Tree.Tree[7]->Objects[0]->Key == "key", L"key key not found. (in object5)" );
		//		Assert::IsTrue( Tree.Tree[7]->Objects[0]->Value == "value", L"value key not found. (in object5)" );

		//		// Check the nested example dialogue tree.
		//		Assert::IsTrue( Tree.Tree[8]->Key == "session", L"session key not found." );
		//		const auto& Session = Tree.Tree[8];
		//		Assert::IsTrue( Session->Objects[0], L"First cue not found." );
		//		const auto& Cue = Session->Objects[0];
		//		const auto& Name = Cue->operator[]("name");
		//		Assert::IsTrue( Name, L"name key not found." );
		//		Assert::IsTrue( Name->Value == "Start", L"Start value not found." );

		//		const auto& Conditions = JSON::Find( Cue->Objects, "conditions" );
		//		Assert::IsTrue( Conditions, L"conditions array not found." );
		//		const auto& Memory = Conditions->Objects[0]->GetValue( "type" );
		//		Assert::IsTrue( Memory == "memory", L"Expected \"memory\" type." );

		//		Assert::IsTrue( Session->Objects[1], L"Second cue not found." );
		//		const auto& Cue2 = Session->Objects[1];
		//		const auto& Choices = JSON::Find( Cue2->Objects, "choices" );
		//		Assert::IsTrue( Choices, L"choices array not found." );
		//		Assert::IsTrue( Choices->Objects.size() == 1, L"Expected choices size of 1." );
		//	}
		//}
	};

	TEST_CLASS( UtilityFunctions )
	{
	public:
		TEST_METHOD( ParseStringTokens )
		{
			// Logger::WriteMessage( "Parsing line of tokens." );
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
				// Logger::WriteMessage( Token.c_str() );
			}

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}

		TEST_METHOD( ParseOBJ )
		{
			// Logger::WriteMessage( "Parsing OBJ." );
			bool Success = false;

			char Path[FILENAME_MAX];
			_getcwd( Path, sizeof( Path ) );
			// Logger::WriteMessage( Path );
			CFile File( "../TestModels/LoftyLagoonTestModel.obj" );
			if( File.Exists() )
			{
				File.Load();
				FPrimitive Primitive;

				Timer Timer;
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
				// Logger::WriteMessage( buffer );
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

		TEST_METHOD( SerializeMarker )
		{
			bool Success = false;

			CData Data;
			DataMarker::Mark( Data, "TestMarker" );

			const uint8_t Input = 255;
			Data << Input;

			Data.ReadToStart();
			if( !DataMarker::Check( Data, "TestMarker2" ) &&
				DataMarker::Check( Data, "TestMarker" ) )
			{
				uint8_t Output = 0;
				Data >> Output;

				if( Output == Input )
				{
					Success = true;
				}
			}

			Assert::AreEqual( 1, Success ? 1 : 0 );
		}

		TEST_METHOD( SerializeMarkerHop )
		{
			CData Data;
			DataMarker::Mark( Data, "TestMarker" );

			Data.WriteToEnd();

			const uint8_t Input = 255;
			Data << Input;

			Data.ReadToStart();
			if( DataMarker::Check( Data, "TestMarker" ) )
			{
				uint8_t Output = 0;
				Data >> Output;
			}
			else
			{
				Assert::Fail( L"TestMarker not found." );
			}

			if( !DataMarker::Check( Data, "TestMarker2" ) )
			{
				// Good.
			}
			else
			{
				Assert::Fail( L"TestMarker2 shouldn't exist yet." );
			}

			Data.WriteToEnd();

			DataMarker::Mark( Data, "TestMarker2" );
			Data << Input;

			Data.ReadToStart();
			if( DataMarker::Check( Data, "TestMarker" ) )
			{
				uint8_t Output = 0;
				Data >> Output;
			}
			else
			{
				Assert::Fail( L"TestMarker corrupted." );
			}

			if( DataMarker::Check( Data, "TestMarker2" ) )
			{
				uint8_t Output = 0;
				Data >> Output;

				if( Output != Input )
				{
					Assert::Fail( L"TestMarker2 Output doesn't match Input." );
				}
			}
			else
			{
				Assert::Fail( L"TestMarker2 not found." );
			}
		}

		// Test if we can recover from reading an incorrect type.
		TEST_METHOD( SerializeRecovery )
		{
			CData Data;
			int8_t Input = 5;
			Data << Input;
			Data.ReadToStart();
			int32_t Output32 = 0;
			Data >> Output32;

			int8_t Output8 = 0;
			Data >> Output8;

			if( Input != Output32 )
			{
				Assert::Fail( L"Failed to extract 32-bit output." );
			}

			if( Input != Output8 )
			{
				Assert::Fail( L"Failed to extract 8-bit output." );
			}

			if( !Data.Valid() )
			{
				Assert::Fail( L"Data invalid." );
			}
		}

		// Test if we can recover from reading a DataString vector mismatch.
		TEST_METHOD( SerializeRecoveryString )
		{
			CData Data;

			// Submit data.
			const int32_t InputSize = 1;
			Data << InputSize;

			// Move cursor to start.
			Data.ReadToStart();

			// Extract the data.
			int32_t OutputSize;
			Data >> OutputSize;

			std::vector<std::string> Strings;
			for( int32_t Index = 0; Index < OutputSize; Index++ )
			{
				std::string String;
				DataString::Decode( Data, String );

				if( String.length() > 0 )
				{
					Strings.emplace_back( String );
				}
			}

			if( !Strings.empty() )
			{
				Assert::Fail( L"Extracted unexpected strings." );
			}

			if( InputSize != OutputSize )
			{
				Assert::Fail( L"Data corrupted." );
			}

			if( !Data.Valid() )
			{
				Assert::Fail( L"Data invalid." );
			}
		}

		TEST_METHOD( UniqueID )
		{
			UniqueIdentifier Identifier;

			if( Identifier.Valid() )
			{
				Assert::Fail( L"Identifier valid." );
			}

			Identifier.Random();

			if( !Identifier.Valid() )
			{
				Assert::Fail( L"Identifier invalid." );
			}
		}

		TEST_METHOD( RunLengthEncoding )
		{
			const std::string Test = "0000000000000000000000000000000000000000000000000000000000000000000fsdfdsagaweadaf65464";

			const auto Encoded = RunLength::Encode( Test.data(), Test.size() );
			const auto Decoded = RunLength::Decode( Encoded.data(), Encoded.size() );

			const auto Result = std::string( Decoded.begin(), Decoded.end() );

			if( Test != Result || Encoded.size() > Test.size() )
			{
				const auto Source = std::string( Encoded.begin(), Encoded.end() );

				// Logger::WriteMessage( Test.c_str() );
				// Logger::WriteMessage( Source.c_str() );
				// Logger::WriteMessage( Result.c_str() );
				Assert::Fail( L"Decoded data differs." );
			}
		}

		TEST_METHOD( HexStringToColor )
		{
			const std::string Test = "#00FF00";
			const auto Result = Color::FromHex( Test );

			if( Result.R != 0 || Result.G != 255 || Result.B != 0 )
			{
				Assert::Fail( L"String to Color failed." );
			}
		}

		TEST_METHOD( StringReplace )
		{
			const std::string Input = "ISayHello";
			const std::string Find = "Say";
			const std::string Replace = "Shout";
			const std::string Expected = "IShoutHello";

			const auto Result = String::Replace( Input, Find, Replace );

			Assert::IsTrue( Result == Expected, L"Failed to replace in string." );
		}

		TEST_METHOD( StringSplit )
		{
			const std::string Input = "One,Two";
			constexpr char Delimiter = ',';
			const std::string ExpectedLeft = "One";
			const std::string ExpectedRight = "Two";

			const auto Result = String::Split( Input, Delimiter );

			Assert::IsTrue( 
				Result.first == ExpectedLeft &&
				Result.second == ExpectedRight,
				L"Failed to split string." );
		}

		TEST_METHOD( ImportLMI_Mesh )
		{
			CFile File( "../TestModels/cube.lmi" );
			File.Load( true );

			// Target data structures.
			FPrimitive Primitive;
			AnimationSet Set;
			Assert::IsTrue( LoftyMeshInterface::Import( File, &Primitive, Set ), L"Failed to import LMI header." );
			Assert::IsTrue( Primitive.VertexCount == 24, L"Incorrect LMI vertex count." );
			Assert::IsTrue( Primitive.IndexCount == 36, L"Incorrect LMI index count." );
		}

		TEST_METHOD( ImportLMI_MeshRigged )
		{
			CFile File( "../TestModels/icosphere.lmi" );
			File.Load( true );

			// Target data structures.
			FPrimitive Primitive;
			AnimationSet Set;
			Assert::IsTrue( LoftyMeshInterface::Import( File, &Primitive, Set ), L"Failed to import LMI header." );

			// Do it be an icosphere with the triangle-ey bits.
			Assert::IsTrue( Primitive.VertexCount == 88, L"Incorrect LMI vertex count." );
			Assert::IsTrue( Primitive.IndexCount == 240, L"Incorrect LMI index count." );

			// Check if we have allocated the expected amount of bones.
			Assert::IsTrue( Set.Skeleton.Bones.size() == 3, L"Incorrect LMI bone count." );
		}
	};

	TEST_CLASS( Assets )
	{
	public:
		TEST_METHOD( AddAndRemoveReferenceFromAssetPool )
		{
			AssetPool<int> Pool;
			Pool.Create( "test", 1 );
			Assert::IsTrue( Pool.GetAssets().size() == 1, L"Unexpected pool size after addition." );
			Pool.Remove( "test" );
			Assert::IsTrue( Pool.GetAssets().empty(), L"Unexpected pool size after removal.");
		}

		TEST_METHOD( AddAndRemovePointerFromAssetPool )
		{
			AssetPool<int*> Pool;
			int* Data = new int( 1 );
			Pool.Create( "test", Data );
			Assert::IsTrue( Pool.GetAssets().size() == 1, L"Unexpected pool size after addition." );
			Pool.Remove( "test" ); // Should handle the deletion of the pointer data itself as well.
			Assert::IsTrue( Pool.GetAssets().empty(), L"Unexpected pool size after removal." );
		}

		TEST_METHOD( DestroyReferenceAssetPool )
		{
			AssetPool<int> Pool;
			Pool.Create( "test", 1 );
			Assert::IsTrue( Pool.GetAssets().size() == 1, L"Unexpected pool size after addition." );
			Pool.Destroy();
			Assert::IsTrue( Pool.GetAssets().empty(), L"Unexpected pool size after removal." );
			Assert::IsTrue( Pool.Get().empty(), L"Unexpected pool size after removal." );
		}

		TEST_METHOD( DestroyPointerAssetPool )
		{
			AssetPool<int*> Pool;
			int* Data = new int( 1 );
			Pool.Create( "test", Data );
			Assert::IsTrue( Pool.GetAssets().size() == 1, L"Unexpected pool size after addition." );
			Pool.Destroy();
			Assert::IsTrue( Pool.GetAssets().empty(), L"Unexpected pool size after removal." );
			Assert::IsTrue( Pool.Get().empty(), L"Unexpected pool size after removal." );
		}
	};

	TEST_CLASS( Mathematics )
	{
	public:
		TEST_METHOD( Vector3DAddition )
		{
			const Vector3D A = Vector3D::One;
			const Vector3D B = Vector3D::One;
			const Vector3D C = A + B;
			const bool Success =
				Math::Equal( C.X, 2.0f ) &&
				Math::Equal( C.Y, 2.0f ) &&
				Math::Equal( C.Z, 2.0f );

			Assert::IsTrue( Success, L"Failed to add vectors." );
		}

		bool TestEuler( const Vector3D& Orientation ) const
		{
			const Matrix4D RotationMatrix = Math::EulerToMatrix( Orientation );
			const Vector3D ConvertedOrientation = Math::MatrixToEuler( RotationMatrix );

			const bool Equal =
				Math::Equal( Orientation.X, ConvertedOrientation.X, 1.0f ) &&
				Math::Equal( Orientation.Y, ConvertedOrientation.Y, 1.0f ) &&
				Math::Equal( Orientation.Z, ConvertedOrientation.Z, 1.0f )
			;

			const std::string String = "\n"
				"Old X: " + std::to_string( Orientation.X ) + "\n"
				"Old Y: " + std::to_string( Orientation.Y ) + "\n"
				"Old Z: " + std::to_string( Orientation.Z ) + "\n"
				"New X: " + std::to_string( ConvertedOrientation.X ) + "\n"
				"New Y: " + std::to_string( ConvertedOrientation.Y ) + "\n"
				"New Z: " + std::to_string( ConvertedOrientation.Z ) + "\n"
			;
			// Logger::WriteMessage( String.c_str() );

			return Equal;
		}

		/*TEST_METHOD( EulerToRotationMatrix )
		{
			const bool Equal =
				TestEuler( Vector3D( 90.0f, 0.0f, 0.0f ) ) &&
				TestEuler( Vector3D( 0.0f, 90.0f, 0.0f ) ) &&
				TestEuler( Vector3D( 0.0f, 0.0f, 90.0f ) ) &&
				TestEuler( Vector3D( 90.0f, 90.0f, 0.0f ) ) &&
				TestEuler( Vector3D( 90.0f, 90.0f, 90.0f ) ) &&
				TestEuler( Vector3D( -90.0f, 0.0f, 0.0f ) )
			;
			
			Assert::IsTrue( Equal, L"Failed to convert Euler angles to rotation matrix." );
		}*/

		void TestTransform( Vector3D SourcePosition, Vector3D SourceOrientation, Vector3D SourceScale )
		{
			std::wstring Text = L"\n";
			Text += L"pos: " + ToString( SourcePosition ) + L"\n";
			Text += L"siz: " + ToString( SourceScale ) + L"\n";
			Text += L"ori: " + ToString( SourceOrientation ) + L"\n";

			FTransform Transform = { SourcePosition, SourceOrientation, SourceScale };
			Matrix4D Matrix = Transform.GetTransformationMatrix();

			Matrix3D Rotation;
			Vector3D Position, Scale;
			Matrix.Decompose( Position, Rotation, Scale );

			Vector3D Direction = Rotation.Transform( WorldForward );
			Vector3D Orientation = Math::DirectionToEuler( Direction );

			Text += L"converted\n";
			Text += L"pos: " + ToString( Position ) + L"\n";
			Text += L"siz: " + ToString( Scale ) + L"\n";
			Text += L"dir: " + ToString( Direction ) + L"\n";
			Text += L"ori: " + ToString( Orientation ) + L"\n";

			if( Direction.Length() == 0 )
			{
				Text += L"Transformed direction has a magnitude of zero.";
				Assert::Fail( Text.c_str() );
			}

			if( !Math::Equal( SourcePosition, Position ) )
			{
				Text += L"Positions don't match.";
				Assert::Fail( Text.c_str() );
			}

			if( !Math::Equal( SourceScale, Scale ) )
			{
				Text += L"Sizes don't match.";
				Assert::Fail( Text.c_str() );
			}

			if( !Math::Equal( SourceOrientation, Orientation ) )
			{
				Text += L"Orientations don't match.";
				Assert::Fail( Text.c_str() );
			}
		}

		TEST_METHOD( DecompositionTransformMatrix4D )
		{
			TestTransform( { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -45.0f }, { 1.0f, 1.0f, 1.0f } );
			TestTransform( { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -180.0f }, { 1.0f, 1.0f, 1.0f } );
			TestTransform( { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -90.0f }, { 1.0f, 1.0f, 1.0f } );
		}
	};
}

namespace Containers
{
	TEST_CLASS( FixedVector )
	{
	public:
		TEST_METHOD( Test )
		{
			::FixedVector<int> Vector( 2 );
			Vector[0] = 5;
			Vector[1] = 7;

			Assert::IsTrue( Vector[0] == 5, L"First entry incorrect." );
			Assert::IsTrue( Vector[1] == 7, L"Second entry incorrect." );
			Assert::IsTrue( Vector.size() == 2, L"Size incorrect." );
			Assert::IsTrue( !Vector.empty(), L"Vector is considered empty." );
		}
	};

	TEST_CLASS( GreedyVector )
	{
	public:
		TEST_METHOD( Initialization )
		{
			::GreedyVector<int> Vector( 2 );
			Vector[0] = 5;
			Vector[1] = 7;

			Assert::IsTrue( Vector[0] == 5, L"First entry incorrect." );
			Assert::IsTrue( Vector[1] == 7, L"Second entry incorrect." );
		}

		TEST_METHOD( AddToExisting )
		{
			::GreedyVector<int> Vector( 2 );
			Vector[0] = 5;
			Vector[1] = 7;

			Vector.add( 3 );

			Assert::IsTrue( Vector[0] == 5, L"First entry incorrect." );
			Assert::IsTrue( Vector[1] == 7, L"Second entry incorrect." );
			Assert::IsTrue( Vector[2] == 3, L"Third entry incorrect." );
			Assert::IsTrue( Vector.size() == 3, L"Size incorrect." );
			Assert::IsTrue( Vector.capacity() == 4, L"Capacity incorrect." );
		}

		TEST_METHOD( AddToEmpty )
		{
			::GreedyVector<int> Vector( 0 );
			Vector.add( 3 );

			Assert::IsTrue( Vector[0] == 3, L"First entry incorrect." );
			Assert::IsTrue( Vector.size() == 1, L"Size incorrect." );
			Assert::IsTrue( Vector.capacity() == 1, L"Capacity incorrect." );
		}

		TEST_METHOD( AddMany )
		{
			::GreedyVector<int> Vector( 0 );

			for( size_t Index = 0; Index < 10000; Index++ )
			{
				Vector.add( 75 );
			}

			Assert::IsTrue( Vector[0] == 75, L"First entry incorrect." );
			Assert::IsTrue( Vector.size() == 10000, L"Size incorrect." );
			Assert::IsTrue( Vector.capacity() == 16384, L"Capacity incorrect." );
		}

		TEST_METHOD( Remove )
		{
			::GreedyVector<int> Vector( 0 );
			Vector.add( 1 );
			Vector.add( 2 );
			Vector.add( 3 );

			// Remove the entry at index 1.
			Vector.remove( 1 );

			Assert::IsTrue( Vector[1] == 3, L"Unexpected value after removal." );
			Assert::IsTrue( Vector.size() == 2, L"Unexpected size.");
			Assert::IsTrue( Vector.capacity() == 4, L"Unexpected capacity.");
		}

		TEST_METHOD( Clear )
		{
			::GreedyVector<int> Vector( 2 );
			Vector[0] = 5;
			Vector[1] = 7;

			Vector.add( 3 );

			Assert::IsTrue( Vector.size() == 3, L"Size incorrect." );
			Assert::IsTrue( Vector.capacity() == 4, L"Capacity incorrect." );
			Assert::IsTrue( !Vector.empty(), L"Vector is considered empty." );

			Vector.clear();

			Assert::IsTrue( Vector.size() == 0, L"Size changed unexpectedly." );
			Assert::IsTrue( Vector.capacity() == 4, L"Capacity changed unexpectedly." );
		}

		TEST_METHOD( Reserve )
		{
			::GreedyVector<int> Vector( 0 );
			Vector.reserve( 2048 );

			Assert::IsTrue( Vector.size() == 0, L"Size incorrect." );
			Assert::IsTrue( Vector.capacity() == 2048, L"Capacity incorrect." );
		}

		TEST_METHOD( Grow )
		{
			::GreedyVector<int> Vector( 2 );
			Vector.grow();

			Assert::IsTrue( Vector.size() == 0, L"Size incorrect." );
			Assert::IsTrue( Vector.capacity() == 4, L"Capacity incorrect." );
		}

		TEST_METHOD( Swap )
		{
			::GreedyVector<int> Vector( 2 );
			Vector[0] = 5;
			Vector[1] = 7;

			Vector.swap( 0, 1 );

			Assert::IsTrue( Vector[0] == 7, L"Swap failed (0)." );
			Assert::IsTrue( Vector[1] == 5, L"Swap failed (1)." );
		}

		TEST_METHOD( Pop )
		{
			::GreedyVector<int> Vector( 2 );
			Vector[0] = 5;
			Vector[1] = 7;

			Vector.pop();

			Assert::IsTrue( Vector.size() == 1, L"Pop failed.");
		}

		TEST_METHOD( Front )
		{
			::GreedyVector<int> Vector( 3 );
			Vector[0] = 5;
			Vector[1] = 3;
			Vector[2] = 7;

			Assert::IsTrue( Vector.front() == 5 );
		}

		TEST_METHOD( Back )
		{
			::GreedyVector<int> Vector( 3 );
			Vector[0] = 5;
			Vector[1] = 3;
			Vector[2] = 7;

			Assert::IsTrue( Vector.back() == 7 );
		}

		TEST_METHOD( SwapAndPop )
		{
			::GreedyVector<int> Vector( 0 );
			Vector.add( 5 );
			Vector.add( 7 );

			// Remove the entry at index 1.
			Vector.remove_unordered( 0 );

			Assert::IsTrue( Vector[0] == 7, L"Unexpected value after removal." );
			Assert::IsTrue( Vector.size() == 1, L"Unexpected size." );
			Assert::IsTrue( Vector.capacity() == 2, L"Unexpected capacity." );
		}

		TEST_METHOD( RangeBasedForLoop )
		{
			::GreedyVector<int> Vector( 10 );
			int Array[10] = { 0, 2, 4, 6, 8, 1, 3, 5, 7, 9 };

			// Copy the array into the vector.
			for( size_t Index = 0; Index < 10; Index++ )
			{
				Vector[Index] = Array[Index];
			}

			// Compare the vector data to the array using a ranged-based for loop.
			size_t Index = 0;
			for( auto& Entry : Vector )
			{
				Assert::IsTrue( Entry == Array[Index], L"For loop comparison failed." );
				++Index;
			}

			Assert::IsTrue( Vector.size() == 10, L"Size incorrect." );
			Assert::IsTrue( Vector.capacity() == 10, L"Capacity incorrect." );
		}
	};
}
