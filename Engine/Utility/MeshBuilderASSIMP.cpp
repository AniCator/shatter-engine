// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MeshBuilder.h"

#include <map>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Display/UserInterface.h>

#include <ThirdParty/assimp/include/assimp/Importer.hpp>
#include <ThirdParty/assimp/include/assimp/postprocess.h>

#include <ThirdParty/assimp/include/assimp/scene.h>
#include <ThirdParty/assimp/include/assimp/mesh.h>

struct FMeshData
{
	FMeshData()
	{
		Vertices.reserve( 10000 );
		Indices.reserve( 10000 );
	}

	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;
};

void PrintMatrix( const aiMatrix4x4& Transform )
{
	Log::Event( "%.3f %.3f %.3f %.3f\n%.3f %.3f %.3f %.3f\n%.3f %.3f %.3f %.3f\n%.3f %.3f %.3f %.3f\n\n", 
		Transform.a1, Transform.a2, Transform.a3, Transform.a4, 
		Transform.b1, Transform.b2, Transform.b3, Transform.b4,
		Transform.c1, Transform.c2, Transform.c3, Transform.c4,
		Transform.d1, Transform.d2, Transform.d3, Transform.d4 );
}

void AddMesh( const aiMatrix4x4& Transform, const aiScene* Scene, const aiMesh* Mesh, FMeshData& MeshData )
{
	const bool HasInvalidPrimitive = ( Mesh->mPrimitiveTypes & ( aiPrimitiveType_POINT | aiPrimitiveType_LINE | aiPrimitiveType_POLYGON ) ) != 0;
	if( HasInvalidPrimitive )
	{
		Log::Event( Log::Warning, "Sub-mesh \"%s\" contains invalid primitive type. (point, line, polygon)", Mesh->mName.C_Str() );
		return;
	}

	const size_t IndexOffset = MeshData.Vertices.size();
	for( size_t VertexIndex = 0; VertexIndex < Mesh->mNumVertices; VertexIndex++ )
	{
		FVertex NewVertex;

		const auto& Vertex = Mesh->mVertices[VertexIndex];
		auto TransformedVector = Transform * Vertex;
		NewVertex.Position = Vector3D( TransformedVector.z, TransformedVector.x, TransformedVector.y );

		const auto& Normal = Mesh->mNormals[VertexIndex];
		auto Transposed = Transform;
		// Transposed = Transposed.Transpose();
		auto TransformedNormal = aiMatrix3x3( Transform ) * Normal;
		NewVertex.Normal = Vector3D( TransformedNormal.z, TransformedNormal.x, TransformedNormal.y );

		if( Mesh->HasTextureCoords( 0 ) )
		{
			const auto& Coordinate = Mesh->mTextureCoords[0][VertexIndex];
			NewVertex.TextureCoordinate = Vector2D( Coordinate.x, Coordinate.y );
		}
		else
		{
			NewVertex.TextureCoordinate = Vector2D( 0.5f, 0.5f );
		}

		if( Mesh->HasVertexColors( 0 ) )
		{
			const auto& Color = Mesh->mColors[0][VertexIndex];
			NewVertex.Color = Vector3D( Color.r, Color.g, Color.b );
		}
		else
		{
			NewVertex.Color = Vector3D( 1.0f, 1.0f, 1.0f );
		}

		MeshData.Vertices.emplace_back( NewVertex );
	}

	for( size_t FaceIndex = 0; FaceIndex < Mesh->mNumFaces; FaceIndex++ )
	{
		const auto& Face = Mesh->mFaces[FaceIndex];
		if( Face.mNumIndices == 3 )
		{
			for( size_t Index = 0; Index < Face.mNumIndices; Index++ )
			{
				MeshData.Indices.emplace_back( IndexOffset + Face.mIndices[Index] );
			}
		}
	}
}

void ParseNodes( const aiMatrix4x4& Transform, const aiScene* Scene, const aiNode* Node, FMeshData& MeshData )
{
	for( size_t MeshIndex = 0; MeshIndex < Node->mNumMeshes; MeshIndex++ )
	{
		AddMesh( Node->mTransformation, Scene, Scene->mMeshes[Node->mMeshes[MeshIndex]], MeshData );
	}

	for( size_t ChildIndex = 0; ChildIndex < Node->mNumChildren; ChildIndex++ )
	{
		const auto Child = Node->mChildren[ChildIndex];
		ParseNodes( Transform, Scene, Child, MeshData );
	}
}

