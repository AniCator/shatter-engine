// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

struct DragEntity
{
	class CPointEntity* Entity = nullptr;
	int Plane = -1;
};

struct DragTransform
{
	struct FTransform* Transform = nullptr;
	int Plane = -1;
};

struct DragVector
{
	class Vector3D* Point = nullptr;
	int Plane = -1;
};

// Renders an interactive gizmo for moving point entities, takes a drag persistence struct for tracking which entity is being dragged.
bool PointGizmo( CPointEntity* Entity, DragEntity& DragPersistence );

// Renders an interactive gizmo for moving transforms, takes a drag persistence struct for tracking which entity is being dragged.
bool PointGizmo( FTransform* Transform, DragTransform& DragPersistence );

// Renders an interactive gizmo for moving transforms, takes a drag persistence struct for tracking which entity is being dragged.
bool PointGizmo( Vector3D* Point, DragVector& DragPersistence );
