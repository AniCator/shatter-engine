// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Shader.h"
#include <Engine/Configuration/Configuration.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Math.h>

#include <sstream>

// Depth pre-pass.
ConRef<bool> UsePrePass( "render.PrePass" );

#define EnableAutoReload 0

// Compares the input file's modification date to that of the provided reference time.
// Returns true when the file is newer than the given time.
bool IsModified( const std::string& Location, const time_t& Time )
{
	const CFile File( Location );
	if( File.Exists() && File.ModificationDate() > Time )
	{
		return true;
	}

	return false;
}

CShader::CShader()
{
	ModificationTime = time( nullptr );

#if EnableAutoReload == 1
	ShouldAutoReload = true;
#endif
}

bool CShader::Load( const bool& ShouldLink )
{
	const bool BuildGeometryShader = ShaderType == EShaderType::Geometry;
	const bool BuildVertexShader = ShaderType == EShaderType::Vertex || ShaderType == EShaderType::Fragment || BuildGeometryShader;
	const bool BuildFragmentShader = ShaderType == EShaderType::Fragment || BuildGeometryShader;
	const bool BuildComputeShader = ShaderType == EShaderType::Compute;

	bool CanLink = true;
	if( BuildGeometryShader )
	{
		// Log::Event( "Building geometry shader.\n" );
		if( !Load( GeometryLocation, Handles.GeometryShader, EShaderType::Geometry ) )
		{
			CanLink = false;
		}
	}
	
	if( BuildVertexShader )
	{
		// Log::Event( "Building vertex shader.\n" );
		if( !Load( VertexLocation, Handles.VertexShader, EShaderType::Vertex ) )
		{
			CanLink = false;
		}
	}

	if( BuildFragmentShader )
	{
		// Log::Event( "Building fragment shader.\n" );
		if( !Load( FragmentLocation, Handles.FragmentShader, EShaderType::Fragment ) )
		{
			CanLink = false;
		}
	}

	if( BuildComputeShader )
	{
		// Log::Event( "Building compute shader.\n" );
		if( !Load( ComputeLocation, Handles.ComputeShader, EShaderType::Compute ) )
		{
			CanLink = false;
		}
	}
	
	if( CanLink && ShouldLink )
	{
		if( Handles.Program != 0 )
		{
			glDeleteProgram( Handles.Program );
		}

		Link();
		return Handles.Program != 0;
	}

	return false;
}

bool CShader::Load( const char* FileLocation, const bool& ShouldLink, const EShaderType& Type )
{
	ShaderType = Type;
	Location = FileLocation;

	// Append the required file extensions.
	VertexLocation = FileLocation;
	VertexLocation += ".vs";
	GeometryLocation = FileLocation;
	GeometryLocation += ".gs";
	FragmentLocation = FileLocation;
	FragmentLocation += ".fs";
	ComputeLocation = FileLocation;
	ComputeLocation += ".cs";

	return Load();
}

bool CShader::Load( const char* VertexLocationIn, const char* FragmentLocationIn, const bool& ShouldLink )
{
	// Append the required file extensions.
	Location = FragmentLocationIn; // for debugging.
	VertexLocation = VertexLocationIn;
	VertexLocation += ".vs";
	FragmentLocation = FragmentLocationIn;
	FragmentLocation += ".fs";

	bool CanLink = true;
	if( !Load( VertexLocation, Handles.VertexShader, EShaderType::Vertex ) )
	{
		CanLink = false;
	}

	if( !Load( FragmentLocation, Handles.FragmentShader, EShaderType::Fragment ) )
	{
		CanLink = false;
	}

	if( CanLink && ShouldLink )
	{
		Link();

		return Handles.Program != 0;
	}

	return false;
}

