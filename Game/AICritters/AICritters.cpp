// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "AICritters.h"

#include <Profiling/Profiling.h>
#include <Profiling/Logging.h>

#include <Display/Window.h>
#include <Display/Rendering/Renderable.h>

#include <Configuration/Configuration.h>

#include <Utility/Math.h>
#include <Utility/Math/SIMDVector.h>

#include <cstdlib>

#undef GetCurrentTime

CAICritters::CAICritters()
{
	CConfiguration& Configuration = CConfiguration::GetInstance();
	Configuration.AppendFile( "AICritters.ini" );
	Configuration.Reload();
}

CAICritters::~CAICritters()
{

}

void CAICritters::Initialize()
{
	std::srand( 0 );

	MaximumCritters = CConfiguration::GetInstance().GetInteger( "critters" );

	Critters.reserve( MaximumCritters );

	for( int Index = 0; Index < MaximumCritters; Index++ )
	{
		CCritter* Critter = new CCritter();

		const float SpawnZoneSize = CConfiguration::GetInstance().GetFloat( "spawnzone" );
		const float SpawnZoneSizeHalf = SpawnZoneSize * 0.5f;

		const float RandomOffsetX = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * SpawnZoneSize ) - SpawnZoneSizeHalf;
		const float RandomOffsetY = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * SpawnZoneSize ) - SpawnZoneSizeHalf;
		Critter->Position[0] = RandomOffsetX;
		Critter->Position[1] = RandomOffsetY;

		const float AmbientNoise = ( ( ( static_cast<float>( std::rand() ) / RAND_MAX ) ) * 2.0f - 1.0f ) * 0.0001f;
		Critter->Energy += AmbientNoise;

		Critter->Renderable = new CRenderable();
		Critters.push_back( Critter );
	}

	// Clear the grid
	Grid.Clear();

	// Reconstruct the grid
	for( int Index = 0; Index < Critters.size(); Index++ )
	{
		Grid.Insert( Critters[Index] );
	}
}

static int64_t CollisionChecks = 0;
void CAICritters::Frame()
{
	int64_t CrittersSize = int64_t( Critters.size() );
	CProfileVisualisation::GetInstance().AddCounterEntry( FProfileTimeEntry( "Critters", CrittersSize ) );
	CProfileVisualisation::GetInstance().AddCounterEntry( FProfileTimeEntry( "Collision Checks", CollisionChecks ) );
}