void MeshBuilder::ASSIMP( FPrimitive<>& Primitive, const CFile& File )
{
	Log::Event( "Running ASSIMP to import \"%s\"\n", File.Location().c_str() );

	Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile( File.Location().c_str(), 
		aiProcess_CalcTangentSpace			| // Calculate tangents and bitangents if possible.
		aiProcess_JoinIdenticalVertices		| // Join identical vertices/ optimize indexing.
		aiProcess_ValidateDataStructure		| // Perform a full validation of the loader's output.
		aiProcess_ImproveCacheLocality		| // Improve the cache locality of the output vertices.
		aiProcess_RemoveRedundantMaterials	| // Remove redundant materials.
		aiProcess_FindDegenerates			| // Remove degenerated polygons from the import.
		aiProcess_FindInvalidData			| // Detect invalid model data, such as invalid normal vectors.
		aiProcess_GenUVCoords				| // Convert spherical, cylindrical, box and planar mapping to proper UVs.
		aiProcess_TransformUVCoords			| // Preprocess UV transformations (scaling, translation ...).
		aiProcess_FindInstances				| // Search for instanced meshes and remove them by references to one master.
		aiProcess_LimitBoneWeights			| // Limit bone weights to 4 per vertex.
		aiProcess_OptimizeMeshes			| // Join small meshes, if possible.
		aiProcess_SplitByBoneCount			| // Split meshes with too many bones.

		aiProcess_GenSmoothNormals			| // Generate smooth normal vectors if not existing.
		aiProcess_SplitLargeMeshes			| // Split large, unrenderable meshes into submeshes.
		aiProcess_Triangulate				| // Triangulate polygons with more than 3 edges.
		aiProcess_SortByPType				| // Make 'clean' meshes which consist of a single typ of primitives.
		0
	);

	Log::Event( "Meshes found: %i\n", Scene->mNumMeshes );

	if( Scene->mNumMeshes > 0 )
	{
		auto Transform = Scene->mRootNode->mTransformation;
		FMeshData MeshData;
		ParseNodes( Transform, Scene, Scene->mRootNode, MeshData );

		if( true )
		{
			std::vector<FVertex> FatVertices;
			std::vector<uint32_t> FatIndices;
			std::map<FVertex, uint32_t> IndexMap;

			for( size_t Index = 0; Index < MeshData.Indices.size(); Index++ )
			{
				FVertex Vertex = MeshData.Vertices[MeshData.Indices[Index]];

				uint32_t FatIndex = 0;
				const bool ExistingVertex = FindVertex( Vertex, IndexMap, FatIndex );
				if( ExistingVertex )
				{
					FatIndices.emplace_back( FatIndex );
				}
				else
				{
					FatVertices.emplace_back( Vertex );
					FatIndices.emplace_back( FatVertices.size() - 1 );
				}
			}

			FVertex* VertexArray = new FVertex[FatVertices.size()];
			for( size_t Index = 0; Index < FatVertices.size(); Index++ )
			{
				VertexArray[Index] = FatVertices[Index];
			}

			glm::uint* IndexArray = new glm::uint[FatIndices.size()];
			for( size_t Index = 0; Index < FatIndices.size(); Index++ )
			{
				IndexArray[Index] = FatIndices[Index];
			}

			Primitive.Vertices = VertexArray;
			Primitive.VertexCount = static_cast<uint32_t>( FatVertices.size() );
			Primitive.Indices = IndexArray;
			Primitive.IndexCount = static_cast<uint32_t>( FatIndices.size() );
			Primitive.HasNormals = true;
		}
		else
		{
			FVertex* VertexArray = new FVertex[MeshData.Vertices.size()];
			for( size_t Index = 0; Index < MeshData.Vertices.size(); Index++ )
			{
				VertexArray[Index] = MeshData.Vertices[Index];
			}

			glm::uint* IndexArray = new glm::uint[MeshData.Indices.size()];
			for( size_t Index = 0; Index < MeshData.Indices.size(); Index++ )
			{
				IndexArray[Index] = MeshData.Indices[Index];
			}

			Primitive.Vertices = VertexArray;
			Primitive.VertexCount = static_cast<uint32_t>( MeshData.Vertices.size() );
			Primitive.Indices = IndexArray;
			Primitive.IndexCount = static_cast<uint32_t>( MeshData.Indices.size() );
			Primitive.HasNormals = true;
		}
	}
}