bool CShader::Load( const std::string& FileLocation, GLuint& HandleIn, const EShaderType& Type )
{
	CFile ShaderSource( FileLocation );
	if( !ShaderSource.Exists() )
		return false;
	
	const bool Loaded = ShaderSource.Load();

	// Update the modification time in case we want to automatially reload the file when it has been changed.
	ModificationTime = ShaderSource.ModificationDate() > ModificationTime ? ShaderSource.ModificationDate() : ModificationTime;

	if( Loaded )
	{
		const std::string Data = Process( ShaderSource );
		const char* ProcessedCode = Data.c_str();
		if( ProcessedCode )
		{
			// NOTE: If a render pass crashes here, it means it is tring to load a shader before OpenGL and GLAD have been initialized.
			const auto NativeType = static_cast<GLuint>( Type );
			HandleIn = glCreateShader( NativeType );
			if( HandleIn > 0 )
			{
				glShaderSource( HandleIn, 1, &ProcessedCode, nullptr );

				return true;
			}
			else
			{
				Log::Event( Log::Error, "Failed to create shader \"%s\".\n", FileLocation.c_str() );
			}
		}
	}

	return false;
}

bool CShader::Load( GLuint& HandleIn, const EShaderType& Type, const std::string& Code )
{
	const std::string Data = Process( Code );
	const char* ProcessedCode = Data.c_str();
	if( ProcessedCode )
	{
		const auto NativeType = static_cast<GLuint>( Type );
		HandleIn = glCreateShader( NativeType );
		if( HandleIn > 0 )
		{
			glShaderSource( HandleIn, 1, &ProcessedCode, nullptr );

			return true;
		}
		else
		{
			Log::Event( Log::Error, "Failed to create shader.\n" );
		}
	}

	return false;
}

bool CShader::Reload()
{
	return Load();
}

GLuint CShader::Activate()
{
	if( ShouldAutoReload )
	{
		const bool UsingGeometryShader = ShaderType == EShaderType::Geometry;
		const bool UsingVertexShader = ShaderType == EShaderType::Vertex || ShaderType == EShaderType::Fragment;
		const bool UsingFragmentShader = ShaderType == EShaderType::Fragment;
		const bool UsingComputeShader = ShaderType == EShaderType::Compute;

		bool Modified = false;
		if( UsingGeometryShader && IsModified( GeometryLocation, ModificationTime ) )
		{
			Modified = true;
		}
		
		if( UsingVertexShader && IsModified( VertexLocation, ModificationTime ) )
		{
			Modified = true;
		}

		if( UsingFragmentShader && IsModified( FragmentLocation, ModificationTime ) )
		{
			Modified = true;
		}

		if( UsingComputeShader && IsModified( ComputeLocation, ModificationTime ) )
		{
			Modified = true;
		}

		if( Modified )
		{
			Reload();
		}
	}

	glUseProgram( Handles.Program );

	return Handles.Program;
}

const FProgramHandles& CShader::GetHandles() const
{
	return Handles;
}

const EBlendMode::Type& CShader::GetBlendMode() const
{
	return BlendMode;
}

const EDepthMask::Type& CShader::GetDepthMask() const
{
	return DepthMask;
}

const EDepthTest::Type& CShader::GetDepthTest() const
{
	return DepthTest;
}

void CShader::AutoReload( const bool& Enable )
{
	ShouldAutoReload = Enable;
}

const std::vector<std::pair<std::string, Uniform>>& CShader::GetDefaults() const
{
	return Defaults;
}

std::string CShader::Process( const CFile& File )
{
	const char* Data = File.Fetch<char>();
	if( Data == nullptr )
		return "";
	
	std::stringstream Stream;
	Stream << Data;

	return Process( Stream );
}

std::string CShader::Process( const std::string& Data )
{
	std::stringstream Stream;
	Stream << Data;

	return Process( Stream );
}

