// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>

#include <Engine/Display/Rendering/Uniform.h>
#include <Engine/Physics/PhysicalSurface.h>
#include <Engine/Resource/Asset.h>

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
	void Apply( class CRenderable* Renderable ) const;

	/// <summary>
	/// Applies this material's uniforms to the given renderable.
	/// </summary>
	void ApplyUniforms( class CRenderable* Renderable ) const;

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

	void Reload() override;

	Material Material;
	std::string Location;
};

Material LoadMaterial( const std::string& Location );