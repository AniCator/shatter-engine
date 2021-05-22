// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MeshBuilder.h"

#include <map>

#include <Engine/Animation/AnimationSet.h>
#include <Engine/Animation/Skeleton.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Display/UserInterface.h>


#include <ThirdParty/assimp/include/assimp/Importer.hpp>
#include <ThirdParty/assimp/include/assimp/postprocess.h>

#include <ThirdParty/assimp/include/assimp/scene.h>
#include <ThirdParty/assimp/include/assimp/mesh.h>

struct NamedBone
{
	std::string Name;
	Bone* Bone;
};

struct FMeshData
{
	FMeshData()
	{
		Vertices.reserve( 10000 );
		Indices.reserve( 10000 );

		Skeleton = nullptr;
		Parent = nullptr;
	}

	std::vector<FVertex> Vertices;
	std::vector<uint32_t> Indices;

	Skeleton* Skeleton;
	aiMatrix4x4 InverseTransform;
	aiNode* Parent;

	std::map<std::string, const aiNode*> Nodes;
	std::map<std::string, const aiNode*> BoneToNode;
	std::map<std::string, NamedBone> NodeToBone;
	std::map<std::string, Animation> Animations;
};

template<typename MatrixType>
void PrintMatrix( const MatrixType& Transform )
{
	Log::Event( "%.3f %.3f %.3f %.3f\n%.3f %.3f %.3f %.3f\n%.3f %.3f %.3f %.3f\n%.3f %.3f %.3f %.3f\n\n", 
		Transform[0][0], Transform[1][0], Transform[2][0], Transform[3][0],
		Transform[0][1], Transform[1][1], Transform[2][1], Transform[3][1],
		Transform[0][2], Transform[1][2], Transform[2][2], Transform[3][2],
		Transform[0][3], Transform[1][3], Transform[2][3], Transform[3][3] );
}

// Converts ASSIMP style matrix to Shatter's convention.
template<typename MatrixType>
void ConvertMatrix( Matrix4D& TargetMatrix, const MatrixType& SourceMatrix )
{
	// ASSIMP is row first. (character = row, number = column)
	// ASSIMP's [] operator gets you the character for [0]
	// [0][0] would be a1, [0][1] would be a2
	// This function converts the rows to column major.
	TargetMatrix[0][0] = SourceMatrix[0][0]; // a1 to [0][0]
	TargetMatrix[0][1] = SourceMatrix[1][0]; // b1 to [0][1]
	TargetMatrix[0][2] = SourceMatrix[2][0]; // c1 to [0][2]
	TargetMatrix[0][3] = SourceMatrix[3][0]; // d1 to [0][3]

	TargetMatrix[1][0] = SourceMatrix[0][1]; // a2 to [1][0]
	TargetMatrix[1][1] = SourceMatrix[1][1]; // b2 to [1][1]
	TargetMatrix[1][2] = SourceMatrix[2][1]; // c2 to [1][2]
	TargetMatrix[1][3] = SourceMatrix[3][1]; // d2 to [1][3]

	TargetMatrix[2][0] = SourceMatrix[0][2]; // a3 to [2][0]
	TargetMatrix[2][1] = SourceMatrix[1][2]; // b3 to [2][1]
	TargetMatrix[2][2] = SourceMatrix[2][2]; // c3 to [2][2]
	TargetMatrix[2][3] = SourceMatrix[3][2]; // d3 to [2][3]

	TargetMatrix[3][0] = SourceMatrix[0][3]; // a4 to [3][0]
	TargetMatrix[3][1] = SourceMatrix[1][3]; // b4 to [3][1]
	TargetMatrix[3][2] = SourceMatrix[2][3]; // c4 to [3][2]
	TargetMatrix[3][3] = SourceMatrix[3][3]; // d4 to [3][3]
}

