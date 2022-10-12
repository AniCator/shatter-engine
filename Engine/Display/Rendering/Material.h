// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>

#include <Engine/Display/Rendering/Uniform.h>
#include <Engine/Resource/Asset.h>

enum class PhysicalSurface : uint8_t
{
	None,

	Stone,
	Metal,
	Wood,
	Concrete,
	Brick,
	Sand,
	Dirt,
	Gravel,
	Grass,
	Forest,
	Rock,

	User12,
	User13,
	User14,
	User15,
	User16,

	Maximum
};

struct Material
{
	std::string Name;
	std::string Shader;
	std::vector<std::string> Textures;
	std::vector<std::pair<std::string, Uniform>> Uniforms;

	bool DoubleSided = false;
	PhysicalSurface Surface = PhysicalSurface::None;

	/// <summary>
	/// Applies this material's settings to the given renderable.
	/// </summary>
	/// <remarks>Performs asset lookups.</remarks>
	void Apply( class CRenderable* Renderable );

	static void RegisterAssetLoader();
};

class MaterialAsset : public Asset
{
public:
	~MaterialAsset() override = default;

	const std::string& GetType() const override
	{
		static const std::string Type = "material";
		return Type;
	}

	Material Material;
};

Material LoadMaterial( const std::string& Location );