std::string CShader::Process( std::stringstream& Stream )
{
	if( UsePrePass )
	{
		// Default to equal, if we're running a pre-pass.
		DepthTest = EDepthTest::LessEqual;
		DepthMask = EDepthMask::Ignore;
	}
	else
	{
		DepthTest = EDepthTest::Less;
		DepthMask = EDepthMask::Write;
	}

	std::stringstream OutputStream;
	std::string Line;
	while( std::getline( Stream, Line ) )
	{
		bool bParsed = false;
		if( Line[0] == '#' )
		{
			std::stringstream LineStream( Line );
			std::string Preprocessor;
			LineStream >> Preprocessor;

			if( Preprocessor == "#include" )
			{
				std::string Path;
				LineStream >> Path;

				Path = Path.substr( 1, Path.length() - 2 );
				bParsed = true;

				std::stringstream ShaderPath;
				ShaderPath << "Shaders/" << Path;
				CFile IncludeSource( ShaderPath.str() );
				const bool Loaded = IncludeSource.Load();

				if( Loaded )
				{
					const char* IncludeData = IncludeSource.Fetch<char>();
					OutputStream << "\n" << IncludeData << "\n";
				}
			}
			else if( Preprocessor == "#blendmode" )
			{
				std::string Mode;
				LineStream >> Mode;

				BlendMode = EBlendMode::Opaque;

				if( Mode == "alpha" )
				{
					BlendMode = EBlendMode::Alpha;
				}
				else if( Mode == "additive" )
				{
					BlendMode = EBlendMode::Additive;
				}

				bParsed = true;
			}
			else if( Preprocessor == "#depthwrite" )
			{
				std::string Mode;
				LineStream >> Mode;

				DepthMask = EDepthMask::Write;

				if( Mode == "0" )
				{
					DepthMask = EDepthMask::Ignore;
				}

				bParsed = true;
			}
			else if( Preprocessor == "#depthtest" )
			{
				std::string Mode;
				LineStream >> Mode;

				DepthTest = EDepthTest::Less;

				if( Mode == "0" )
				{
					DepthTest = EDepthTest::Never;
				}
				else if( Mode == "<" )
				{
					DepthTest = EDepthTest::Less;
				}
				else if( Mode == "<=" )
				{
					DepthTest = EDepthTest::LessEqual;
				}
				else if( Mode == "==" )
				{
					DepthTest = EDepthTest::Equal;
				}
				else if( Mode == ">" )
				{
					DepthTest = EDepthTest::Greater;
				}
				else if( Mode == "!=" )
				{
					DepthTest = EDepthTest::NotEqual;
				}
				else if( Mode == ">=" )
				{
					DepthTest = EDepthTest::GreaterEqual;
				}
				else if( Mode == "1" )
				{
					DepthTest = EDepthTest::Always;
				}

				bParsed = true;
			}
		}

		if( !bParsed )
		{
			OutputStream << Line << "\n";
		}
	}

	return OutputStream.str();
}

