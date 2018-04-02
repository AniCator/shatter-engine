#include "Battlefield.h"

#include <ctime>

#include <Game/Game.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Rendering/Camera.h>

#include "../Unit/Unit.h"
#include "../Unit/PlayerUnit.h"

CSquareoidsBattlefield::CSquareoidsBattlefield()
{
	std::srand( static_cast<uint32_t>( std::time( 0 ) ) );

	for( int Index = 0; Index < 5; Index++ )
	{
		CSquareoidsUnit* NewUnit = new CSquareoidsUnit();

		const float RandomOffsetX = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * 512 ) - 256;
		const float RandomOffsetY = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * 512 ) - 256;

		FSquareoidUnitData& UnitData = NewUnit->GetUnitData();
		UnitData.Position[0] = RandomOffsetX;
		UnitData.Position[1] = RandomOffsetY;

		SquareoidUnits.push_back( NewUnit );
	}

	PlayerUnit = new CSquareoidsPlayerUnit();
	SquareoidUnits.push_back( PlayerUnit );

	Camera = new CCamera();
}

CSquareoidsBattlefield::~CSquareoidsBattlefield()
{
	for( auto SquareoidUnit : SquareoidUnits )
	{
		delete SquareoidUnit;
		SquareoidUnit = nullptr;
	}

	SquareoidUnits.clear();

	delete Camera;
}

void CSquareoidsBattlefield::Update()
{
	CRenderer& Renderer = CWindow::GetInstance().GetRenderer();
	CMesh* SquareMesh = Renderer.FindMesh( "square" );

	// Tick units and do brute force interaction checks
	for( auto SquareoidUnitA : SquareoidUnits )
	{
		SquareoidUnitA->Tick();

		for( auto SquareoidUnitB : SquareoidUnits )
		{
			if( SquareoidUnitA != SquareoidUnitB )
			{
				SquareoidUnitA->Interaction( SquareoidUnitB );
			}
		}
	}

	// Queue units for rendering
	for( auto SquareoidUnit : SquareoidUnits )
	{
		FSquareoidUnitData& UnitData = SquareoidUnit->GetUnitData();

		CRenderable* Renderable = new CRenderable();
		Renderable->SetMesh( SquareMesh );

		FRenderDataInstanced& RenderData = Renderable->GetRenderData();

		RenderData.Color = UnitData.Color;
		RenderData.Position = UnitData.Position;
		RenderData.Size = UnitData.Size;

		Renderer.QueueDynamicRenderable( Renderable );
	}

	// Configure the camera setup
	FCameraSetup& Setup = Camera->GetCameraSetup();
	FSquareoidUnitData& UnitData = PlayerUnit->GetUnitData();

	const float DeltaX = Setup.CameraPosition[0] + UnitData.Position[0];
	const float DeltaY = Setup.CameraPosition[1] - UnitData.Position[1];

	const float InterpolationFactor = 0.1f;
	const float OneMinusInterp = 1.0f - InterpolationFactor;
	Setup.CameraPosition[0] = ( Setup.CameraPosition[0] * OneMinusInterp ) + ( -DeltaX * InterpolationFactor );
	Setup.CameraPosition[1] = ( Setup.CameraPosition[1] * OneMinusInterp ) + ( -DeltaY * InterpolationFactor );

	Setup.CameraPosition[2] = 600.0f;

	Setup.CameraDirection = glm::vec3( 0.0f, 0.0f, -1.0f );

	Renderer.SetCamera( *Camera );
}