template<typename MatrixType>
void CopyMatrix( Matrix4D& TargetMatrix, const MatrixType& SourceMatrix )
{
	TargetMatrix[0][0] = SourceMatrix[0][0];
	TargetMatrix[0][1] = SourceMatrix[0][1];
	TargetMatrix[0][2] = SourceMatrix[0][2];
	TargetMatrix[0][3] = SourceMatrix[0][3];

	TargetMatrix[1][0] = SourceMatrix[1][0];
	TargetMatrix[1][1] = SourceMatrix[1][1];
	TargetMatrix[1][2] = SourceMatrix[1][2];
	TargetMatrix[1][3] = SourceMatrix[1][3];

	TargetMatrix[2][0] = SourceMatrix[2][0];
	TargetMatrix[2][1] = SourceMatrix[2][1];
	TargetMatrix[2][2] = SourceMatrix[2][2];
	TargetMatrix[2][3] = SourceMatrix[2][3];

	TargetMatrix[3][0] = SourceMatrix[3][0];
	TargetMatrix[3][1] = SourceMatrix[3][1];
	TargetMatrix[3][2] = SourceMatrix[3][2];
	TargetMatrix[3][3] = SourceMatrix[3][3];
}

// Used to orient from FBX/ASSIMP space into Shatter space.
aiMatrix3x3 GetWorldRotationMatrix3x3()
{
	aiMatrix3x3 RotationX;
	aiMatrix3x3 RotationY;
	aiMatrix3x3 RotationZ;
	aiMatrix3x3::Rotation( glm::radians( 90.0f ), aiVector3D( 1.0f, 0.0f, 0.0f ), RotationX ); // Pitch
	aiMatrix3x3::Rotation( glm::radians( 0.0f ), aiVector3D( 0.0f, 1.0f, 0.0f ), RotationY ); // Yaw
	aiMatrix3x3::Rotation( glm::radians( 0.0f ), aiVector3D( 0.0f, 0.0f, 1.0f ), RotationZ ); // Roll
	const aiMatrix3x3 RotationMatrix = aiMatrix3x3( RotationZ * RotationY * RotationX );

	return RotationMatrix;
}

aiMatrix4x4 GetWorldRotationMatrix()
{
	aiMatrix3x3 RotationX;
	aiMatrix3x3 RotationY;
	aiMatrix3x3 RotationZ;
	aiMatrix3x3::Rotation( glm::radians( 90.0f ), aiVector3D( 1.0f, 0.0f, 0.0f ), RotationX ); // Pitch
	aiMatrix3x3::Rotation( glm::radians( 0.0f ), aiVector3D( 0.0f, 1.0f, 0.0f ), RotationY ); // Yaw
	aiMatrix3x3::Rotation( glm::radians(0.0f ), aiVector3D( 0.0f, 0.0f, 1.0f ), RotationZ ); // Roll
	const aiMatrix4x4 RotationMatrix = aiMatrix4x4( RotationZ * RotationY * RotationX );

	return RotationMatrix;
}

void Print( const Vector4D& Column )
{
	Log::Event( "%.3f %.3f %.3f %.3f\n", Column.X, Column.Y, Column.Z, Column.W );
}

void Print( const std::string& Name, const Matrix4D& Matrix )
{
	Log::Event( "%s\n", Name.c_str() );
	Print( Matrix.Columns[0] );
	Print( Matrix.Columns[1] );
	Print( Matrix.Columns[2] );
	Print( Matrix.Columns[3] );
}

void Print( const ai_real& a1, const ai_real& a2, const ai_real& a3, const ai_real& a4 )
{
	Log::Event( "%.3f %.3f %.3f %.3f\n", a1, a2, a3, a4 );
}

void Print( const std::string& Name, const aiMatrix4x4& Matrix )
{
	Log::Event( "%sASSIMP\n", Name.c_str() );
	Print( Matrix.a1, Matrix.a2, Matrix.a3, Matrix.a4 );
	Print( Matrix.b1, Matrix.b2, Matrix.b3, Matrix.b4 );
	Print( Matrix.c1, Matrix.c2, Matrix.c3, Matrix.c4 );
	Print( Matrix.d1, Matrix.d2, Matrix.d3, Matrix.d4 );
}