GLuint CShader::Link()
{
	const bool NeedsGeometryShader = ShaderType == EShaderType::Geometry;
	const bool NeedsVertexShader = ShaderType == EShaderType::Vertex || ShaderType == EShaderType::Fragment || NeedsGeometryShader;
	const bool NeedsFragmentShader = ShaderType == EShaderType::Fragment || NeedsGeometryShader;
	const bool NeedsComputeShader = ShaderType == EShaderType::Compute;

	if( NeedsGeometryShader && Handles.GeometryShader == 0 )
	{
		Log::Event( Log::Error, "Shader doesn't have a vertex shader but requires it.\n" );
		return 0;
	}
	
	if( NeedsVertexShader && Handles.VertexShader == 0 )
	{
		Log::Event( Log::Error, "Shader doesn't have a geometry shader but requires it.\n" );
		return 0;
	}

	if( NeedsFragmentShader && Handles.FragmentShader == 0 )
	{
		Log::Event( Log::Error, "Shader doesn't have a fragment shader but requires it.\n" );
		return 0;
	}

	if( NeedsComputeShader && Handles.ComputeShader == 0 )
	{
		Log::Event( Log::Error, "Shader doesn't have a compute shader but requires it.\n" );
		return 0;
	}

	// Compile all shaders
	bool ShaderCompiled = false;
	
#ifdef WIN32
	const bool Debugger = IsDebuggerPresent() > 0;
#else
	const bool Debugger = false;
#endif

	int Attempts = 0;
	while( !ShaderCompiled )
	{
		if( Attempts > 0 && !Debugger )
		{
			Log::Event( Log::Error, "Failed to compile shader \"%s\".\n", Location.c_str() );
			Attempts = 0;
			return 0;
		}

		bool HasNoErrors = true;
		if( NeedsGeometryShader )
		{
			glCompileShader( Handles.GeometryShader );
			if( LogErrorsShader( Handles.GeometryShader, EShaderType::Geometry, "GeometryShader" ) )
			{
				HasNoErrors = false;
			}
		}
		
		if( NeedsVertexShader )
		{
			glCompileShader( Handles.VertexShader );
			if( LogErrorsShader( Handles.VertexShader, EShaderType::Vertex, "VertexShader" ) )
			{
				HasNoErrors = false;
			}
		}

		if( NeedsFragmentShader )
		{
			glCompileShader( Handles.FragmentShader );
			if( LogErrorsShader( Handles.FragmentShader, EShaderType::Fragment, "FragmentShader" ) )
			{
				HasNoErrors = false;
			}
		}

		if( NeedsComputeShader )
		{
			glCompileShader( Handles.ComputeShader );
			if( LogErrorsShader( Handles.ComputeShader, EShaderType::Compute, "ComputeShader" ) )
			{
				HasNoErrors = false;
			}
		}
		
		ShaderCompiled = HasNoErrors;

		if( !ShaderCompiled )
		{
			Load( false );
		}

		Attempts++;
	}

	// Create the program
	const GLuint ProgramHandle = glCreateProgram();

	// Attach shaders to program
	if( NeedsVertexShader )
	{
		glAttachShader( ProgramHandle, Handles.VertexShader );
	}

	if( NeedsGeometryShader )
	{
		glAttachShader( ProgramHandle, Handles.GeometryShader );
	}
	
	if( NeedsFragmentShader )
	{
		glAttachShader( ProgramHandle, Handles.FragmentShader );
	}

	if( NeedsComputeShader )
	{
		glAttachShader( ProgramHandle, Handles.ComputeShader );
	}

	// Link and set program to use
	glLinkProgram( ProgramHandle );

	const bool HasErrorsProgram = LogErrorsProgram( ProgramHandle );

	if( NeedsVertexShader )
	{
		glDeleteShader( Handles.VertexShader );
	}

	if( NeedsGeometryShader )
	{
		glDeleteShader( Handles.GeometryShader );
	}

	if( NeedsFragmentShader )
	{
		glDeleteShader( Handles.FragmentShader );
	}

	if( NeedsComputeShader )
	{
		glDeleteShader( Handles.ComputeShader );
	}

	if( HasErrorsProgram )
	{
		return 0;
	}

	Handles.Program = ProgramHandle;

	GatherDefaults();

	return Handles.Program;
}

