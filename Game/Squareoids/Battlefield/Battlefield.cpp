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

	for( int Index = 0; Index < 1024; Index++ )
	{
		CSquareoidsUnit* NewUnit = new CSquareoidsUnit();

		const float RandomOffsetX = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * 8192 ) - 4096;
		const float RandomOffsetY = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * 8192 ) - 4096;

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

	std::vector<ISquareoidsUnit*> DeadUnits;

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

		if( SquareoidUnitA != PlayerUnit )
		{
			FSquareoidUnitData& UnitData = SquareoidUnitA->GetUnitData();
			if( UnitData.Health < 0.0f )
			{
				DeadUnits.push_back( SquareoidUnitA );
			}
		}
	}

	// Cleanup
	for( auto Iterator = SquareoidUnits.begin(); Iterator != SquareoidUnits.end(); )
	{
		bool RemovedDeadUnit = false;
		for( auto DeadUnit : DeadUnits )
		{
			if( *Iterator == DeadUnit )
			{
				Iterator = SquareoidUnits.erase( Iterator );
				RemovedDeadUnit = true;
				break;
			}
		}

		if( !RemovedDeadUnit )
		{
			++Iterator;
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

	const float InterpolationFactor = 0.314f;
	const float OneMinusInterp = 1.0f - InterpolationFactor;
	Setup.CameraPosition[0] = ( Setup.CameraPosition[0] * OneMinusInterp ) + ( ( Setup.CameraPosition[0] - DeltaX ) * InterpolationFactor );
	Setup.CameraPosition[1] = ( Setup.CameraPosition[1] * OneMinusInterp ) + ( ( Setup.CameraPosition[1] - DeltaY ) * InterpolationFactor );

	const float Speed = Math::Length( UnitData.Velocity ) * 2.0f - 1.0f;
	Setup.CameraPosition[2] = glm::clamp( Setup.CameraPosition[2] + Speed * 0.1f, 100.0f + UnitData.Health, 600.0f + UnitData.Health );
	Setup.CameraPosition[2] = UnitData.Health * 2.0f + 100.0f;

	Setup.CameraDirection = glm::vec3( 0.0f, 0.0f, -1.0f );

	Renderer.SetCamera( *Camera );
}