// This function is run in AddMesh and takes in an IndexOffset that is used to determine where the sub-mesh indices are located.
void UpdateSkeleton( const aiMatrix4x4& Transform, const aiScene* Scene, const aiMesh* Mesh, const aiNode* Node, FMeshData& MeshData, const size_t& IndexOffset )
{
	if( !Scene->HasAnimations() || !MeshData.Skeleton )
		return;

	// aiMatrix3x3 RotationMatrix;
	// aiMatrix3x3::Rotation( glm::radians( -90.0f ), aiVector3D( 0.0f, 0.0f, 1.0f ), RotationMatrix );
	// aiMatrix4x4 RotMat = aiMatrix4x4( RotationMatrix );
	const auto RotationMatrix = GetWorldRotationMatrix();
	auto RotationMatrixInverse = RotationMatrix;
	RotationMatrixInverse.Inverse();

	// Log::Event( "Calculating bone weights.\n" );
	Skeleton& Skeleton = *MeshData.Skeleton;
	if( Skeleton.Weights.size() != Mesh->mNumVertices )
	{
		Skeleton.Weights.resize( Mesh->mNumVertices );
	}

	// Assign the bone weights for each vertex.
	for( size_t BoneIndex = 0; BoneIndex < Mesh->mNumBones; BoneIndex++ )
	{
		auto* Bone = Mesh->mBones[BoneIndex];
		// Log::Event( "Importing bone \"%s\" (%i).\n", Bone->mName.C_Str(), BoneIndex );

		for( size_t WeightIndex = 0; WeightIndex < Bone->mNumWeights; WeightIndex++ )
		{
			size_t VertexID = IndexOffset + Bone->mWeights[WeightIndex].mVertexId;
			VertexWeight& Weight = Skeleton.Weights[VertexID];

			for( size_t InfluenceIndex = 0; InfluenceIndex < MaximumInfluences; InfluenceIndex++ )
			{
				// if( Weight.Weight[InfluenceIndex] == 0.0f )
				{
					Weight.Index[InfluenceIndex] = BoneIndex;
					Weight.Weight[InfluenceIndex] = Bone->mWeights[InfluenceIndex].mWeight;

					// Log::Event( "Weight %.2f.\n", Weight.Weight[InfluenceIndex] );
					break;
				}
			}
		}
	}

	for( size_t Index = 0; Index < MeshData.Vertices.size(); Index++ )
	{
		auto& Vertex = MeshData.Vertices[Index];
		VertexWeight& Weight = Skeleton.Weights[Index];
		Vertex.Bone = Vector4D( 
			Weight.Index[0], 
			Weight.Index[1], 
			Weight.Index[2], 
			Weight.Index[3] );

		Vertex.Weight = Vector4D( 
			Weight.Weight[0], 
			Weight.Weight[1], 
			Weight.Weight[2], 
			Weight.Weight[3] );
	}

	// Log::Event( "Calculating bone matrices.\n" );
	if( Skeleton.Bones.size() != Mesh->mNumBones )
	{
		Skeleton.Bones.resize( Mesh->mNumBones );
	}

	if( Skeleton.MatrixNames.size() != Mesh->mNumBones )
	{
		Skeleton.MatrixNames.resize( Mesh->mNumBones );
	}

	auto GlobalTransformationInverse = Transform;
	GlobalTransformationInverse.Inverse();
	//if( MeshData.Parent )
	//{
	//	GlobalTransformation = MeshData.Parent->mTransformation;// *GlobalTransformation;
	//}

	// ConvertMatrix( MeshData.Skeleton->GlobalMatrix, GlobalTransformation );

	// Map the bones to their respective nodes and viceversa.
	for( size_t BoneIndex = 0; BoneIndex < Mesh->mNumBones; BoneIndex++ )
	{
		const auto* Bone = Mesh->mBones[BoneIndex];

		aiMatrix4x4 BoneTransformation;		
		auto BoneIterator = MeshData.Nodes.find( Bone->mName.C_Str() );
		if( BoneIterator != MeshData.Nodes.end() )
		{
			BoneTransformation = BoneIterator->second->mTransformation;
		}

		auto InverseOffsetMatrix = Bone->mOffsetMatrix * BoneTransformation;
		InverseOffsetMatrix.Inverse();
	
		// Assign the offset matrix to the bone in column major format.
		ConvertMatrix( Skeleton.Bones[BoneIndex].Matrix, InverseOffsetMatrix );

		Skeleton.MatrixNames[BoneIndex] = Bone->mName.C_Str();

		MeshData.BoneToNode.insert_or_assign( Bone->mName.C_Str(), nullptr );

		NamedBone NodeBone;
		NodeBone.Name = Bone->mName.C_Str();
		NodeBone.Bone = &Skeleton.Bones[BoneIndex];
		NodeBone.Bone->Index = BoneIndex; // TODO: Check if this is ok here or if we should use the line in UpdateBoneHierarchy instead.
		MeshData.NodeToBone.insert_or_assign( Bone->mName.C_Str(), NodeBone );
	}
}

