// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "UserInterface.h"

#include <Engine/Application/Application.h>
#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/Display/Rendering/RenderTexture.h>
#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Resource/Assets.h>
#include <Engine/World/World.h>

#include <Game/Game.h>

#include <ThirdParty/imgui-1.70/imgui.h>
#include <Engine/Display/imgui_impl_opengl3.h>

Color Color::Red = Color( 255, 0, 0 );
Color Color::Green = Color( 0, 255, 0 );
Color Color::Blue = Color( 0, 0, 255 );
Color Color::White = Color( 255, 255, 255 );
Color Color::Black = Color( 0, 0, 0 );

Color Color::Yellow = Color( 255, 255, 0 );
Color Color::Purple = Color( 255, 0, 255 );
Color Color::Cyan = Color( 0, 255, 255 );

extern ConfigurationVariable<bool> FlipHorizontal;

std::mutex LineMutex;

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
		DrawLine( const Vector3D& Start, const Vector3D& End, const Color& Color, const double Duration = 0.0 )
		{
			this->Start = Start;
			this->End = End;
			this->Color = Color;
			this->StartTime = GameLayersInstance->GetCurrentTime();
			this->Duration = Duration;
		}

		Vector3D Start = Vector3D::Zero;
		Vector3D End = Vector3D::One;
		Color Color = Color::White;
		double StartTime = 0.0;
		double Duration = 0.0;
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

	struct DrawCircleScreen
	{
		DrawCircleScreen( const Vector2D& Position, const float& Radius, const Color& Color )
		{
			this->Position = Position;
			this->Radius = Radius;
			this->Color = Color;
		}

		Vector2D Position;
		float Radius;
		Color Color;
	};

	std::vector<DrawCircleScreen> CirclesScreen;

	struct DrawImage
	{
		DrawImage( const Vector3D& Position, const Vector2D& Size, const ::CTexture* Texture, const Color& Color )
		{
			this->Position = Position;
			this->Size = Size;
			this->Texture = reinterpret_cast<ImTextureID>( Texture->GetHandle() );
			this->Color = Color;
		}

		Vector3D Position;
		Vector2D Size;
		ImTextureID Texture;
		Color Color;
	};

	std::vector<DrawImage> Images;

	struct DrawText
	{
		DrawText() = delete;
		DrawText( const Vector3D& Position, const char* Start, const char* End, const Color& Color, const Vector2D& Offset = { 0.0f,0.0f } )
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
			memcpy( Text, Start, Length + 1 );

			this->Color = Color;
			this->Offset = Offset;
		}

		Vector3D Position;
		Vector2D Offset;
		char* Text = nullptr;
		size_t Length;
		Color Color;
	};

	std::vector<DrawText> Texts;

	struct DrawTextScreen
	{
		DrawTextScreen( const Vector2D& Position, const char* Start, const char* End, const Color& Color, const float Size )
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
			memcpy( Text, Start, Length + 1 );

			this->Color = Color;
			this->Size = Size;
		}

		Vector2D Position;
		char* Text = nullptr;
		size_t Length;
		Color Color;
		float Size;
	};

	std::vector<DrawTextScreen> TextsScreen;

	struct FixedText
	{
		FixedText() = delete;
		FixedText( const char* Start, const char* End, const Color& Color )
		{
			if( End )
			{
				Length = End - Start;
			}
			else
			{
				Length = strlen( Start );
			}

			this->Text = new char[Length + 1];
			memcpy( Text, Start, Length + 1 );

			this->Color = Color;
		}

		char* Text = nullptr;
		size_t Length;
		Color Color;
	};

	std::vector<FixedText> FixedTextOutput;

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

		float NormalizedScreenPositionX;
		if ( FlipHorizontal )
		{
			NormalizedScreenPositionX = ( 2.0f * ScreenPosition[0] ) / Width - 1.0f;
		}
		else
		{
			NormalizedScreenPositionX = ( 2.0f * (1.0f - ScreenPosition[0]) ) / Width - 1.0f;
		}

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

		const glm::vec4 WorldPositionHomogenoeus = glm::vec4( Math::ToGLM( WorldPosition ), 1.0f );
		const glm::vec4 ClipSpacePosition = ProjectionMatrix * ViewMatrix * WorldPositionHomogenoeus;
		const glm::vec3 NormalizedPosition = glm::vec3( ClipSpacePosition.x, -ClipSpacePosition.y, ClipSpacePosition.z ) / ClipSpacePosition.w;
		
		bool Front = ClipSpacePosition.w > 0.0f;
		const float Bias = Front ? 1.0f : -1.0f;

		Vector2D ScreenPosition;

		ScreenPosition.X = ( NormalizedPosition.x * 0.5f + 0.5f );

		if ( FlipHorizontal )
		{
			ScreenPosition.X = 1.0f - ScreenPosition.X;
		}

		ScreenPosition.X *= Width;
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

	// Liang-Barsky function by Daniel White @ http://www.skytopia.com/project/articles/compsci/clipping.html
	// This function inputs 8 numbers, and outputs 4 new numbers (plus a boolean value to say whether the clipped line is drawn at all).
	//
	template<typename T> bool LiangBarsky( T edgeLeft, T edgeRight, T edgeBottom, T edgeTop,   // Define the x/y clipping values for the border.
		T x0src, T y0src, T x1src, T y1src,                 // Define the start and end points of the line.
		T& x0clip, T& y0clip, T& x1clip, T& y1clip )         // The output values, so declare these outside.
	{

		T t0 = 0.0; T t1 = 1.0;
		T xdelta = x1src - x0src;
		T ydelta = y1src - y0src;
		T p, q, r;

		for( int edge = 0; edge < 4; edge++ ) {   // Traverse through left, right, bottom, top edges.
			if( edge == 0 ) { p = -xdelta;    q = -( edgeLeft - x0src ); }
			if( edge == 1 ) { p = xdelta;     q = ( edgeRight - x0src ); }
			if( edge == 2 ) { p = -ydelta;    q = -( edgeBottom - y0src ); }
			if( edge == 3 ) { p = ydelta;     q = ( edgeTop - y0src ); }
			r = q / p;
			if( p == 0 && q < 0 ) return false;   // Don't draw line at all. (parallel line outside)

			if( p < 0 ) {
				if( r > t1 ) return false;         // Don't draw line at all.
				else if( r > t0 ) t0 = r;            // Line is clipped!
			}
			else if( p > 0 ) {
				if( r < t0 ) return false;      // Don't draw line at all.
				else if( r < t1 ) t1 = r;         // Line is clipped!
			}
		}

		x0clip = x0src + t0 * xdelta;
		y0clip = y0src + t0 * ydelta;
		x1clip = x0src + t1 * xdelta;
		y1clip = y0src + t1 * ydelta;

		return true;        // (clipped) line is drawn
	}

	Vector2D WorldLineToScreenPosition( const Vector3D& WorldPositionA, const Vector3D& WorldPositionB, bool* IsInFront )
	{
		const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();
		const glm::mat4& ViewMatrix = Camera.GetViewMatrix();

		glm::vec4 WorldPositionHomogenoeusA = glm::vec4( Math::ToGLM( WorldPositionA ), 1.0f );
		glm::vec4 ClipSpacePositionA = ProjectionMatrix * ViewMatrix * WorldPositionHomogenoeusA;
		glm::vec3 NormalizedPositionA = glm::vec3( ClipSpacePositionA.x, -ClipSpacePositionA.y, ClipSpacePositionA.z ) / ClipSpacePositionA.w;

		glm::vec4 WorldPositionHomogenoeusB = glm::vec4( Math::ToGLM( WorldPositionB ), 1.0f );
		glm::vec4 ClipSpacePositionB = ProjectionMatrix * ViewMatrix * WorldPositionHomogenoeusB;
		glm::vec3 NormalizedPositionB = glm::vec3( ClipSpacePositionB.x, -ClipSpacePositionB.y, ClipSpacePositionB.z ) / ClipSpacePositionB.w;

		bool Front = ClipSpacePositionB.w > 0.0f;
		const float Bias = Front ? 1.0f : -1.0f;

		Vector2D ScreenPositionA;
		ScreenPositionA.X = ( NormalizedPositionA.x * 0.5f + 0.5f ) * Width;
		ScreenPositionA.Y = ( NormalizedPositionA.y * 0.5f + 0.5f ) * Height;

		Vector2D ScreenPositionB;
		ScreenPositionB.X = ( NormalizedPositionB.x * 0.5f + 0.5f ) * Width;
		ScreenPositionB.Y = ( NormalizedPositionB.y * 0.5f + 0.5f ) * Height;

		Vector2D DrawPositionA, DrawPositionB;

		const bool Valid = LiangBarsky( 0.0f, Width, 0.0f, Height, ScreenPositionA.X, ScreenPositionA.Y, ScreenPositionB.X, ScreenPositionB.Y, DrawPositionA.X, DrawPositionA.Y, DrawPositionB.X, DrawPositionB.Y );

		if( ScreenPositionB.X < 0.0f )
		{
			Front = false;
		}
		else if( ScreenPositionB.X > Width )
		{
			Front = false;
		}

		if( ScreenPositionB.Y < 0.0f )
		{
			Front = false;
		}
		else if( ScreenPositionB.Y > Height )
		{
			Front = false;
		}

		if( IsInFront )
		{
			*IsInFront = Front && Valid;
		}

		if( Valid )
		{
			return DrawPositionB;
		}

		return ScreenPositionB;
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

		Vector2D DrawPositionA, DrawPositionB;
		const bool Valid = LiangBarsky( 0.0f, Width, 0.0f, Height, ScreenStart.X, ScreenStart.Y, ScreenEnd.X, ScreenEnd.Y, DrawPositionA.X, DrawPositionA.Y, DrawPositionB.X, DrawPositionB.Y );

		if( ( StartIsInFront || EndIsInFront ) && Valid )
		{
			AddLine( DrawPositionA, DrawPositionB, Color );
		}
	}

	void AddLine( const Vector3D& Start, const Vector3D& End, const Color& Color, const double& Duration )
	{
		if( CApplication::IsPaused() )
			return;

		std::unique_lock<std::mutex> Lock( LineMutex );
		
		DrawLine Line( Start, End, Color, Duration );
		Lines.emplace_back( Line );
	}

	void AddVector( const Vector3D& Origin, const Vector3D& Vector, const Color& Color, const double& Duration )
	{
		if( CApplication::IsPaused() )
			return;

		std::unique_lock<std::mutex> Lock( LineMutex );

		DrawLine Line( Origin, Origin + Vector, Color, Duration );
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
		const auto ScreenA = WorldToScreenPosition( A, &IsInFrontA );

		bool IsInFrontB = false;
		const auto ScreenB = WorldToScreenPosition( B, &IsInFrontB );

		bool IsInFrontC = false;
		const auto ScreenC = WorldToScreenPosition( C, &IsInFrontC );

		if( IsInFrontA || IsInFrontB || IsInFrontC )
		{
			AddTriangleFilled( ScreenA, ScreenB, ScreenC, Color );
		}
	}

	void AddCircle2D( const Vector2D& Position, float Radius, const Color& Color )
	{
		if( DrawList )
		{
			DrawList->AddCircleFilled( ImVec2( Position.X, Position.Y ), Radius, GetColor( Color ) );
		}
	}

	void AddCircleInternal( const Vector3D& Position, float Radius, const Color& Color )
	{
		bool IsInFront = false;
		const auto& ScreenPosition = WorldToScreenPosition( Position, &IsInFront );

		if( IsInFront )
		{
			AddCircle2D( ScreenPosition, Radius, Color );
		}
	}

	void AddCircle( const Vector2D& Position, float Radius, const Color& Color )
	{
		if( CApplication::IsPaused() )
			return;
		
		DrawCircleScreen Circle( Position, Radius, Color );
		CirclesScreen.emplace_back( Circle );
	}

	void AddCircle( const Vector3D& Position, float Radius, const Color& Color )
	{
		if( CApplication::IsPaused() )
			return;
		
		DrawCircle Circle( Position, Radius, Color );
		Circles.emplace_back( Circle );
	}

	void AddText2D( const Vector2D& Position, const char* Start, const char* End, const Color& Color, const float Size )
	{
		if( DrawList )
		{
			DrawList->AddText( ImGui::GetIO().FontDefault, Size, ImVec2( Position.X, Position.Y ), GetColor( Color ), Start, End );
		}
	}

	void AddTextInternal( const Vector2D& Position, const char* Start, const char* End, const Color& Color, const float Size )
	{
		AddText2D( Position, Start, End, Color, Size );
	}

	void AddText( const Vector2D& Position, const char* Start, const char* End, const Color& Color, const float Size )
	{
		if( CApplication::IsPaused() )
			return;
		
		DrawTextScreen Text( Position, Start, End, Color, Size );
		TextsScreen.emplace_back( Text );
	}

	void AddTextInternal( const Vector3D& Position, const char* Start, const char* End, const Color& Color, const Vector2D& Offset = { 0.0f,0.0f } )
	{
		bool IsInFront = false;
		const auto& ScreenPosition = WorldToScreenPosition( Position, &IsInFront ) + Offset;

		if( IsInFront )
		{
			AddText2D( ScreenPosition, Start, End, Color, 16.0f );
		}
	}

	void AddText( const Vector3D& Position, const char* Start, const char* End, const Color& Color, const Vector2D& Offset )
	{
		if( CApplication::IsPaused() )
			return;
		
		DrawText Text( Position, Start, End, Color, Offset );
		Texts.emplace_back( Text );
	}

	void AddText( const Vector2D& Position, const std::string& Name, const Vector3D& Vector, const Color& Color, const float Size )
	{
		const std::string VectorString = Name + ": " + std::to_string( Vector.X ) + ", " + std::to_string( Vector.Y ) + ", " + std::to_string( Vector.Z );
		AddText( Position, VectorString.c_str(), nullptr, Color, Size );
	}

	void AddText( const Vector3D& Position, const std::string& Name, const Vector3D& Vector, const Color& Color, const Vector2D& Offset )
	{
		const std::string VectorString = Name + ": " + std::to_string( Vector.X ) + ", " + std::to_string( Vector.Y ) + ", " + std::to_string( Vector.Z );
		AddText( Position, VectorString.c_str(), nullptr, Color, Offset );
	}

	void AddText( const Vector2D& Position, const std::string& Name, const float& Float, const Color& Color, const float Size )
	{
		const std::string VectorString = Name + ": " + std::to_string( Float );
		AddText( Position, VectorString.c_str(), nullptr, Color, Size );
	}

	void AddText( const Vector3D& Position, const std::string& Name, const float& Float, const Color& Color, const Vector2D& Offset )
	{
		const std::string VectorString = Name + ": " + std::to_string( Float );
		AddText( Position, VectorString.c_str(), nullptr, Color, Offset );
	}

	void AddText( const char* Start, const char* End, const Color& Color )
	{
		FixedText Text( Start, End, Color );
		FixedTextOutput.emplace_back( Text );
	}

	void AddText( const std::string& Text, const Color& Color )
	{
		AddText( Text.c_str(), nullptr, Color );
	}

	void AddText( const std::string& Name, const Vector3D& Vector, const Color& Color )
	{
		const std::string VectorString = Name + ": " + std::to_string( Vector.X ) + ", " + std::to_string( Vector.Y ) + ", " + std::to_string( Vector.Z );
		AddText( VectorString.c_str(), nullptr, Color );
	}

	void AddText( const std::string& Name, const float& Float, const Color& Color )
	{
		const std::string VectorString = Name + ": " + std::to_string( Float );
		AddText( VectorString.c_str(), nullptr, Color );
	}

	void RenderFixedTextOutput()
	{
		auto Position = Vector2D( 10.0f, 20.0f );
		for( const auto& Text : FixedTextOutput )
		{
			size_t LineBreaks = 0;
			for( size_t Index = 0; Index < Text.Length; Index++ )
			{
				if( Text.Text[Index] == '\n')
				{
					LineBreaks++;
				}
			}

			AddText2D( Position, Text.Text, Text.Text + Text.Length, Text.Color, 16.0f );
			Position.Y += 16.0f + 16.0f * LineBreaks;
		}
	}

	void AddImageInternal( const Vector3D& Position, const Vector2D& Size, const ImTextureID Texture, const Color& Color )
	{	
		if( DrawList )
		{
			bool IsInFront = false;
			const auto& ScreenPosition = WorldToScreenPosition( Position, &IsInFront );

			if( IsInFront )
			{
				const auto Minimum = ScreenPosition - Size * 0.5f;
				const auto Maximum = ScreenPosition + Size * 0.5f;
				DrawList->AddImage( Texture, ImVec2( Minimum.X, Minimum.Y ), ImVec2( Maximum.X, Maximum.Y ), ImVec2( 0, 1 ), ImVec2( 1, 0 ), GetColor( Color ) );
			}
		}
	}

	void AddImage( const Vector3D& Position, const Vector2D& Size, const ::CTexture* Texture, const Color& Color )
	{
		if( !Texture )
			return;

		if( CApplication::IsPaused() )
			return;
		
		DrawImage Image( Position, Size, Texture, Color );
		Images.emplace_back( Image );
	}

	void AddAABB( const Vector3D& Minimum, const Vector3D& Maximum, const Color& Color, const double& Duration )
	{
		if( CApplication::IsPaused() )
			return;
		
		const auto BottomNW = Vector3D( Minimum.X, Maximum.Y, Minimum.Z );
		const auto BottomNE = Vector3D( Maximum.X, Maximum.Y, Minimum.Z );
		const auto BottomSE = Vector3D( Maximum.X, Minimum.Y, Minimum.Z );
		const auto BottomSW = Vector3D( Minimum.X, Minimum.Y, Minimum.Z );

		// Draw the bottom quad.
		AddLine( BottomNW, BottomNE, Color, Duration );
		AddLine( BottomNE, BottomSE, Color, Duration );
		AddLine( BottomSE, BottomSW, Color, Duration );
		AddLine( BottomSW, BottomNW, Color, Duration );

		const auto TopNW = Vector3D( Minimum.X, Maximum.Y, Maximum.Z );
		const auto TopNE = Vector3D( Maximum.X, Maximum.Y, Maximum.Z );
		const auto TopSE = Vector3D( Maximum.X, Minimum.Y, Maximum.Z );
		const auto TopSW = Vector3D( Minimum.X, Minimum.Y, Maximum.Z );

		// Draw the top quad.
		AddLine( TopNW, TopNE, Color, Duration );
		AddLine( TopNE, TopSE, Color, Duration );
		AddLine( TopSE, TopSW, Color, Duration );
		AddLine( TopSW, TopNW, Color, Duration );

		// Connecting lines between the top and bottom.
		AddLine( TopNW, BottomNW, Color, Duration );
		AddLine( TopNE, BottomNE, Color, Duration );
		AddLine( TopSE, BottomSE, Color, Duration );
		AddLine( TopSW, BottomSW, Color, Duration );
	}

	void AddBox( const Vector3D& Center, const Vector3D& Size, const Color& Color )
	{
		if( CApplication::IsPaused() )
			return;
		
		const Vector3D HalfSize = Size * 0.5f;
		const Vector3D Minimum = Center - Size;
		const Vector3D Maximum = Center + Size;
		AddAABB( Minimum, Maximum, Color );
	}

	void AddSphere( const Vector3D& Center, const float& Radius, const Color& Color, const double& Duration )
	{
		constexpr uint32_t Steps = 16;
		constexpr float Delta = 1.0f / static_cast<float>( Steps );
		float Rotation = 0.0f;
		for( uint32_t Index = 0; Index < Steps; Index++ )
		{
			Vector3D SegmentStart = Vector3D::Zero;
			Vector3D SegmentEnd = Vector3D::Zero;

			float Offset = Delta * static_cast<float>( Index ) * Math::Pi2();
			float Offset2 = Delta * static_cast<float>( Index + 1 ) * Math::Pi2();
			SegmentStart.X = sin( Offset );
			SegmentStart.Y = cos( Offset );

			SegmentEnd.X = sin( Offset2 );
			SegmentEnd.Y = cos( Offset2 );

			AddLine( Center + SegmentStart * Radius, Center + SegmentEnd * Radius, Color, Duration );

			SegmentStart = Vector3D::Zero;
			SegmentEnd = Vector3D::Zero;

			SegmentStart.X = sin( Offset );
			SegmentStart.Z = cos( Offset );

			SegmentEnd.X = sin( Offset2 );
			SegmentEnd.Z = cos( Offset2 );
			AddLine( Center + SegmentStart * Radius, Center + SegmentEnd * Radius, Color, Duration );

			SegmentStart = Vector3D::Zero;
			SegmentEnd = Vector3D::Zero;

			SegmentStart.Y = sin( Offset );
			SegmentStart.Z = cos( Offset );

			SegmentEnd.Y = sin( Offset2 );
			SegmentEnd.Z = cos( Offset2 );
			AddLine( Center + SegmentStart * Radius, Center + SegmentEnd * Radius, Color, Duration );

			Rotation += Delta;
		}
	}

	void Reset()
	{
		CConfiguration& Configuration = CConfiguration::Get();
		Width = Configuration.GetFloat( "window.Width" );
		Height = Configuration.GetFloat( "window.Height" );

		if( DrawList )
		{
			delete DrawList;
			DrawList = nullptr;
		}

		if( !DrawList )
		{
			DrawList = new ImDrawList( ImGui::GetDrawListSharedData() );

			if( DrawList->CmdBuffer.empty() )
			{
				DrawList->AddDrawCmd();
			}
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
		std::unique_lock<std::mutex> Lock( LineMutex );

		// Lines can have durations so they have to be removed based whether they have expired.
		const auto CurrentTime = GameLayersInstance->GetCurrentTime();
		for( auto LineIterator = Lines.begin(); LineIterator != Lines.end(); )
		{
			const auto& Line = *LineIterator;
			const auto EndTime = Line.StartTime + Line.Duration;
			if( CurrentTime > EndTime )
			{
				// Note: It's possible that we sometimes may get stuck here when a lot of lines are drawn.
				const auto Offset = LineIterator - Lines.begin();
				std::swap( *LineIterator, Lines.back() );
				Lines.pop_back();
				LineIterator = Lines.begin() + Offset;
			}
			else
			{
				++LineIterator;
			}
		}
		
		Circles.clear();
		CirclesScreen.clear();
		Images.clear();

		for( const auto& Text : Texts )
		{
			delete[] Text.Text;
		}
		Texts.clear();

		for( const auto& Text : TextsScreen )
		{
			delete[] Text.Text;
		}
		TextsScreen.clear();

		for( const auto& Text : FixedTextOutput )
		{
			delete[] Text.Text;
		}
		FixedTextOutput.clear();

		CConfiguration& Configuration = CConfiguration::Get();
		Width = Configuration.GetFloat( "window.Width" );
		Height = Configuration.GetFloat( "window.Height" );

		DrawData.DisplaySize.x = Width;
		DrawData.DisplaySize.y = Height;
	}

	class CLinePass : public CRenderPass
	{
	public:
		CLinePass( int Width, int Height, const CCamera& Camera, const bool AlwaysClear = true ) : CRenderPass( "Lines", Width, Height, Camera, AlwaysClear )
		{
			auto& Assets = CAssets::Get();
			LineShader = Assets.CreateNamedShader( "Line", "Shaders/Line" );
			Capacity = 0;
			RenderLines = nullptr;
		}

		~CLinePass()
		{
			delete RenderLines;
		}

		virtual void Clear() override
		{

		}

		virtual void Draw( CRenderable* Renderable ) override
		{

		}

		struct RenderLine
		{
			Vector4D Position;
			Vector4D Color;
		};

		uint32_t RenderBatch( const size_t StartIndex, const size_t EndIndex )
		{
			GLuint VertexArrayObject = 0;
			GLuint VertexBufferObject = 0;

			const size_t Points = EndIndex - StartIndex;
			const size_t PayloadSize = Points * sizeof( RenderLine );

			glGenBuffers( 1, &VertexBufferObject );
			glBindBuffer( GL_ARRAY_BUFFER, VertexBufferObject );
			glBufferData( GL_ARRAY_BUFFER, PayloadSize, 0, GL_DYNAMIC_DRAW );

			glBufferSubData( GL_ARRAY_BUFFER, 0, PayloadSize, &RenderLines[StartIndex] );

			glGenVertexArrays( 1, &VertexArrayObject );
			glBindVertexArray( VertexArrayObject );

			glBindBuffer( GL_ARRAY_BUFFER, VertexBufferObject );

			glEnableVertexAttribArray( 0 );
			const void* PositionPointer = reinterpret_cast<void*>( offsetof( RenderLine, Position ) );
			glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, sizeof( RenderLine ), PositionPointer );

			glEnableVertexAttribArray( 1 );
			const void* ColorPointer = reinterpret_cast<void*>( offsetof( RenderLine, Color ) );
			glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( RenderLine ), ColorPointer );

			glBindVertexArray( VertexArrayObject );
			// glBindBuffer( GL_ARRAY_BUFFER, VertexBufferObject );
			glDrawArrays( GL_LINES, 0, Points );
			Calls++;

			glDeleteVertexArrays( 1, &VertexArrayObject );
			VertexArrayObject = 0;

			glDeleteBuffers( 1, &VertexBufferObject );
			VertexBufferObject = 0;

			return Calls;
		}

		virtual uint32_t Render( UniformMap& Uniforms ) override
		{
			if( !LineShader )
				return Calls;

			std::unique_lock<std::mutex> Lock( LineMutex );

			const size_t LineCount = Lines.size();
			const size_t Points = LineCount * 2;

			if( LineCount == 0 )
				return Calls;

			if( Points > Capacity )
			{
				delete RenderLines;

				RenderLines = new RenderLine[Points];
				Capacity = Points;
			}

			memset( RenderLines, 0, Points * sizeof( RenderLine ) );

			int RenderIndex = 0;
			for( size_t Index = 0; Index < LineCount; )
			{
				auto& ByteColor = Lines[Index].Color;
				Vector4D Color;
				Color.R = static_cast<float>( ByteColor.R ) / 255.0f;
				Color.G = static_cast<float>( ByteColor.G ) / 255.0f;
				Color.B = static_cast<float>( ByteColor.B ) / 255.0f;
				Color.A = static_cast<float>( ByteColor.A ) / 255.0f;

				auto& Start = Lines[Index].Start;
				RenderLines[RenderIndex].Position = Vector4D( Start.X, Start.Y, Start.Z, 1.0f );
				RenderLines[RenderIndex].Color = Vector4D( Color.X, Color.Y, Color.Z, Color.W );

				auto& End = Lines[Index].End;
				RenderLines[RenderIndex + 1].Position = Vector4D( End.X, End.Y, End.Z, 1.0f );
				RenderLines[RenderIndex + 1].Color = Vector4D( Color.X, Color.Y, Color.Z, Color.W );

				Index += 1;
				RenderIndex += 2;
			}

			Begin();

			LineShader->Activate();

			ConfigureBlendMode( LineShader );

			ConfigureDepthMask( LineShader );
			ConfigureDepthTest( LineShader );

			const glm::mat4& ViewMatrix = Camera.GetViewMatrix();
			const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();
			auto ProjectionView = ProjectionMatrix * ViewMatrix;

			const GLint ProjectionViewMatrixLocation = glGetUniformLocation( LineShader->GetHandles().Program, "ProjectionView" );
			if( ProjectionViewMatrixLocation > -1 )
			{
				glUniformMatrix4fv( ProjectionViewMatrixLocation, 1, GL_FALSE, &ProjectionView[0][0] );
			}

			glDisable( GL_CULL_FACE );

			// Render batches
			const static size_t BatchSize = 65536;
			size_t StartIndex = 0;
			size_t EndIndex = StartIndex + Math::Min( Points - StartIndex, BatchSize );
			while( StartIndex != EndIndex )
			{
				Calls += RenderBatch( StartIndex, EndIndex );
				StartIndex = EndIndex;
				EndIndex = StartIndex + Math::Min( Points - StartIndex, BatchSize );
			}

			CProfiler::Get().AddCounterEntry( ProfileTimeEntry( "Debug Lines", static_cast<int64_t>( Lines.size() ) ), true );
			CProfiler::Get().AddCounterEntry( ProfileTimeEntry( "Debug Line Batches", static_cast<int64_t>( EndIndex / BatchSize ) ), true );

			End();

			return Calls;
		}

		CShader* LineShader;
		size_t Capacity;
		RenderLine* RenderLines;
	};

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

		auto* World = CWorld::GetPrimaryWorld();
		if( World && World->GetActiveCamera() )
		{
			auto ActiveCamera = World->GetActiveCamera();
			auto& Window = CWindow::Get();
			static CLinePass LinePass( Window.GetWidth(), Window.GetHeight(), *ActiveCamera );
			LinePass.SetCamera( *ActiveCamera );
			LinePass.ViewportWidth = Window.GetWidth();
			LinePass.ViewportHeight = Window.GetHeight();

			if( Window.GetRenderer().ForceWireFrame )
			{
				Window.GetRenderer().AddRenderPass( &LinePass, RenderPassLocation::Standard );
			}
			else
			{
				Window.GetRenderer().AddRenderPass( &LinePass, RenderPassLocation::Standard );
			}
		}

		for( const auto& Circle : Circles )
		{
			AddCircleInternal( Circle.Position, Circle.Radius, Circle.Color );
		}

		for( const auto& Circle : CirclesScreen )
		{
			AddCircle2D( Circle.Position, Circle.Radius, Circle.Color );
		}

		for( const auto& Image : Images )
		{
			AddImageInternal( Image.Position, Image.Size, Image.Texture, Image.Color );
		}

		for( const auto& Text : Texts )
		{
			AddTextInternal( Text.Position, Text.Text, Text.Text + Text.Length, Text.Color, Text.Offset );
		}

		for( const auto& Text : TextsScreen )
		{
			AddTextInternal( Text.Position, Text.Text, Text.Text + Text.Length, Text.Color, Text.Size );
		}

		RenderFixedTextOutput();

		if( DrawList->CmdBuffer.empty() )
		{
			DrawList->AddDrawCmd();
		}	
	}

	void Render()
	{
		ImGui_ImplOpenGL3_RenderDrawData( &DrawData );
	}

	void SetCamera( const CCamera& CameraIn )
	{
		Camera = CameraIn;
	}

	unsigned int GetWidth()
	{
		return static_cast<unsigned int>( Width );
	}

	unsigned int GetHeight()
	{
		return static_cast<unsigned int>( Height );
	}

	Vector2D RelativeToAbsolute( const Vector2D& Position )
	{
		Vector2D Result;

		Result.X = Position.X * Width;
		Result.Y = Position.Y * Height;
		
		return Result;
	}
}
