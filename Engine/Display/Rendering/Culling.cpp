// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Culling.h"

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Renderable.h>

namespace Culling
{
	bool SphereCull( const CCamera& Camera, const BoundingBox& Bounds )
	{
		const auto& Frustum = Camera.GetFrustum();

		const auto& Center = ( Bounds.Maximum + Bounds.Minimum ) * 0.5f;
		const auto Minimum = Math::Abs( Bounds.Minimum );
		const auto& Maximum = Math::Abs( Bounds.Maximum );

		const float X = Math::Max( Minimum.X, Maximum.X );
		const float Y = Math::Max( Minimum.Y, Maximum.Y );
		const float Z = Math::Max( Minimum.Z, Maximum.Z );

		const float Radius = Math::VectorMax( X, Y, Z );
		return Frustum.Contains( Center, Radius );
	}

	void SphereCull( const CCamera& Camera, CRenderable* Renderable )
	{
		auto& RenderData = Renderable->GetRenderData();
		RenderData.ShouldRender = false;

		if( RenderData.DisableRender )
			return;

		if( SphereCull( Camera, RenderData.WorldBounds ) )
		{
			RenderData.ShouldRender = true;
		}
	}

	void SphereCull( const CCamera& Camera, const std::vector<CRenderable*>& Renderables )
	{
		for( auto* Renderable : Renderables )
		{
			SphereCull( Camera, Renderable );
		}
	}

	bool Frustum( const CCamera& Camera, const BoundingBox& Bounds )
	{
		return SphereCull( Camera, Bounds );
	}

	void Frustum( const CCamera& Camera, CRenderable* Renderable )
	{
		SphereCull( Camera, Renderable );
	}

	void Frustum( const CCamera& Camera, const std::vector<CRenderable*>& Renderables )
	{
		SphereCull( Camera, Renderables );
	}
}