void UpdateBoneNode( const aiNode* Node, FMeshData& MeshData )
{
	if( MeshData.BoneToNode.find( Node->mName.C_Str() ) != MeshData.BoneToNode.end() )
	{
		MeshData.BoneToNode.insert_or_assign( Node->mName.C_Str(), Node );
	}
}

void UpdateBoneHierarchy( const aiNode* Node, FMeshData& MeshData )
{
	const bool ValidBone = MeshData.NodeToBone.find( Node->mName.C_Str() ) != MeshData.NodeToBone.end();
	
	Skeleton& Skeleton = *MeshData.Skeleton;

	if( ValidBone )
	{
		for( size_t BoneIndex = 0; BoneIndex < Skeleton.Bones.size(); BoneIndex++ )
		{
			auto& Bone = Skeleton.Bones[BoneIndex];
			auto& NamedBone = MeshData.NodeToBone[Node->mName.C_Str()];

			const aiMatrix4x4& NodeTransformation = MeshData.BoneToNode[NamedBone.Name]->mTransformation;
			aiMatrix4x4 NodeTransformationInverse = NodeTransformation;
			NodeTransformationInverse.Inverse();
			Matrix4D Transformation;
			ConvertMatrix( Bone.ModelMatrix, NodeTransformation );
			ConvertMatrix( Bone.InverseModelMatrix, NodeTransformationInverse );
			// Bone.Matrix = Bone.ModelMatrix * Bone.InverseModelMatrix * Bone.Matrix;
			// Bone.Index = BoneIndex;
		}
	}

	if( Node->mParent )
	{
		const bool ValidBone = MeshData.NodeToBone.find( Node->mName.C_Str() ) != MeshData.NodeToBone.end();
		const bool ValidParent = MeshData.NodeToBone.find( Node->mParent->mName.C_Str() ) != MeshData.NodeToBone.end();
		if( ValidBone && ValidParent )
		{
			auto& Bone = MeshData.NodeToBone[Node->mName.C_Str()];
			auto& Parent = MeshData.NodeToBone[Node->mParent->mName.C_Str()];

			if( Parent.Bone )
			{
				Bone.Bone->ParentIndex = Parent.Bone->Index;
				Parent.Bone->Children.emplace_back( Bone.Bone->Index );
			}
			else
			{
				Bone.Bone->ParentIndex = -1;
			}
		}
	}

	for( size_t ChildIndex = 0; ChildIndex < Node->mNumChildren; ChildIndex++ )
	{
		auto Child = Node->mChildren[ChildIndex];
		UpdateBoneHierarchy( Child, MeshData );
	}
}

void AddMesh( const aiMatrix4x4& Transform, const aiScene* Scene, const aiMesh* Mesh, const aiNode* Node, FMeshData& MeshData )
{
	const bool HasInvalidPrimitive = ( Mesh->mPrimitiveTypes & ( aiPrimitiveType_POINT | aiPrimitiveType_LINE | aiPrimitiveType_POLYGON ) ) != 0;
	if( HasInvalidPrimitive )
	{
		Log::Event( Log::Warning, "Sub-mesh \"%s\" contains invalid primitive type. (point, line, polygon)", Mesh->mName.C_Str() );
		return;
	}

	const bool HasBones = Mesh->mNumBones > 0;
	const auto Identity = aiMatrix4x4();
	// const auto NodeTransformation = HasBones ? Identity : Transform;
	const auto NodeTransformation = Transform;
	
	const size_t IndexOffset = MeshData.Vertices.size();
	for( size_t VertexIndex = 0; VertexIndex < Mesh->mNumVertices; VertexIndex++ )
	{
		FVertex NewVertex;

		const auto& Vertex = Mesh->mVertices[VertexIndex];
		auto TransformedVector = NodeTransformation * Vertex;
		NewVertex.Position = Vector3D( TransformedVector.x, TransformedVector.y, TransformedVector.z );

		const auto& Normal = Mesh->mNormals[VertexIndex];
		auto TransformedNormal = aiMatrix3x3( NodeTransformation ) * Normal;
		NewVertex.Normal = Vector3D( TransformedNormal.x, TransformedNormal.y, TransformedNormal.z );

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

		NewVertex.Bone = Vector4D( -1.0f, -1.0f, -1.0f, -1.0f );
		NewVertex.Weight = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f );

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

	UpdateSkeleton( Transform, Scene, Mesh, Node, MeshData, IndexOffset );
}