void CAICritters::Tick()
{
	const bool BruteForce = CConfiguration::GetInstance().GetInteger( "bruteforce" ) > 0;

	const float DeltaTime = static_cast<float>( GameLayersInstance->GetDeltaTime() );

	CollisionChecks = 0;

	CRenderer& Renderer = CWindow::GetInstance().GetRenderer();
	CMesh* SquareMesh = Renderer.FindMesh( "square" );
	CMesh* LineSquareMesh = Renderer.FindMesh( "linesquare" );

	if( !BruteForce )
	{
		Grid.Tick();
		for( int i = 0; i < MaximumCritters; i++ )
		{
			Grid.AccumulateEnergy( Critters[i] );
		}

		Grid.AverageEnergy();

		for( int x = 0; x < CellsX; x++ )
		{
			for( int y = 0; y < CellsY; y++ )
			{
				CCritter* CritterCellCenter = Grid.GetCell( x, y );
				if( CritterCellCenter == nullptr )
					continue;

				const float CritterCellEnergyCenter = Grid.GetCellEnergy( x, y );
				float CritterCellEnergyN = 0.0f;
				float CritterCellEnergyE = 0.0f;
				float CritterCellEnergyS = 0.0f;
				float CritterCellEnergyW = 0.0f;

				float CritterCellEnergyNE = 0.0f;
				float CritterCellEnergySE = 0.0f;
				float CritterCellEnergySW = 0.0f;
				float CritterCellEnergyNW = 0.0f;

				static const glm::vec3 CritterCellDirectionN = glm::vec3( 0.0f, 1.0f, 0.0f );
				static const glm::vec3 CritterCellDirectionE = glm::vec3( 1.0f, 0.0f, 0.0f );
				static const glm::vec3 CritterCellDirectionS = glm::vec3( 0.0f, -1.0f, 0.0f );
				static const glm::vec3 CritterCellDirectionW = glm::vec3( -1.0f, 0.0f, 0.0f );

				const int CellNY = y + 1;
				if( CellNY > -1 && CellNY < CellsY )
				{
					CritterCellEnergyN = Grid.GetCellEnergy( x, CellNY );
				}

				const int CellEX = x + 1;
				if( CellEX > -1 && CellEX < CellsX )
				{
					CritterCellEnergyE = Grid.GetCellEnergy( CellEX, y );
				}

				const int CellSY = y - 1;
				if( CellSY > -1 && CellSY < CellsY )
				{
					CritterCellEnergyS = Grid.GetCellEnergy( x, CellSY );
				}

				const int CellWX = x - 1;
				if( CellWX > -1 && CellWX < CellsX )
				{
					CritterCellEnergyW = Grid.GetCellEnergy( CellWX, y );
				}

				if( CellNY > -1 && CellNY < CellsY )
				{
					if( CellEX > -1 && CellEX < CellsX )
					{
						CritterCellEnergyNE = Grid.GetCellEnergy( CellEX, CellNY );
					}
				}

				if( CellSY > -1 && CellSY < CellsY )
				{
					if( CellEX > -1 && CellEX < CellsX )
					{
						CritterCellEnergySE = Grid.GetCellEnergy( CellEX, CellSY );
					}
				}

				if( CellSY > -1 && CellSY < CellsY )
				{
					if( CellWX > -1 && CellWX < CellsX )
					{
						CritterCellEnergySW = Grid.GetCellEnergy( CellWX, CellSY );
					}
				}

				if( CellNY > -1 && CellNY < CellsY )
				{
					if( CellWX > -1 && CellWX < CellsX )
					{
						CritterCellEnergyNW = Grid.GetCellEnergy( CellWX, CellNY );
					}
				}

				CCritter* CritterA = CritterCellCenter;

				while( CritterA )
				{
					CCritter* CritterB = CritterA->Next;

					while( CritterB )
					{
						CritterA->Collision( CritterB );
						CritterB = CritterB->Next;

						CollisionChecks++;
					}

					// Check against neighboring cells
					CritterA->Collision( CritterCellDirectionN, CritterCellEnergyN );
					CritterA->Collision( CritterCellDirectionE, CritterCellEnergyE );
					CritterA->Collision( CritterCellDirectionS, CritterCellEnergyS );
					CritterA->Collision( CritterCellDirectionW, CritterCellEnergyW );

					CritterA->Collision( ( CritterCellDirectionN + CritterCellDirectionE ) * 0.5f, CritterCellEnergyNE );
					CritterA->Collision( ( CritterCellDirectionS + CritterCellDirectionE ) * 0.5f, CritterCellEnergySE );
					CritterA->Collision( ( CritterCellDirectionS + CritterCellDirectionW ) * 0.5f, CritterCellEnergySW );
					CritterA->Collision( ( CritterCellDirectionN + CritterCellDirectionW ) * 0.5f, CritterCellEnergyNW );

					CritterA->Tick();

					if( CritterA->Position[0] < WorldSizeHalfNegated || CritterA->Position[0] > -WorldSizeHalfNegated || CritterA->Position[1] < WorldSizeHalfNegated || CritterA->Position[1] > -WorldSizeHalfNegated )
					{
						CritterA->Direction *= -0.0f;
					}

					CritterA->Position = glm::clamp( CritterA->Position, static_cast<float>( WorldSizeHalfNegated ), static_cast<float>( -WorldSizeHalfNegated ) );

					const bool ValidRenderable = CritterA->Renderable->GetMesh() != nullptr;
					FRenderDataInstanced& RenderData = CritterA->Renderable->GetRenderData();

					if( !ValidRenderable )
					{
						CritterA->Renderable->SetMesh( SquareMesh );
						RenderData.Size = glm::vec3( CritterA->Size, CritterA->Size, CritterA->Size );
						RenderData.Color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
					}
					else
					{
						RenderData.Position = CritterA->Position;
						const float ActualSize = CritterA->Size;
						RenderData.Size = glm::vec3( ActualSize, ActualSize, ActualSize );
						RenderData.Color = glm::vec4( 1.0f, 0.0f, CritterA->Energy, 1.0f );
					}

					Renderer.QueueRenderable( CritterA->Renderable );

					CCritter* CritterAPrevious = CritterA;
					CritterA = CritterA->Next;
					Grid.UpdatePosition( CritterAPrevious );
				}
			}
		}

		if( CConfiguration::GetInstance().GetInteger( "showgrid" ) > 0 )
		{
			for( int x = 0; x < CellsX; x++ )
			{
				for( int y = 0; y < CellsY; y++ )
				{
					const float CellEnergy = Grid.GetCellEnergy( x, y );
					if( CellEnergy > 0.01f )
					{
						CRenderable* Renderable = new CRenderable();
						Renderable->SetMesh( SquareMesh );

						FRenderDataInstanced& RenderData = Renderable->GetRenderData();
						RenderData.Position = glm::vec3( x * CellSize + WorldSizeHalfNegated, y * CellSize + WorldSizeHalfNegated, 1.0f );
						RenderData.Size = glm::vec3( CellSize, CellSize, CellSize );
						// RenderData.Color = glm::vec4( 0.0f, 0.1f, 0.33f, 1.0f );

						RenderData.Color = glm::vec4( 0.0f, 0.0f, CellEnergy * 0.05f, 1.0f );

						Renderer.QueueDynamicRenderable( Renderable );
					}
				}
			}
		}
	}
	else
	{
		for( int IndexA = 0; IndexA < MaximumCritters; IndexA++ )
		{
			CCritter* CritterA = Critters[IndexA];

			for( int IndexB = 0; IndexB < MaximumCritters; IndexB++ )
			{
				CCritter* CritterB = Critters[IndexB];

				// Skip self
				if( CritterA == CritterB )
					continue;

				CollisionChecks++;

				CritterA->Collision( CritterB );
			}

			CritterA->Tick();

			const bool ValidRenderable = CritterA->Renderable->GetMesh() != nullptr;
			FRenderDataInstanced& RenderData = CritterA->Renderable->GetRenderData();

			if( !ValidRenderable )
			{
				CritterA->Renderable->SetMesh( SquareMesh );
				RenderData.Size = glm::vec3( CritterA->Size, CritterA->Size, CritterA->Size );
				RenderData.Color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
			}
			else
			{
				RenderData.Position = CritterA->Position;
				const float ActualSize = CritterA->Size;
				RenderData.Size = glm::vec3( ActualSize, ActualSize, ActualSize );
				RenderData.Color = glm::vec4( 1.0f, 0.0f, CritterA->Energy, 1.0f );
			}

			Renderer.QueueRenderable( CritterA->Renderable );
		}
	}
}

