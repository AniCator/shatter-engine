#include "Battlefield.h"

#include <ctime>

#include <Game/Game.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <glm/gtc/matrix_transform.hpp>

#include <Engine/Profiling/Profiling.h>

#include "../Unit/Unit.h"
#include "../Unit/PlayerUnit.h"

#include <ThirdParty/glm/glm.hpp>

CSquareoidsBattlefield::CSquareoidsBattlefield()
{
	std::srand( static_cast<uint32_t>( std::time( 0 ) ) );

	SpatialRegion = new CSpatialRegion<ISquareoidsUnit>();

	for( int Index = 0; Index < 512; Index++ )
	{
		CSquareoidsUnit* NewUnit = new CSquareoidsUnit();

		const float RandomOffsetX = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * 8192 ) - 4096;
		const float RandomOffsetY = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * 8192 ) - 4096;

		FSquareoidUnitData& UnitData = NewUnit->GetUnitData();
		UnitData.Position[0] = RandomOffsetX;
		UnitData.Position[1] = RandomOffsetY;

		SquareoidUnits.push_back( NewUnit );

		SpatialRegion->Insert( NewUnit, &UnitData.Position[0] );
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

	// UpdateBruteForce();
	UpdateSpatialGrid();

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

	DeadUnits.clear();

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
	Setup.CameraPosition[2] = UnitData.Health * 3.14f + 300.0f;

	Setup.CameraDirection = glm::vec3( 0.0f, 0.0f, -1.0f );

	Camera->Update();
	Renderer.SetCamera( *Camera );

	// Cursor square
	CRenderable* Renderable = new CRenderable();
	Renderable->SetMesh( SquareMesh );

	FRenderDataInstanced& RenderData = Renderable->GetRenderData();

	IInput& Input = CInputLocator::GetService();
	FFixedPosition2D MousePosition = Input.GetMousePosition();

	glm::mat4& ProjectionInverse = glm::inverse( Camera->GetProjectionMatrix() );
	glm::mat4& ViewInverse = glm::inverse( Camera->GetViewMatrix() );
	float NormalizedMouseX = ( 2.0f * MousePosition.X ) / 1920.0f - 1.0f;
	float NormalizedMouseY = 1.0f - ( 2.0f * MousePosition.Y ) / 1080.0f;
	glm::vec4 MousePositionClipSpace = glm::vec4( NormalizedMouseX, NormalizedMouseY, -1.0f, 1.0f );
	glm::vec4 MousePositionEyeSpace = ProjectionInverse * MousePositionClipSpace;

	// Un-project Z and W
	MousePositionEyeSpace[2] = -1.0f;
	MousePositionEyeSpace[3] = 1.0f;

	glm::vec3 MousePositionWorldSpace = ViewInverse * MousePositionEyeSpace;
	glm::vec3 MouseDirection = glm::normalize( Camera->GetCameraSetup().CameraPosition - MousePositionWorldSpace );

	bool RayCast = false;
	glm::vec3 StartPosition = Camera->GetCameraSetup().CameraPosition;
	float CastDelta = -0.1f;

	glm::vec3 RayCastResult = StartPosition;

	while( !RayCast )
	{
		StartPosition += MouseDirection * CastDelta;
		const float Delta = 100.0f - StartPosition[2];

		if( fabs( Delta ) < 0.5f )
		{
			RayCastResult = StartPosition;
			RayCast = true;
		}
	}

	RenderData.Color = glm::vec4( 1.0f, 0.5f, 0.0f, 1.0f );
	RenderData.Position = RayCastResult;
	RenderData.Position[2] = 100.0f;
	RenderData.Size = glm::vec3( 10.0f, 10.0f, 10.0f );

	Renderer.QueueDynamicRenderable( Renderable );

	// Visualize in profiler
	CProfileVisualisation& Profiler = CProfileVisualisation::GetInstance();

	char PositionXString[32];
	sprintf_s( PositionXString, "%f", RenderData.Position[0] );
	char PositionYString[32];
	sprintf_s( PositionYString, "%f", RenderData.Position[1] );
	char PositionZString[32];
	sprintf_s( PositionZString, "%f", RenderData.Position[2] );

	Profiler.AddDebugMessage( "3DMousePositionX", PositionXString );
	Profiler.AddDebugMessage( "3DMousePositionY", PositionYString );
	Profiler.AddDebugMessage( "3DMousePositionZ", PositionZString );
}