void ParseNodes( const aiMatrix4x4& Transform, const aiScene* Scene, const aiNode* Node, FMeshData& MeshData )
{
	MeshData.Nodes.insert_or_assign( Node->mName.C_Str(), Node );

	if( Node->mParent )
	{
		MeshData.Parent = Node->mParent;
	}

	for( size_t MeshIndex = 0; MeshIndex < Node->mNumMeshes; MeshIndex++ )
	{
		const auto& Mesh = Scene->mMeshes[Node->mMeshes[MeshIndex]];
		AddMesh( Transform * Node->mTransformation, Scene, Mesh, Node, MeshData );
	}

	UpdateBoneNode( Node, MeshData );

	for( size_t ChildIndex = 0; ChildIndex < Node->mNumChildren; ChildIndex++ )
	{
		const auto Child = Node->mChildren[ChildIndex];
		ParseNodes( Transform, Scene, Child, MeshData );
	}
}

bool CompareKeys( const Key& A, const Key& B )
{
	return A.Time < B.Time;
}

void ParseAnimations( const aiMatrix4x4& Transform, const aiScene* Scene, FMeshData& MeshData )
{
	if( !Scene->HasAnimations() || !MeshData.Skeleton )
		return;

	for( size_t AnimationIndex = 0; AnimationIndex < Scene->mNumAnimations; AnimationIndex++ )
	{
		auto Sequence = Scene->mAnimations[AnimationIndex];
		if( Sequence )
		{
			Animation NewAnimation;
			NewAnimation.Name = Sequence->mName.C_Str();
			if( Sequence->mTicksPerSecond > 0.0f )
			{
				NewAnimation.Duration = Sequence->mDuration / Sequence->mTicksPerSecond;
			}
			else
			{
				NewAnimation.Duration = Sequence->mDuration;
			}

			for( size_t ChannelIndex = 0; ChannelIndex < Sequence->mNumChannels; ChannelIndex++ )
			{
				auto Channel = Sequence->mChannels[ChannelIndex];
				if( Channel )
				{
					// auto Node = MeshData.BoneToNode[Channel->mNodeName.C_Str()];
					if( MeshData.NodeToBone.find( Channel->mNodeName.C_Str() ) != MeshData.NodeToBone.end() )
					{
						const auto& Bone = MeshData.NodeToBone[Channel->mNodeName.C_Str()];

						for( size_t KeyIndex = 0; KeyIndex < Channel->mNumPositionKeys; KeyIndex++ )
						{
							const auto& ChannelKey = Channel->mPositionKeys[KeyIndex];

							Key Key;
							Key.BoneIndex = Bone.Bone->Index;
							Key.Time = ChannelKey.mTime / Sequence->mTicksPerSecond;
							Key.Value.X = ChannelKey.mValue.x;
							Key.Value.Y = ChannelKey.mValue.y;
							Key.Value.Z = ChannelKey.mValue.z;

							NewAnimation.PositionKeys.emplace_back( Key );
						}

						for( size_t KeyIndex = 0; KeyIndex < Channel->mNumRotationKeys; KeyIndex++ )
						{
							const auto& ChannelKey = Channel->mRotationKeys[KeyIndex];

							Key Key;
							Key.BoneIndex = Bone.Bone->Index;
							Key.Time = ChannelKey.mTime / Sequence->mTicksPerSecond;

							Key.Value.X = ChannelKey.mValue.x;
							Key.Value.Y = ChannelKey.mValue.y;
							Key.Value.Z = ChannelKey.mValue.z;
							Key.Value.W = ChannelKey.mValue.w;

							NewAnimation.RotationKeys.emplace_back( Key );
						}

						for( size_t KeyIndex = 0; KeyIndex < Channel->mNumScalingKeys; KeyIndex++ )
						{
							const auto& ChannelKey = Channel->mScalingKeys[KeyIndex];

							Key Key;
							Key.BoneIndex = Bone.Bone->Index;
							Key.Time = ChannelKey.mTime / Sequence->mTicksPerSecond;
							Key.Value.X = ChannelKey.mValue.x;
							Key.Value.Y = ChannelKey.mValue.y;
							Key.Value.Z = ChannelKey.mValue.z;

							NewAnimation.ScalingKeys.emplace_back( Key );
						}
					}
				}
			}

			std::sort( NewAnimation.PositionKeys.begin(), NewAnimation.PositionKeys.end(), CompareKeys );
			std::sort( NewAnimation.RotationKeys.begin(), NewAnimation.RotationKeys.end(), CompareKeys );
			std::sort( NewAnimation.ScalingKeys.begin(), NewAnimation.ScalingKeys.end(), CompareKeys );

			MeshData.Animations.insert_or_assign( NewAnimation.Name, NewAnimation );
			Log::Event( "Imported animation \"%s\" (%i keys)\n", NewAnimation.Name.c_str(), NewAnimation.PositionKeys.size() + NewAnimation.RotationKeys.size() + NewAnimation.ScalingKeys.size() );
		}
	}

	// Copy the animations to the skeleton.
	MeshData.Skeleton->Animations = MeshData.Animations;
}