void CAICritters::Shutdown()
{
	for( int Index = 0; Index < MaximumCritters; Index++ )
	{
		delete Critters[Index];
	}

	Critters.clear();
}

CTestLayer::CTestLayer()
{
	
}

CTestLayer::~CTestLayer()
{

}

void CTestLayer::Initialize()
{
	for( int i = 0; i < 1000; i++ )
	{
		TestRenderables.push_back( new CRenderable() );
	}
}

void CTestLayer::Frame()
{
	
}

void CTestLayer::Tick()
{
	CRenderer& Renderer = CWindow::GetInstance().GetRenderer();

	const float TimeStamp = static_cast<float>( GameLayersInstance->GetCurrentTime() );
	const float Frequency = TimeStamp;
	const float Amplitude = 1.0f;
	const float SinTime = sin( Frequency * 8.0f ) * Amplitude;
	const float CosTime = cos( Frequency * 8.0f ) * Amplitude;

	int Index = 0;
	for( auto TestRenderable : TestRenderables )
	{
		float Offset = static_cast<float>( Index );
		const float Sin2Time = ( sin( Frequency + Offset ) * Offset * Amplitude );// +sin( ( Frequency + Offset ) * 10 ) * 10;
		const float Cos2Time = cos( Frequency + Offset ) * Offset * Amplitude;

		const bool ValidRenderable = TestRenderable->GetMesh() != nullptr;
		FRenderDataInstanced& RenderData = TestRenderable->GetRenderData();

		if( !ValidRenderable )
		{
			TestRenderable->SetMesh( Renderer.FindMesh( "square" ) );
			RenderData.Size = glm::vec3( 5.0f, 5.0f, 5.0f );
			RenderData.Color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
		}
		else
		{
			RenderData.Position = glm::vec3( Cos2Time, Sin2Time, 100.0f );

			const float Distance = glm::clamp( 1.0f - glm::distance( RenderData.Position, glm::vec3( 0.0f, 0.0f, 0.0f ) ) * 0.002f, 0.25f, 1.0f );

			RenderData.Color = glm::vec4( Distance, 0.0f, 0.0f, 1.0f );
		}

		Renderer.QueueRenderable( TestRenderable );

		Index++;
	}
}

