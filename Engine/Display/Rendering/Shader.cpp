// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Shader.h"
#include <Engine/Profiling/Logging.h>

#include <sstream>

#define AutoReload 0

CShader::CShader()
{
	BlendMode = EBlendMode::Opaque;
	DepthMask = EDepthMask::Write;
	DepthTest = EDepthTest::Less;
}

CShader::~CShader()
{

}

bool CShader::Load( bool ShouldLink )
{
	if( VertexLocation.length() > 0 && FragmentLocation.length() > 0 )
	{
		const bool LoadedVertexShader = Load( VertexLocation.c_str(), Handles.VertexShader, EShaderType::Vertex );
		const bool LoadedFragmentShader = Load( FragmentLocation.c_str(), Handles.FragmentShader, EShaderType::Fragment );

		if( LoadedVertexShader && LoadedFragmentShader && ShouldLink )
		{
			Link();

			return Handles.Program != 0;
		}
	}

	return false;
}

bool CShader::Load( const char* FileLocation, bool ShouldLink )
{
	VertexLocation = FileLocation;
	FragmentLocation = FileLocation;

	std::stringstream VertexPath;
	std::stringstream FragmentPath;

	VertexPath << VertexLocation << ".vs";
	FragmentPath << FragmentLocation << ".fs";

	VertexLocation = VertexPath.str();
	FragmentLocation = FragmentPath.str();

	const bool LoadedVertexShader = Load( VertexPath.str().c_str(), Handles.VertexShader, EShaderType::Vertex );
	const bool LoadedFragmentShader = Load( FragmentPath.str().c_str(), Handles.FragmentShader, EShaderType::Fragment );

	if( LoadedVertexShader && LoadedFragmentShader && ShouldLink )
	{
		Link();

		return Handles.Program != 0;
	}

	return false;
}

bool CShader::Load( const char* VertexLocationIn, const char* FragmentLocationIn, bool ShouldLink )
{
	VertexLocation = VertexLocationIn;
	FragmentLocation = FragmentLocationIn;

	std::stringstream VertexPath;
	std::stringstream FragmentPath;

	VertexPath << VertexLocation << ".vs";
	FragmentPath << FragmentLocation << ".fs";

	VertexLocation = VertexPath.str();
	FragmentLocation = FragmentPath.str();

	const bool LoadedVertexShader = Load( VertexPath.str().c_str(), Handles.VertexShader, EShaderType::Vertex );
	const bool LoadedFragmentShader = Load( FragmentPath.str().c_str(), Handles.FragmentShader, EShaderType::Fragment );

	if( LoadedVertexShader && LoadedFragmentShader && ShouldLink )
	{
		Link();

		return Handles.Program != 0;
	}

	return false;
}

bool CShader::Load( const char* FileLocation, GLuint& HandleIn, EShaderType ShaderType )
{
	CFile ShaderSource( FileLocation );
	const bool Loaded = ShaderSource.Load();

	ModificationTime = ShaderSource.ModificationDate() > ModificationTime ? ShaderSource.ModificationDate() : ModificationTime;

	if( Loaded )
	{
		std::string Data = Process( ShaderSource );
		const char* ShaderData = Data.c_str();
		if( ShaderData )
		{
			GLuint ShaderTypeGL = static_cast<GLuint>( ShaderType );

			HandleIn = glCreateShader( ShaderTypeGL );
			glShaderSource( HandleIn, 1, &ShaderData, NULL );

			return true;
		}
	}

	return false;
}

bool CShader::Reload()
{
	Log::Event( "Recompiling \"%s\"...\n", FragmentLocation.c_str() );
	return Load();
}

