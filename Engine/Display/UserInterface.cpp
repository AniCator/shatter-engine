// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "UserInterface.h"

#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Camera.h>

#include <ThirdParty/imgui-1.70/imgui.h>
#include <Engine/Display/imgui_impl_opengl3.h>

Color Color::Red = Color( 255, 0, 0 );
Color Color::Green = Color( 0, 255, 0 );
Color Color::Blue = Color( 0, 0, 255 );
Color Color::White = Color( 255, 255, 255 );
Color Color::Black = Color( 0, 0, 0 );

namespace UI
{
	bool Ready = false;
	ImDrawList* DrawList = nullptr;
	ImDrawData DrawData;
	
	CCamera Camera;
	float Width;
	float Height;

	struct DrawLine
	{
		DrawLine( const Vector3D& Start, const Vector3D& End, const Color& Color )
		{
			this->Start = Start;
			this->End = End;
			this->Color = Color;
		}

		Vector3D Start;
		Vector3D End;
		Color Color;
	};

	std::vector<DrawLine> Lines;

	struct DrawCircle
	{
		DrawCircle( const Vector3D& Position, const float& Radius, const Color& Color )
		{
			this->Position = Position;
			this->Radius = Radius;
			this->Color = Color;
		}

		Vector3D Position;
		float Radius;
		Color Color;
	};

	std::vector<DrawCircle> Circles;

	struct DrawText
	{
		DrawText( const Vector3D& Position, const char* Start, const char* End, const Color& Color )
		{
			this->Position = Position;
			
			if( End )
			{
				Length = End - Start;
			}
			else
			{
				Length = strlen( Start );
			}

			this->Text = new char[Length + 1];
			for( size_t Index = 0; Index < Length; Index++ )
			{
				Text[Index] = Start[Index];
			}

			this->Color = Color;
		}

		Vector3D Position;
		char* Text;
		size_t Length;
		Color Color;
	};

	std::vector<DrawText> Texts;

	ImU32 GetColor( Color Color )
	{
		return IM_COL32( Color.R, Color.G, Color.B, Color.A );
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
			Front = false;
		}
		else if( ScreenPosition.X > Width )
		{
			Front = false;
		}

		if( ScreenPosition.Y < 0.0f )
		{
			Front = false;
		}
		else if( ScreenPosition.Y > Height )
		{
			Front = false;
		}

		if( IsInFront )
		{
			*IsInFront = Front;
		}

		return ScreenPosition;
	}

	void AddLine( const Vector2D& Start, const Vector2D& End, const Color& Color )
	{
		if( DrawList )
		{
			DrawList->AddLine( ImVec2( Start.X, Start.Y ), ImVec2( End.X, End.Y ), GetColor( Color ), 1.0f );
		}
	}

	void AddLineInternal( const Vector3D& Start, const Vector3D& End, const Color& Color )
	{
		bool StartIsInFront = false;
		auto& ScreenStart = WorldToScreenPosition( Start, &StartIsInFront );

		bool EndIsInFront = false;
		auto& ScreenEnd = WorldToScreenPosition( End, &EndIsInFront );

		if( StartIsInFront || EndIsInFront )
		{
			AddLine( ScreenStart, ScreenEnd, Color );
		}
	}

	void AddLine( const Vector3D& Start, const Vector3D& End, const Color& Color )
	{
		DrawLine Line( Start, End, Color );
		Lines.emplace_back( Line );
	}

	void AddTriangleFilled( const Vector2D& A, const Vector2D& B, const Vector2D& C, const Color& Color )
	{
		if( DrawList )
		{
			DrawList->AddTriangleFilled( ImVec2( A.X, A.Y ), ImVec2( B.X, B.Y ), ImVec2( C.X, C.Y ), GetColor( Color ) );
		}
	}

	void AddTriangleFilled( const Vector3D& A, const Vector3D& B, const Vector3D& C, const Color& Color )
	{
		bool IsInFrontA = false;
		auto& ScreenA = WorldToScreenPosition( A, &IsInFrontA );

		bool IsInFrontB = false;
		auto& ScreenB = WorldToScreenPosition( B, &IsInFrontB );

		bool IsInFrontC = false;
		auto& ScreenC = WorldToScreenPosition( C, &IsInFrontC );

		if( IsInFrontA || IsInFrontB || IsInFrontC )
		{
			AddTriangleFilled( ScreenA, ScreenB, ScreenC, Color );
		}
	}

	void AddCircle( const Vector2D& Position, float Radius, const Color& Color )
	{
		if( DrawList )
		{
			DrawList->AddCircleFilled( ImVec2( Position.X, Position.Y ), Radius, GetColor( Color ) );
		}
	}

	void AddCircleInternal( const Vector3D& Position, float Radius, const Color& Color )
	{
		bool IsInFront = false;
		auto& ScreenPosition = WorldToScreenPosition( Position, &IsInFront );

		if( IsInFront )
		{
			AddCircle( ScreenPosition, Radius, Color );
		}
	}

	void AddCircle( const Vector3D& Position, float Radius, const Color& Color )
	{
		DrawCircle Circle( Position, Radius, Color );
		Circles.emplace_back( Circle );
	}

	void AddText( const Vector2D& Position, const char* Start, const char* End, const Color& Color )
	{
		if( DrawList )
		{
			DrawList->AddText( ImVec2( Position.X, Position.Y ), GetColor( Color ), Start, End );
		}
	}

	void AddTextInternal( const Vector3D& Position, const char* Start, const char* End, const Color& Color )
	{
		bool IsInFront = false;
		auto& ScreenPosition = WorldToScreenPosition( Position, &IsInFront );

		if( IsInFront )
		{
			AddText( ScreenPosition, Start, End );
		}
	}

	void AddText( const Vector3D& Position, const char* Start, const char* End, const Color& Color )
	{
		DrawText Text( Position, Start, End, Color );
		Texts.emplace_back( Text );
	}

	void AddAABB( const Vector3D& Minimum, const Vector3D& Maximum, const Color& Color )
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

	void AddBox( const Vector3D& Center, const Vector3D& Size, const Color& Color )
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
		Lines.clear();
		Circles.clear();
		Texts.clear();
	}

	void Frame()
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

		for( const auto& Line : Lines )
		{
			AddLineInternal( Line.Start, Line.End, Line.Color );
		}

		for( const auto& Circle : Circles )
		{
			AddCircleInternal( Circle.Position, Circle.Radius, Circle.Color );
		}

		for( const auto& Text : Texts )
		{
			AddTextInternal( Text.Position, Text.Text, Text.Text + Text.Length, Text.Color );
		}

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