void CShader::GatherDefaults()
{
	Uniform Uniform;

	GLint Size = 0;
	GLenum Type = 0;

	constexpr GLsizei BufferSize = 256;
	GLchar Name[BufferSize] = {};
	GLsizei Length = 0;

	GLsizei MaximumLength = 0;
	glGetProgramiv( Handles.Program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &MaximumLength );

	if( MaximumLength > BufferSize )
	{
		Log::Event( Log::Error, "Can't poll defaults.\n" );
		return;
	}

	// Extract the default values.
	GLint ActiveUniforms = 0;
	glGetProgramiv( Handles.Program, GL_ACTIVE_UNIFORMS, &ActiveUniforms );
	for( GLint Index = 0; Index < ActiveUniforms; Index++ )
	{
		glGetActiveUniform( Handles.Program, Index, BufferSize, &Length, &Size, &Type, Name );

		// Log::Event( "Uniform: %s ", Name );

		bool Handled = true;

		switch( Type )
		{
		case GL_FLOAT_VEC4:
		{
			// glUniform4fv
			glGetUniformfv( Handles.Program, Index, &Uniform.Uniform4.X );
			Uniform.Set( Uniform.Uniform4 );

			// Log::Event( "Value: %.3f %.3f %.3f %.3f\n", Uniform.Uniform4.X, Uniform.Uniform4.Y, Uniform.Uniform4.Z, Uniform.Uniform4.W );
			break;
		}
		case GL_FLOAT_VEC3:
		{
			// glUniform3fv
			glGetUniformfv( Handles.Program, Index, &Uniform.Uniform3.X );
			Uniform.Set( Uniform.Uniform3 );
			// Log::Event( "Value: %.3f %.3f %.3f\n", Uniform.Uniform3.X, Uniform.Uniform3.Y, Uniform.Uniform3.Z );
			break;
		}
		case GL_FLOAT_VEC2:
		{
			// glUniform2fv
			glGetUniformfv( Handles.Program, Index, &Uniform.Uniform2.X );
			Uniform.Set( Uniform.Uniform2 );
			// Log::Event( "Value: %.3f %.3f\n", Uniform.Uniform2.X, Uniform.Uniform2.Y );
			break;
		}
		// case GL_FLOAT_MAT4:
		// {
		// 	// glUniformMatrix4fv
		// 	// TODO: 
		// 	break;
		// }
		case GL_UNSIGNED_INT:
		{
			// glUniform1ui
			glGetUniformuiv( Handles.Program, Index, &Uniform.UniformUnsigned );
			Uniform.Set( Uniform.UniformUnsigned );
			// Log::Event( "Value: %u\n", Uniform.UniformUnsigned );
			break;
		}
		case GL_BOOL:
		case GL_INT:
		{
			// glUniform1i
			glGetUniformiv( Handles.Program, Index, &Uniform.UniformSigned4[0] );
			Uniform.Set( Uniform.UniformSigned4[0] );
			// Log::Event( "Value: %i\n", Uniform.UniformSigned4[0] );
			break;
		}
		case GL_INT_VEC4:
		{
			// glUniform4iv
			glGetUniformiv( Handles.Program, Index, Uniform.UniformSigned4 );
			Uniform.Set( Uniform.UniformSigned4 );
			// Log::Event( "Value: %i %i %i %i\n", Uniform.UniformSigned4[0], Uniform.UniformSigned4[1], Uniform.UniformSigned4[2], Uniform.UniformSigned4[3] );
			break;
		}
		case GL_FLOAT:
		{
			// glUniform1f
			glGetUniformfv( Handles.Program, Index, &Uniform.UniformFloat );
			Uniform.Set( Uniform.UniformFloat );
			// Log::Event( "Value: %f\n", Uniform.UniformFloat );
			break;
		}
		default:
		{
			// Log::Event( "\n" );
			Handled = false;
			break;
		}
		}

		if( Handled )
		{
			Defaults.emplace_back();
			auto& Default = Defaults.back();
			Default.first = Name;
			Default.second = Uniform;
		}
	}
}

