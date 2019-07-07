// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "UserInterface.h"

#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Camera.h>

#include <ThirdParty/imgui-1.70/imgui.h>
#include <Engine/Display/imgui_impl_opengl3.h>

namespace UI
{
	bool Ready = false;
	ImDrawList* DrawList = nullptr;
	ImDrawData DrawData;
	
	CCamera Camera;
	float Width;
	float Height;

	ImU32 GetColor( Color::Type Color )
	{
		switch( Color )
		{
			case Color::Red:
				return IM_COL32( 255, 0, 0, 255 );
			case Color::Green:
				return IM_COL32( 0, 255, 0, 255 );
			case Color::Blue:
				return IM_COL32( 0, 0, 255, 255 );
			case Color::White:
				return IM_COL32( 255, 255, 255, 255 );
			default:
				break;
		}

		return IM_COL32( 0, 0, 0, 255 );
	}

	Vector3D ScreenPositionToWorld( const Vector2D& ScreenPosition )
	{
		const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();
		const glm::mat4& ViewMatrix = Camera.GetViewMatrix();

		glm::mat4& ProjectionInverse = glm::inverse( ProjectionMatrix );
		glm::mat4& ViewInverse = glm::inverse( ViewMatrix );
		const float NormalizedScreenPositionX = ( 2.0f * ScreenPosition[0] ) / Width - 1.0f;
		const float NormalizedScreenPositionY = 1.0f - ( 2.0f * ScreenPosition[1] ) / Height;
		glm::vec4 ScreenPositionClipSpace = glm::vec4( NormalizedScreenPositionX, NormalizedScreenPositionY, -1.0f, 1.0f );
		glm::vec4 ScreenPositionViewSpace = ProjectionInverse * ScreenPositionClipSpace;

		ScreenPositionViewSpace[2] = -1.0f;
		ScreenPositionViewSpace[3] = 1.0f;

		glm::vec3 WorldSpacePosition = ViewInverse * ScreenPositionViewSpace;

		return Vector3D( WorldSpacePosition[0], WorldSpacePosition[1], WorldSpacePosition[2] );
	}

	Vector2D WorldToScreenPosition( const Vector3D& WorldPosition, bool* IsInFront )
	{
		const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();
		const glm::mat4& ViewMatrix = Camera.GetViewMatrix();

		glm::vec4 WorldPositionHomogenoeus = glm::vec4( Math::ToGLM( WorldPosition ), 1.0f );
		glm::vec4 ClipSpacePosition = ProjectionMatrix * ViewMatrix * WorldPositionHomogenoeus;
		glm::vec3 NormalizedPosition = glm::vec3( ClipSpacePosition.x, -ClipSpacePosition.y, ClipSpacePosition.z ) / ClipSpacePosition.w;
		
		bool Front = ClipSpacePosition.w > 0.0f;
		const float Bias = Front ? 1.0f : -1.0f;

		Vector2D ScreenPosition;
		ScreenPosition.X = ( NormalizedPosition.x * 0.5f + 0.5f ) * Width;
		ScreenPosition.Y = ( NormalizedPosition.y * 0.5f + 0.5f ) * Height;

		ScreenPosition *= Bias;

		if( ScreenPosition.X < 0.0f )
		{
			ScreenPosition.X = 0.0f;
			Front = false;
		}
		else if( ScreenPosition.X > Width )
		{
			ScreenPosition.X = Width;
			Front = false;
		}

		if( ScreenPosition.Y < 0.0f )
		{
			ScreenPosition.Y = 0.0f;
			Front = false;
		}
		else if( ScreenPosition.Y > Height )
		{
			ScreenPosition.Y = Height;
			Front = false;
		}

		if( IsInFront )
		{
			*IsInFront = Front;
		}

		return ScreenPosition;
	}

	void AddLine( const Vector2D& Start, const Vector2D& End, Color::Type Color )
	{
		if( DrawList )
		{
			DrawList->AddLine( ImVec2( Start.X, Start.Y ), ImVec2( End.X, End.Y ), GetColor( Color ), 1.0f );
		}
	}

	void AddLine( const Vector3D& Start, const Vector3D& End, Color::Type Color )
	{
		bool StartIsInFront = false;
		auto& ScreenStart = WorldToScreenPosition( Start, &StartIsInFront );

		bool EndIsInFront = false;
		auto& ScreenEnd = WorldToScreenPosition( End, &EndIsInFront );

		if( StartIsInFront || EndIsInFront )
		{
			AddLine( WorldToScreenPosition( Start ), WorldToScreenPosition( End ), Color );
		}
	}