GLuint CShader::Activate()
{
#if AutoReload == 1
	CFile VertexShader( VertexLocation.c_str() );
	CFile FragmentShader( FragmentLocation.c_str() );

	if( VertexShader.ModificationDate() > ModificationTime || FragmentShader.ModificationDate() > ModificationTime )
	{
		Reload();
	}
#endif

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

bool LogShaderCompilationErrors( GLuint v )
{
	GLint ByteLength = 0;
	GLsizei StringLength = 0;

	glGetShaderiv( v, GL_INFO_LOG_LENGTH, &ByteLength );

	if( ByteLength > 1 )
	{
		GLchar* CompileLog = new GLchar[ByteLength];
		glGetShaderInfoLog( v, ByteLength, &StringLength, CompileLog );

		Log::Event( Log::Error, "---Shader Compilation Log---\n%s\n", CompileLog );

		delete CompileLog;

		return true;
	}

	return false;
}

bool LogProgramCompilationErrors( GLuint v )
{
	GLint ByteLength = 0;
	GLsizei StringLength = 0;

	glGetProgramiv( v, GL_LINK_STATUS, &ByteLength );

	if( ByteLength > 1 )
	{
		GLchar* CompileLog = new GLchar[ByteLength];
		glGetProgramInfoLog( v, ByteLength, &StringLength, CompileLog );

		Log::Event( Log::Error, "---Program Compilation Log---\n%s\n", CompileLog );

		delete CompileLog;

		return true;
	}

	return false;
}

std::string CShader::Process(const CFile& File)
{
	const char* ShaderData = File.Fetch<char>();
	std::stringstream StringStream;
	StringStream << ShaderData;

	std::stringstream OutputStream;

	std::string Line;
	while( std::getline( StringStream, Line ) )
	{
		bool bParsed = false;
		if( Line[0] == '#' )
		{
			std::stringstream Stream( Line );;
			std::string Preprocessor;
			Stream >> Preprocessor;

			if( Preprocessor == "#include" )
			{
				std::string Path;
				Stream >> Path;

				Path = Path.substr(1, Path.length() - 2);
				bParsed = true;

				std::stringstream ShaderPath;
				ShaderPath << "Shaders/" << Path;
				CFile IncludeSource( ShaderPath.str().c_str() );
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
				Stream >> Mode;

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
				Stream >> Mode;

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
				Stream >> Mode;

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
	if( Handles.VertexShader == 0 || Handles.FragmentShader == 0 )
	{
		Log::Event( Log::Error, "Shader doesn't have vertex shader nor fragment shader.\n" );
		return 0;
	}

	// Compile all shaders
	bool ShaderCompiled = false;
	bool Debugger = false;

#ifdef WIN32
	Debugger = IsDebuggerPresent() > 0;
#endif

	int Attempts = 0;
	while( !ShaderCompiled )
	{
		if( Attempts > 0 && !Debugger )
		{
			Log::Event( Log::Error, "Failed to compile shader \"%s\".\n", FragmentLocation.c_str() );
			Attempts = 0;
			return 0;
		}

		glCompileShader( Handles.VertexShader );
		const bool HasErrorsVS = LogShaderCompilationErrors( Handles.VertexShader );
		glCompileShader( Handles.FragmentShader );
		const bool HasErrorsFS = LogShaderCompilationErrors( Handles.FragmentShader );
		ShaderCompiled = !HasErrorsVS && !HasErrorsFS;

		if( !ShaderCompiled )
		{
			Load( false );
		}

		Attempts++;
	}

	// Create the program
	GLuint ProgramHandle = glCreateProgram();

	// Attach shaders to program
	glAttachShader( ProgramHandle, Handles.VertexShader );
	glAttachShader( ProgramHandle, Handles.FragmentShader );

	// Link and set program to use
	glLinkProgram( ProgramHandle );

	const bool HasErrorsProgram = LogProgramCompilationErrors( ProgramHandle );

	glDeleteShader( Handles.VertexShader );
	glDeleteShader( Handles.FragmentShader );

	if( HasErrorsProgram )
	{
		return 0;
	}

	Handles.Program = ProgramHandle;

	return ProgramHandle;
}