void PrintLine( const std::string& Data, const int LineNumber )
{
	int Line = 1;
	size_t PreviousOffset = std::string::npos;
	size_t Offset = 0;
	while( Line < LineNumber && Offset < Data.length() && Offset != std::string::npos )
	{
		PreviousOffset = Offset;
		auto LineResetDetect = Data.find( "#line ", Offset + 1 );
		Offset = Data.find( '\n', Offset + 1 );
		Line++;

		if( LineResetDetect < Offset && LineResetDetect != std::string::npos )
		{
			auto LineEnd = Data.find( '\n', LineResetDetect + 6 );
			if( LineResetDetect < LineEnd )
			{
				Line = Math::Integer( Data.substr( LineResetDetect + 6, LineEnd - LineResetDetect ) );
				Line = Math::Max( Line - 1, 0 );
			}
		}
	}

	if( PreviousOffset == std::string::npos )
	{
		Log::Event( "Unable to find line.\n" );
		return;
	}

	const auto Bias = Math::Min( PreviousOffset, 50 );
	auto Start = Data.rfind( '\n', PreviousOffset - Bias );
	auto End = Data.rfind( '\n', Offset + Math::Min( Data.length() - Offset, 50 ) );
	auto Count = End - Start;
	/*if( ( Start + Count + Bias ) < Data.length() )
	{
		Count += Bias;
	}*/

	const auto String = Data.substr( Start, Count );
	Log::Event( Log::Warning, "...\n%s\n", String.c_str() );

	size_t Next = Data.find( "#line ", Start + Count );
	if( Next != std::string::npos )
	{
		PrintLine( Data.substr( Next - 1 ), LineNumber );
	}
}

bool CShader::LogErrorsShader( GLuint Handle, const EShaderType Type, const char* Label )
{
	GLint ByteLength = 0;
	GLsizei StringLength = 0;

	glGetShaderiv( Handle, GL_INFO_LOG_LENGTH, &ByteLength );

	if( ByteLength > 1 )
	{
		GLchar* CompileLog = new GLchar[ByteLength];
		glGetShaderInfoLog( Handle, ByteLength, &StringLength, CompileLog );

		std::string Log = CompileLog;
		delete[] CompileLog;

		Log::Event( Log::Warning, "---Shader Compilation Log--- (%s)\n", Label ? Label : "anonymous" );

		auto LineEnd = Log.find( '(' ) - 1;
		auto LineStart = Log.find( ':' );
		if( LineEnd < LineStart )
		{
			// Assume NVIDIA.
			LineStart = Log.find( '(' );
			LineEnd = Log.find( ')' ) - 1;
		}

		const auto Line = Log.substr( LineStart + 1, LineEnd - LineStart );
		Log::Event( "Line number: %s\n", Line.c_str() );
		const int LineNumber = Math::Integer( Line );

		if( Type == EShaderType::TesselationEvaluation || Type == EShaderType::TesselationControl )
			return true; // Ignore tesselation.

		std::string FileLocation;
		switch( Type )
		{
		case EShaderType::Vertex:
			FileLocation = VertexLocation;
			break;
		case EShaderType::Geometry:
			FileLocation = GeometryLocation;
			break;
		case EShaderType::Compute:
			FileLocation = ComputeLocation;
			break;
		default:
			FileLocation = FragmentLocation;
			break;
		}
		CFile ShaderSource( FileLocation );
		if( ShaderSource.Exists() )
		{
			if( ShaderSource.Load() )
			{
				const std::string Data = Process( ShaderSource );
				PrintLine( Data, LineNumber );
			}
			else
			{
				Log::Event( Log::Warning, "Could not load shader file %s.\n", FileLocation.c_str() );
			}
		}
		else
		{
			Log::Event( Log::Warning, "Shader file %s does not exist.\n", FileLocation.c_str() );
		}

		Log::Event( Log::Error, "%s\n", Log.c_str() );

		return true;
	}

	return false;
}

bool CShader::LogErrorsProgram( GLuint Handle )
{
	GLint ByteLength = 0;
	GLsizei StringLength = 0;

	glGetProgramiv( Handle, GL_INFO_LOG_LENGTH, &ByteLength );

	if( ByteLength > 1 )
	{
		GLchar* CompileLog = new GLchar[ByteLength];
		glGetProgramInfoLog( Handle, ByteLength, &StringLength, CompileLog );

		Log::Event( Log::Error, "---Program Compilation Log---\n%s\n", CompileLog );

		delete[] CompileLog;

		return true;
	}

	return false;
}