	void AddCircle( const Vector2D& Position, float Radius, Color::Type Color )
	{
		if( DrawList )
		{
			DrawList->AddCircleFilled( ImVec2( Position.X, Position.Y ), Radius, GetColor( Color ) );
		}
	}

	void AddCircle( const Vector3D& Position, float Radius, Color::Type Color )
	{
		bool IsInFront = false;
		auto& ScreenPosition = WorldToScreenPosition( Position, &IsInFront );

		if( IsInFront )
		{
			AddCircle( ScreenPosition, Radius, Color );
		}
	}

	void AddText( const Vector2D& Position, const char* Start, const char* End, Color::Type Color )
	{
		if( DrawList )
		{
			DrawList->AddText( ImVec2( Position.X, Position.Y ), GetColor( Color ), Start, End );
		}
	}

	void AddText( const Vector3D& Position, const char* Start, const char* End, Color::Type Color )
	{
		bool IsInFront = false;
		auto& ScreenPosition = WorldToScreenPosition( Position, &IsInFront );

		if( IsInFront )
		{
			AddText( ScreenPosition, Start, End );
		}
	}

	void AddAABB( const Vector3D& Minimum, const Vector3D& Maximum, Color::Type Color )
	{
		Vector3D BottomNW = Vector3D( Minimum.X, Maximum.Y, Minimum.Z );
		Vector3D BottomNE = Vector3D( Maximum.X, Maximum.Y, Minimum.Z );
		Vector3D BottomSE = Vector3D( Maximum.X, Minimum.Y, Minimum.Z );
		Vector3D BottomSW = Vector3D( Minimum.X, Minimum.Y, Minimum.Z );

		AddLine( BottomNW, BottomNE, Color );
		AddLine( BottomNE, BottomSE, Color );
		AddLine( BottomSE, BottomSW, Color );
		AddLine( BottomSW, BottomNW, Color );

		Vector3D TopNW = Vector3D( Minimum.X, Maximum.Y, Maximum.Z );
		Vector3D TopNE = Vector3D( Maximum.X, Maximum.Y, Maximum.Z );
		Vector3D TopSE = Vector3D( Maximum.X, Minimum.Y, Maximum.Z );
		Vector3D TopSW = Vector3D( Minimum.X, Minimum.Y, Maximum.Z );

		AddLine( TopNW, TopNE, Color );
		AddLine( TopNE, TopSE, Color );
		AddLine( TopSE, TopSW, Color );
		AddLine( TopSW, TopNW, Color );

		AddLine( TopNW, BottomNW, Color );
		AddLine( TopNE, BottomNE, Color );
		AddLine( TopSE, BottomSE, Color );
		AddLine( TopSW, BottomSW, Color );
	}

	void AddBox( const Vector3D& Center, const Vector3D& Size, Color::Type Color )
	{
		const Vector3D HalfSize = Size * 0.5f;
		const Vector3D Minimum = Center - Size;
		const Vector3D Maximum = Center + Size;
		AddAABB( Minimum, Maximum, Color );
	}

	void Reset()
	{
		CConfiguration& Configuration = CConfiguration::Get();
		Width = Configuration.GetFloat( "width" );
		Height = Configuration.GetFloat( "height" );

		if( DrawList )
		{
			delete DrawList;
			DrawList = nullptr;
		}

		if( !DrawList )
		{
			DrawList = new ImDrawList( ImGui::GetDrawListSharedData() );
		}

		DrawData.DisplayPos.x = 0.0f;
		DrawData.DisplayPos.y = 0.0f;
		DrawData.DisplaySize.x = Width;
		DrawData.DisplaySize.y = Height;
		DrawData.FramebufferScale.x = 1.0f;
		DrawData.FramebufferScale.y = 1.0f;
	}

	void Refresh()
	{
		if( DrawList )
		{
			DrawList->Clear();
			DrawData.CmdLists = &DrawList;
			DrawData.CmdListsCount = 1;
			DrawData.TotalVtxCount = DrawList->VtxBuffer.size();
			DrawData.TotalIdxCount = DrawList->IdxBuffer.size();
			DrawData.Valid = true;

			DrawList->PushClipRectFullScreen();
			DrawList->PushTextureID( ImGui::GetIO().Fonts->TexID );
		}
		else
		{
			DrawData.CmdListsCount = 0;
		}
	}

	void Frame()
	{
		if( DrawList->CmdBuffer.size() == 0 )
		{
			DrawList->AddDrawCmd();
		}

		ImGui_ImplOpenGL3_RenderDrawData( &DrawData );
	}

	void SetCamera( const CCamera& CameraIn )
	{
		Camera = CameraIn;
	}
}
