// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Culling.h"

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Renderable.h>

namespace Culling
{
	bool SphereCull( const CCamera& Camera, const BoundingBox& Bounds )
	{
		const auto& Frustum = Camera.GetFrustum();
		// TODO: Don't call length here every time.
		return Frustum.Contains( Bounds.Center(), Bounds.Size().Length() * 0.5f );
	}

	bool BoxCull( const CCamera& Camera, const BoundingBox& Bounds )
	{
		const auto& Frustum = Camera.GetFrustum();
		return Frustum.Contains( Bounds );
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

	void BoxCull( const CCamera& Camera, CRenderable* Renderable )
	{
		auto& RenderData = Renderable->GetRenderData();
		RenderData.ShouldRender = false;

		if( RenderData.DisableRender )
			return;

		if( BoxCull( Camera, RenderData.WorldBounds ) )
		{
			RenderData.ShouldRender = true;
		}
	}

	void CombinedCull( const CCamera& Camera, CRenderable* Renderable )
	{
		auto& RenderData = Renderable->GetRenderData();
		RenderData.ShouldRender = false;

		if( RenderData.DisableRender )
			return;

		if( SphereCull( Camera, RenderData.WorldBounds ) && BoxCull( Camera, RenderData.WorldBounds ) )
		{
			RenderData.ShouldRender = true;
		}
	}

	bool Frustum( const CCamera& Camera, const BoundingBox& Bounds )
	{
		return SphereCull( Camera, Bounds ) && BoxCull( Camera, Bounds );
	}

	void Frustum( const CCamera& Camera, CRenderable* Renderable )
	{
		CombinedCull( Camera, Renderable );
	}

	void Frustum( const CCamera& Camera, const std::vector<CRenderable*>& Renderables )
	{
		for( auto* Renderable : Renderables )
		{
			CombinedCull( Camera, Renderable );
		}
	}
}
