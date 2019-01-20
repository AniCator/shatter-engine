#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <Engine/Utility/Primitive.h>

namespace Test
{
	TEST_CLASS( PrimitiveTests )
	{
	public:
		TEST_METHOD( GenerateTriangle )
		{
			Logger::WriteMessage( "Generating a triangle primitive." );
			FPrimitive Primitive;
			CPrimitive::Triangle( Primitive, 1.0f );

			const uint32_t ValidCount = 3;
			Assert::AreEqual( Primitive.VertexCount, ValidCount );
		}

		TEST_METHOD( GeneratePlane )
		{
			Logger::WriteMessage( "Generating a plane primitive." );
			FPrimitive Primitive;
			CPrimitive::Plane( Primitive, 1.0f );

			const uint32_t ValidCount = 4;
			Assert::AreEqual( Primitive.VertexCount, ValidCount );
		}

		TEST_METHOD( GenerateCube )
		{
			Logger::WriteMessage( "Generating a cube primitive." );
			FPrimitive Primitive;
			CPrimitive::Cube( Primitive, 1.0f );

			const uint32_t ValidCount = 8;
			Assert::AreEqual( Primitive.VertexCount, ValidCount );
		}

		TEST_METHOD( GenerateCircle )
		{
			Logger::WriteMessage( "Generating a circle primitive with 32 segments." );
			FPrimitive Primitive;
			CPrimitive::Circle( Primitive, 1.0f, 32 );

			const uint32_t ValidCount = 32;
			Assert::AreEqual( Primitive.VertexCount, ValidCount );
		}

		TEST_METHOD( GenerateSphere )
		{
			Logger::WriteMessage( "Generating a sphere primitive with 32 segments and 16 rings." );
			FPrimitive Primitive;
			CPrimitive::Sphere( Primitive, 1.0f, 32, 16 );

			const uint32_t ValidCount = 482;
			Assert::AreEqual( Primitive.VertexCount, ValidCount );
		}

		TEST_METHOD( GenerateCone )
		{
			Logger::WriteMessage( "Generating a cone primitive with 4 sides." );
			FPrimitive Primitive;
			CPrimitive::Cone( Primitive, 1.0f, 4 );

			const uint32_t ValidCount = 5;
			Assert::AreEqual( Primitive.VertexCount, ValidCount );
		}

		TEST_METHOD( GenerateTorus )
		{
			Logger::WriteMessage( "Generating a torus primitive with 48 major segments and 12 minor segments." );
			FPrimitive Primitive;
			CPrimitive::Torus( Primitive, 1.0f, 48, 12 );

			const uint32_t ValidCount = 576;
			Assert::AreEqual( Primitive.VertexCount, ValidCount );
		}

		TEST_METHOD( GenerateGrid )
		{
			Logger::WriteMessage( "Generating a grid primitive with 10 subdivisions on the X-axis and 10 subdivisions on the Y-axis." );
			FPrimitive Primitive;
			CPrimitive::Grid( Primitive, 1.0f, 10, 10 );

			const uint32_t ValidCount = 100;
			Assert::AreEqual( Primitive.VertexCount, ValidCount );
		}
	};
}
