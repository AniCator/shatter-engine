// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Noise.h"

#include <Engine/Resource/Assets.h>

Vector2D RandomCellular2D( Vector2D Hash ) {
	// Use the hash to generate the coordinates.
	float X = Hash.Dot( Vector2D( 127.1f, 311.7f ) );
	float Y = Hash.Dot( Vector2D( 269.5f, 183.3f ) );

	// Create the vector and perform some sine magic.
	Vector2D Random = Vector2D( X, Y );
	Random.X = std::sinf( Random.X );
	Random.Y = std::sinf( Random.Y );

	// Scale it up with a magic number.
	Random *= 43758.5453;

	// And calculation the fractional remainder.
	Random.X = Random.X - std::floor( Random.X );
	Random.Y = Random.Y - std::floor( Random.Y );

	return Random;
}

Vector3D RandomCellular3D( Vector3D Hash ) {
	// Use the hash to generate the coordinates.
	float X = Hash.Dot( Vector3D( 127.1f, 311.7f, 124.1f ) );
	float Y = Hash.Dot( Vector3D( 269.5f, 183.3f, 232.5f ) );
	float Z = Hash.Dot( Vector3D( 183.4f, 310.7f, 117.1f ) );

	// Create the vector and perform some sine magic.
	Vector3D Random = Vector3D( X, Y, Z );
	Random.X = std::sinf( Random.X );
	Random.Y = std::sinf( Random.Y );
	Random.Z = std::sinf( Random.Z );

	// Scale it up with a magic number.
	Random *= 43758.5453;

	// And calculation the fractional remainder.
	Random.X = Random.X - std::floor( Random.X );
	Random.Y = Random.Y - std::floor( Random.Y );
	Random.Z = Random.Z - std::floor( Random.Z );

	return Random;
}

constexpr float Scale = 6.0f;

CTexture* TextureCellular2D = nullptr;
CTexture* Noise::Cellular2D()
{
	if( TextureCellular2D )
		return TextureCellular2D;

	// Allocate texture data.
	constexpr size_t Size = 256;
	constexpr size_t Channels = 1;
	constexpr size_t Length = Size * Size * Channels;
	auto* Data = new unsigned char[Length];

	float MaximumDistance = 0.0f;
	std::vector<float> Distances;
	Distances.resize( Length );

	for( size_t Index = 0; Index < Length; Index++ )
	{
		// Calculate our X, Y and Z position.
		const size_t X = Index % Size;
		const size_t Y = Index / Size;

		// Convert to a coordinate in the range of [0-1].
		const Vector2D Coordinate = Vector2D(
			static_cast<float>( X ) / static_cast<float>( Size ),
			static_cast<float>( Y ) / static_cast<float>( Size )
		);

		float MinimumDistance = 1.0f;
		Vector2D Scaled = Coordinate * Scale;
		Vector2D Tile = { std::floor( Scaled.X ), std::floor( Scaled.Y ) };
		Vector2D Fraction = { Scaled.X - Tile.X, Scaled.Y - Tile.Y };

		for( int OffsetY = -1; OffsetY <= 1; OffsetY++ )
		{
			for( int OffsetX = -1; OffsetX <= 1; OffsetX++ )
			{
				Vector2D Offset = Vector2D( OffsetX, OffsetY );
				Vector2D Cell = Tile + Offset;

				// Wrap-around.
				Cell.X = std::fmod( Cell.X + Scale, Scale );
				Cell.Y = std::fmod( Cell.Y + Scale, Scale );

				Vector2D Point = RandomCellular2D( Cell );
				Vector2D Difference = Offset + Point - Fraction;
				MinimumDistance = Math::Min( MinimumDistance, Difference.Length() );
			}
		}

		MaximumDistance = Math::Max( MinimumDistance, MaximumDistance );
		Distances[Index] = MinimumDistance;
	}

	// Correct the brightness.
	for( size_t Index = 0; Index < Length; Index++ )
	{
		Data[Index] = ( Distances[Index] / MaximumDistance ) * 255.0f;
	}

	TextureContext Context;
	Context.Name = "noise_cellular_2d";
	Context.Data = Data;
	Context.Width = Size;
	Context.Height = Size;
	Context.Depth = 1;
	Context.Channels = Channels;
	Context.Mode = EFilteringMode::Bilinear;
	Context.Format = EImageFormat::R8;
	Context.GenerateMipMaps = true;
	TextureCellular2D = CAssets::Get().CreateNamedTexture( Context );

	return TextureCellular2D;
}

CTexture* TextureCellular3D = nullptr;
CTexture* Noise::Cellular3D()
{
	if( TextureCellular3D )
		return TextureCellular3D;

	// Allocate texture data.
	constexpr size_t Size = 64;
	constexpr size_t Channels = 1;
	constexpr size_t Length = Size * Size * Size * Channels;
	auto* Data = new unsigned char[Length];

	float MaximumDistance = 0.0f;
	std::vector<float> Distances;
	Distances.resize( Length );

	for( size_t Index = 0; Index < Length; Index++ )
	{
		// Calculate our X, Y and Z position.
		const size_t X = Index % Size;
		const size_t Y = ( Index / Size ) % Size;
		const size_t Z = ( Index / Size ) / Size;

		// Convert to a coordinate in the range of [0-1].
		const Vector3D Coordinate = Vector3D(
			static_cast<float>( X ) / static_cast<float>( Size ),
			static_cast<float>( Y ) / static_cast<float>( Size ),
			static_cast<float>( Z ) / static_cast<float>( Size )
		);

		float MinimumDistance = 1.0f;
		Vector3D Scaled = Coordinate * Scale;
		Vector3D Tile = { std::floor( Scaled.X ), std::floor( Scaled.Y ), std::floor( Scaled.Z ) };
		Vector3D Fraction = { Scaled.X - Tile.X, Scaled.Y - Tile.Y, Scaled.Z - Tile.Z };

		for( int OffsetZ = -1; OffsetZ <= 1; OffsetZ++ )
		{
			for( int OffsetY = -1; OffsetY <= 1; OffsetY++ )
			{
				for( int OffsetX = -1; OffsetX <= 1; OffsetX++ )
				{
					Vector3D Offset = Vector3D( OffsetX, OffsetY, OffsetZ );
					Vector3D Cell = Tile + Offset;

					// Wrap-around.
					Cell.X = std::fmod( Cell.X + Scale, Scale );
					Cell.Y = std::fmod( Cell.Y + Scale, Scale );
					Cell.Z = std::fmod( Cell.Z + Scale, Scale );

					Vector3D Point = RandomCellular3D( Cell );
					Vector3D Difference = Offset + Point - Fraction;
					MinimumDistance = Math::Min( MinimumDistance, Difference.Length() );
				}
			}
		}

		MaximumDistance = Math::Max( MinimumDistance, MaximumDistance );
		Distances[Index] = MinimumDistance;
	}

	// Correct the brightness.
	for( size_t Index = 0; Index < Length; Index++ )
	{
		Data[Index] = ( Distances[Index] / MaximumDistance ) * 255.0f;
	}

	TextureContext Context;
	Context.Name = "noise_cellular_3d";
	Context.Data = Data;
	Context.Width = Size;
	Context.Height = Size;
	Context.Depth = Size;
	Context.Channels = Channels;
	Context.Mode = EFilteringMode::Bilinear;
	Context.Format = EImageFormat::R8;
	Context.GenerateMipMaps = true;
	TextureCellular3D = CAssets::Get().CreateNamedTexture( Context );

	return TextureCellular3D;
}
