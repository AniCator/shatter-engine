// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

class CCamera;
class CRenderable;
struct BoundingBox;

namespace Culling
{
	bool Frustum( const CCamera& Camera, const BoundingBox& Bounds );
	void Frustum( const CCamera& Camera, CRenderable* Renderable );
	void Frustum( const CCamera& Camera, const std::vector<CRenderable*>& Renderables );
}