void FinishSkeleton( FMeshData& MeshData )
{
	if( !MeshData.Skeleton )
		return;

	Skeleton& Skeleton = *MeshData.Skeleton;

	// Find the root bone.
	for( auto& Bone : Skeleton.Bones )
	{
		if( Skeleton.RootIndex < 0 )
		{
			Skeleton.RootIndex = Bone.Index;
			continue;
		}

		if( Bone.ParentIndex < 0 && Bone.Children.size() > Skeleton.Bones[Skeleton.RootIndex].Children.size() )
		{
			Skeleton.RootIndex = Bone.Index;
		}
	}

	// Apply weights
	for( size_t VertexIndex = 0; VertexIndex < Skeleton.Weights.size(); VertexIndex++ )
	{
		MeshData.Vertices[VertexIndex].Bone[0] = Math::Float( Skeleton.Weights[VertexIndex].Index[0] );
		MeshData.Vertices[VertexIndex].Bone[1] = Math::Float( Skeleton.Weights[VertexIndex].Index[1] );
		MeshData.Vertices[VertexIndex].Bone[2] = Math::Float( Skeleton.Weights[VertexIndex].Index[2] );

		MeshData.Vertices[VertexIndex].Weight[0] = Math::Float( Skeleton.Weights[VertexIndex].Weight[0] );
		MeshData.Vertices[VertexIndex].Weight[1] = Math::Float( Skeleton.Weights[VertexIndex].Weight[1] );
		MeshData.Vertices[VertexIndex].Weight[2] = Math::Float( Skeleton.Weights[VertexIndex].Weight[2] );
	}
}

void MeshBuilder::ASSIMP( FPrimitive& Primitive, AnimationSet& Set, const CFile& File )
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

	if( !Scene )
		return;

	Log::Event( "Meshes found: %i\n", Scene->mNumMeshes );

	if( Scene->mNumMeshes > 0 )
	{
		const aiMatrix4x4 RotationMatrix = GetWorldRotationMatrix();
		// auto Transform = Scene->mRootNode->mTransformation;
		auto Transform = Scene->mRootNode->mTransformation * RotationMatrix;
		FMeshData MeshData;
		MeshData.Skeleton = &Set.Skeleton;
		
		MeshData.InverseTransform = Scene->mRootNode->mTransformation;
		MeshData.InverseTransform.Inverse();
		ConvertMatrix( MeshData.Skeleton->GlobalMatrixInverse, Transform );

		ParseNodes( Transform, Scene, Scene->mRootNode, MeshData );
		UpdateBoneHierarchy( Scene->mRootNode, MeshData );
		ParseAnimations( Transform, Scene, MeshData );
		FinishSkeleton( MeshData );

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
		
		/*FVertex* VertexArray = new FVertex[MeshData.Vertices.size()];
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
		Primitive.HasNormals = true;*/
	}
}