void CSquareoidsBattlefield::UpdateBruteForce()
{
	int64_t CollisionChecks = 0;

	// Tick units and do brute force interaction checks
	for( auto SquareoidUnitA : SquareoidUnits )
	{
		SquareoidUnitA->Tick();

		for( auto SquareoidUnitB : SquareoidUnits )
		{
			if( SquareoidUnitA != SquareoidUnitB )
			{
				SquareoidUnitA->Interaction( SquareoidUnitB );
				CollisionChecks++;
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

	CProfileVisualisation::GetInstance().AddCounterEntry( FProfileTimeEntry( "Collision Checks", CollisionChecks ) );
}

void CSquareoidsBattlefield::UpdateSpatialGrid()
{
	int64_t CollisionChecks = 0;

	// Tick units, do spatial lookup and collision checks
	for( auto SquareoidUnitA : SquareoidUnits )
	{
		FSquareoidUnitData& UnitData = SquareoidUnitA->GetUnitData();

		SquareoidUnitA->Tick();

		if( SquareoidUnitA != PlayerUnit )
		{
			if( UnitData.Health < 0.0f )
			{
				DeadUnits.push_back( SquareoidUnitA );
			}
		}

		std::vector<ISquareoidsUnit*> Colliders;

		for( int Corner = 0; Corner < 4; Corner++ )
		{
			glm::vec3 CornerPosition = UnitData.Position;

			const float SizeSplit = UnitData.Size[0] * 0.5f;

			if( Corner == 0 )
			{
				CornerPosition[0] -= SizeSplit;
				CornerPosition[1] -= SizeSplit;
			}
			else if( Corner == 1 )
			{
				CornerPosition[0] += SizeSplit;
				CornerPosition[1] -= SizeSplit;
			}
			else if( Corner == 2 )
			{
				CornerPosition[0] += SizeSplit;
				CornerPosition[1] += SizeSplit;
			}
			else if( Corner == 3 )
			{
				CornerPosition[0] -= SizeSplit;
				CornerPosition[1] += SizeSplit;
			}

			std::set<ISquareoidsUnit*>& LocalSet = SpatialRegion->GetSetForPosition( &CornerPosition[0] );

			for( auto SquareoidUnitB : LocalSet )
			{
				bool Processed = false;
				for( auto Collider : Colliders )
				{
					if( SquareoidUnitB == Collider )
					{
						Processed = true;
						break;
					}
				}

				if( Processed )
					continue;

				SquareoidUnitA->Interaction( SquareoidUnitB );
				CollisionChecks++;

				Colliders.push_back( SquareoidUnitB );
			}
		}
	}

	CProfileVisualisation::GetInstance().AddCounterEntry( FProfileTimeEntry( "Collision Checks", CollisionChecks ) );

	SpatialRegion->Clear();

	for( auto SquareoidUnitA : SquareoidUnits )
	{
		if( SquareoidUnitA )
		{
			FSquareoidUnitData& UnitData = SquareoidUnitA->GetUnitData();
			SpatialRegion->Insert( SquareoidUnitA, &UnitData.Position[0] );
		}
	}

	/*CRenderer& Renderer = CWindow::GetInstance().GetRenderer();
	CMesh* SquareMesh = Renderer.FindMesh( "square" );

	for( size_t X = 0; X < Cells; X++ )
	{
		for( size_t Y = 0; Y < Cells; Y++ )
		{
			TSpatialCell Cell;
			Cell.X = X;
			Cell.Y = Y;

			std::set<ISquareoidsUnit*>& LocalSet = SpatialRegion->GetSetForCell( Cell );
			const float Heat = glm::clamp( static_cast<float>( LocalSet.size() ) / 20.0f, 0.0f, 1.0f );

			CRenderable* Renderable = new CRenderable();
			Renderable->SetMesh( SquareMesh );

			FRenderDataInstanced& RenderData = Renderable->GetRenderData();
			RenderData.Position = glm::vec3(
				X * CellSize + CellSize / 2 - WorldSize,
				Y * CellSize + CellSize / 2 - WorldSize,
				-0.1f );
			RenderData.Size = glm::vec3(
				CellSize / 2,
				CellSize / 2,
				1.0f );
			RenderData.Color = glm::vec4(
				0.5f, Heat, 0.0f,
				1.0f );

			Renderer.QueueDynamicRenderable( Renderable );
		}
	}*/
}
