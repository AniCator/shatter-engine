// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math.h>
#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Color.h>

class CTexture;

namespace UI
{
	Vector3D ScreenPositionToWorld( const Vector2D& ScreenPosition );
	Vector2D WorldToScreenPosition( const Vector3D& WorldPosition, bool* IsInFront = nullptr );

	void AddLine( const Vector2D& Start, const Vector2D& End, const Color& Color = Color::Green );
	void AddLine( const Vector3D& Start, const Vector3D& End, const Color& Color = Color::Green, const double& Duration = -1.0 );
	void AddVector( const Vector3D& Origin, const Vector3D& Vector, const Color& Color = Color::Green, const double& Duration = -1.0 );
	void AddTriangleFilled( const Vector2D& A, const Vector2D& B, const Vector2D& C, const Color& Color = Color::Green );
	void AddTriangleFilled( const Vector3D& A, const Vector3D& B, const Vector3D& C, const Color& Color = Color::Green );
	void AddCircle( const Vector2D& Position, float Radius, const Color& Color = Color::Red );
	void AddCircle( const Vector3D& Position, float Radius, const Color& Color = Color::Red );
	void AddText( const Vector2D& Position, const char* Start, const char* End = nullptr, const Color& Color = Color::White, const float Size = 16.0f );
	void AddText( const Vector3D& Position, const char* Start, const char* End = nullptr, const Color& Color = Color::White, const Vector2D& Offset = { 0.0f,0.0f } );

	void AddText( const Vector2D& Position, const std::string& Name, const Vector3D& Vector, const Color& Color = Color::White, const float Size = 16.0f );
	void AddText( const Vector3D& Position, const std::string& Name, const Vector3D& Vector, const Color& Color = Color::White, const Vector2D& Offset = { 0.0f,0.0f } );

	void AddText( const Vector2D& Position, const std::string& Name, const float& Float, const Color& Color = Color::White, const float Size = 16.0f );
	void AddText( const Vector3D& Position, const std::string& Name, const float& Float, const Color& Color = Color::White, const Vector2D& Offset = { 0.0f,0.0f } );

	void AddText( const char* Start, const char* End = nullptr, const Color& Color = Color::White );
	void AddText( const std::string& Text, const Color& Color = Color::White );

	void AddText( const std::string& Name, const Vector3D& Vector, const Color& Color = Color::White );
	void AddText( const std::string& Name, const float& Float, const Color& Color = Color::White );

	template<typename T>
	void DebugVariable( const std::string& Name, const T& Variable, const Color& Color = Color::White )
	{
		const std::string Combined = Name + ": " + std::to_string( Variable );
		AddText( Combined, Color );
	}
	
	void AddImage( const Vector3D& Position, const Vector2D& Size, const ::CTexture* Texture, const Color& Color = Color::White );
	void AddAABB( const Vector3D& Minimum, const Vector3D& Maximum, const Color& Color = Color::Blue, const double& Duration = -1.0 );
	void AddBox( const Vector3D& Center, const Vector3D& Size, const Color& Color = Color::Blue );
	void AddSphere( const Vector3D& Center, const float& Radius, const Color& Color = Color::Blue, const double& Duration = -1.0 );

	void Reset();

	void Refresh();
	void Frame();
	void Render();

	void SetCamera( const CCamera& Camera );

	unsigned int GetWidth();
	unsigned int GetHeight();
	Vector2D RelativeToAbsolute( const Vector2D& Position );
}

#define DrawDebugVariable(Variable) UI::DebugVariable(#Variable,Variable)
#define DrawDebugText(Text) UI::AddText(Text, nullptr)
#define DrawDebugVectorRed(Origin, Vector) UI::AddLine(Origin, Origin + Vector, Color::Red, 0.5f)
#define DrawDebugVectorGreen(Origin, Vector) UI::AddLine(Origin, Origin + Vector, Color::Green, 0.5f)
#define DrawDebugVectorBlue(Origin, Vector) UI::AddLine(Origin, Origin + Vector, Color::Blue, 0.5f)