void CTestLayer::Shutdown()
{
	TestRenderables.clear();
}

CCritter::CCritter()
{
	Position[0] = 0.0f;
	Position[1] = 0.0f;
	Position[2] = 0.0f;

	Direction[0] = 0.0f;
	Direction[1] = 0.0f;

	Size = CConfiguration::GetInstance().GetFloat( "crittersize" );
	Speed = CConfiguration::GetInstance().GetFloat( "critterspeed" );
	Energy = 1.0f;

	Renderable = nullptr;

	Previous = nullptr;
	Next = nullptr;
}

CCritter::~CCritter()
{
	delete Renderable;
	Renderable = nullptr;
}

void CCritter::Tick()
{
	const float TimeScale = static_cast<float>( GameLayersInstance->GetTimeScale() );
	const float Inertia = 1.0f / Energy;

	PreviousPosition = Position;

	glm::vec2 DirectionNormalized = Direction;
	Math::Normalize( DirectionNormalized );
	Position[0] += DirectionNormalized[0] * TimeScale * Speed * Inertia;
	Position[1] += DirectionNormalized[1] * TimeScale * Speed * Inertia;

	Size = Energy;

	// Direction[0] = 0.0f;
	// Direction[1] = 0.0f;
}

void CCritter::Collision( glm::vec3 CellDirection, float CellEnergy )
{
	const glm::vec2 Direction2D = glm::vec2( CellDirection[0], CellDirection[1] );

	glm::vec2 Direction2DCenter = glm::vec2( -Position[0], -Position[1] );
	glm::vec2 DirectionCenterNormalized = Direction2D;
	Math::Normalize( DirectionCenterNormalized );

	Direction += DirectionCenterNormalized * CellEnergy;
}

void CCritter::Collision( CCritter* Critter )
{
	if( Critter && Critter != this )
	{
		glm::vec2 Impulse = glm::vec2( 0.0f, 0.0f );

		glm::vec3 Direction3D = Critter->Position - Position;
		glm::vec2 Direction2D = glm::vec2( Direction3D[0], Direction3D[1] );
		const float Length = Math::Length( Direction2D );

		// const float MaximumRange = 1.0f / ( Critter->Energy * 1024.0f );
		// const float Falloff = glm::clamp( 1.0f / ( Length * Length * MaximumRange ), 0.0f, 1.0f );

		glm::vec2 DirectionNormalized = Direction2D;
		Math::Normalize( DirectionNormalized, Length );

		const float CombinedSize = Size + Critter->Size;
		if( Length < CombinedSize )
		{
			// Colliding
			if( Energy > 0.002f )
			{
				Energy -= 0.001f;
				Critter->Energy += 0.001f;
			}
		}
		else
		{
			// Scale direction by target critter energy to prioritize high energy targets
			Impulse += DirectionNormalized * Critter->Energy;// *Falloff;
		}

		Direction += Impulse;
	}
}

CGrid::CGrid()
{
	Clear();
}

CGrid::~CGrid()
{

}

void CGrid::Clear()
{
	for( int x = 0; x < CellsX; x++ )
	{
		for( int y = 0; y < CellsY; y++ )
		{
			Critters[x][y] = nullptr;
			CellEnergy[x][y] = 0.0f;
		}
	}
}

FCell CGrid::GetCellCoordinates( glm::vec3 Position ) const
{
	FCell Cell;
	Cell.X = glm::clamp( static_cast<int>( ( Position[0] + WorldSizeHalf ) / CellSize ), 0, CellsX - 1 );
	Cell.Y = glm::clamp( static_cast<int>( ( Position[1] + WorldSizeHalf ) / CellSize ), 0, CellsY - 1 );

	if( Cell.X < 0 || Cell.X > CellsX || Cell.Y < 0 || Cell.Y > CellsY )
	{
		Log::Event( Log::Warning, "Inserted critter is out of bounds!\n" );
	}

	return Cell;
}

