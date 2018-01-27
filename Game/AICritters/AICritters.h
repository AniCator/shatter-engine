// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Game/Game.h>
#include <Display/Rendering/Renderable.h>

static size_t MaximumCritters = 512;

class CCritter
{
public:
	CCritter();

	~CCritter();

	void Tick();

	void Collision( CCritter* Critter );
	void Collision( glm::vec3 CellDirection, float CellEnergy );

	glm::vec3 PreviousPosition;
	glm::vec3 Position;
	glm::vec2 Direction;
	float Size;
	float Speed;
	float Energy;
	CRenderable* Renderable;

	CCritter* Previous;
	CCritter* Next;
};

struct FCell
{
	int X = 0;
	int Y = 0;
};

static const int WorldSize = 8192;
static const int WorldSizeHalf = ( WorldSize / 2 );
static const int WorldSizeHalfNegated = -WorldSizeHalf;
static const int CellSize = 128;
static const int CellsX = WorldSize / CellSize;
static const int CellsY = WorldSize / CellSize;
class CGrid
{
public:
	CGrid();

	~CGrid();

	void Clear();

	FCell GetCellCoordinates( glm::vec3 Position ) const;

	void UpdatePosition( CCritter* Critter );
	void AccumulateEnergy( CCritter* Critter );
	void AverageEnergy();

	void Insert( CCritter* Critter );
	void Tick();

	CCritter* GetCell( const int X, const int Y );
	float GetCellEnergy( const int X, const int Y ) const;

	int GetWorldSize() const;
	int GetCellSize() const;

private:
	CCritter* Critters[CellsX][CellsY];
	float CellEnergy[CellsX][CellsY];
};

class CAICritters : public IGameLayer
{
public:
	CAICritters();
	virtual ~CAICritters();

	virtual void Initialize() override;
	virtual void Frame() override;
	virtual void Tick() override;
	virtual void Shutdown() override;

private:
	std::vector<CCritter*> Critters;

	CGrid Grid;
};

class CTestLayer : public IGameLayer
{
public:
	CTestLayer();
	virtual ~CTestLayer();

	virtual void Initialize() override;
	virtual void Frame() override;
	virtual void Tick() override;
	virtual void Shutdown() override;

private:
	std::vector<CRenderable*> TestRenderables;
};
