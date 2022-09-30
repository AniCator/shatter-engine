// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>

#include <Engine/Display/Rendering/Uniform.h>
#include <Engine/Resource/Asset.h>

struct Material
{
	std::string Name;
	std::string Shader;
	std::vector<std::string> Textures;
	std::vector<Uniform> Uniforms;

	bool DoubleSided = false;

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