void CGrid::UpdatePosition( CCritter* Critter )
{
	FCell PreviousCell = GetCellCoordinates( Critter->PreviousPosition );
	FCell CurrentCell = GetCellCoordinates( Critter->Position );

	if( PreviousCell.X != CurrentCell.X || PreviousCell.Y != CurrentCell.Y )
	{
		// If I have a previous neighbor then I want his next to be my next because I'm jumping out
		if( Critter->Previous )
		{
			Critter->Previous->Next = Critter->Next;
		}

		// My next neighbor should be made aware of the fact that my previous neighbor is now his neighbor
		if( Critter->Next )
		{
			Critter->Next->Previous = Critter->Previous;
		}

		// If we were top dog before we should make sure our next neighbor in line is put in charge
		if( Critters[PreviousCell.X][PreviousCell.Y] == Critter )
		{
			Critters[PreviousCell.X][PreviousCell.Y] = Critter->Next;
		}

		// Re-insert ourselves
		Insert( Critter );
	}
}

void CGrid::AccumulateEnergy( CCritter* Critter )
{
	FCell CurrentCell = GetCellCoordinates( Critter->Position );
	CellEnergy[CurrentCell.X][CurrentCell.Y] += Critter->Energy;
}

void CGrid::AverageEnergy()
{
	static const float DistributionPeak = 0.75f;
	static const float DistributionNeighbors = 1.0f - DistributionPeak;
	/*for( int x = 0; x < CellsX; x++ )
	{
	for( int y = 0; y < CellsY; y++ )
	{
	float Energy = CellEnergy[x][y] * DistributionPeak;
	const int CellNY = y + 1;
	if( CellNY > -1 && CellNY < CellsY )
	{
	Energy += GetCellEnergy( x, CellNY ) * DistributionNeighbors;
	}

	const int CellEX = x + 1;
	if( CellEX > -1 && CellEX < CellsX )
	{
	Energy += GetCellEnergy( CellEX, y ) * DistributionNeighbors;
	}

	const int CellSY = y - 1;
	if( CellSY > -1 && CellSY < CellsY )
	{
	Energy += GetCellEnergy( x, CellSY ) * DistributionNeighbors;
	}

	const int CellWX = x - 1;
	if( CellWX > -1 && CellWX < CellsX )
	{
	Energy += GetCellEnergy( CellWX, y ) * DistributionNeighbors;
	}
	}
	}*/

	for( int x = 0; x < CellsX; x++ )
	{
		for( int y = 0; y < CellsY; y++ )
		{
			float Energy = CellEnergy[x][y];
			const int CellNY = y + 1;
			if( CellNY > -1 && CellNY < CellsY )
			{
				Energy += GetCellEnergy( x, CellNY );
			}

			const int CellEX = x + 1;
			if( CellEX > -1 && CellEX < CellsX )
			{
				Energy += GetCellEnergy( CellEX, y );
			}

			const int CellSY = y - 1;
			if( CellSY > -1 && CellSY < CellsY )
			{
				Energy += GetCellEnergy( x, CellSY );
			}

			const int CellWX = x - 1;
			if( CellWX > -1 && CellWX < CellsX )
			{
				Energy += GetCellEnergy( CellWX, y );
			}

			Energy /= 5;
		}
	}
}

void CGrid::Insert( CCritter* Critter )
{
	FCell NewCell = GetCellCoordinates( Critter->Position );

	// Forget about our left-hand neighbor
	Critter->Previous = nullptr;

	// Current top dog should become our right-hand neighbor
	Critter->Next = Critters[NewCell.X][NewCell.Y];

	// Put me at the top of the list
	Critters[NewCell.X][NewCell.Y] = Critter;

	// Make me the new neighbor
	if( Critter->Next != nullptr )
	{
		Critter->Next->Previous = Critter;
	}
}

void CGrid::Tick()
{
	for( int x = 0; x < CellsX; x++ )
	{
		for( int y = 0; y < CellsY; y++ )
		{
			CellEnergy[x][y] = 0.0f;
		}
	}
}

CCritter* CGrid::GetCell( const int X, const int Y )
{
	return Critters[X][Y];
}

float CGrid::GetCellEnergy( const int X, const int Y ) const
{
	return CellEnergy[X][Y];
}

int CGrid::GetWorldSize() const
{
	return WorldSize;
}

int CGrid::GetCellSize() const
{
	return CellSize;
}
