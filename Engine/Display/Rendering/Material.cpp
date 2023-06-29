// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Material.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/Display/Window.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Structures/JSON.h>
#include <Engine/Utility/TranslationTable.h>

void Material::Apply( CRenderable* Renderable ) const
{
	const auto& Assets = CAssets::Get();
	CShader* ShaderObject = Assets.Shaders.Find( Shader );
	Renderable->SetShader( ShaderObject );

	// Fetch the renderer in case we have to bind frame buffers.
	auto& Window = CWindow::Get();
	CRenderer& Renderer = Window.GetRenderer();

	ETextureSlotType Index = 0;
	for( const auto& Texture : Textures )
	{
		ETextureSlot Slot = static_cast<ETextureSlot>( Index );
		Index++; // Update the index before we do anything else.

		CTexture* TextureObject = Assets.Textures.Find( Texture );
		if( !TextureObject )
			continue;

		Renderable->SetTexture( TextureObject, Slot );
	}

	ApplyUniforms( Renderable );

	if( DoubleSided )
	{
		Renderable->GetRenderData().DoubleSided = true;
	}
}

void Material::ApplyUniforms( CRenderable* Renderable ) const
{
	// Transfer uniform information.
	for( const auto& Uniform : Uniforms )
	{
		Renderable->SetUniform( Uniform.first, Uniform.second );
	}
}

Asset* MaterialLoader( AssetParameters& Parameters )
{
	const auto& Location = Parameters[0];
	CFile File( Location );
	if( !File.Exists() )
		return nullptr;

	auto* Asset = new MaterialAsset();
	Asset->Material = LoadMaterial( Location );
	Asset->Location = Location;
	return Asset;
}

void Material::RegisterAssetLoader()
{
	CAssets::Get().RegisterAssetType( "material", MaterialLoader );
}

static auto ConvertSurfaceString = Translate<std::string, PhysicalSurface>( {
		{"none",		PhysicalSurface::None		},
		{"stone",		PhysicalSurface::Stone		},
		{"metal",		PhysicalSurface::Metal		},
		{"wood",		PhysicalSurface::Wood		},
		{"concrete",	PhysicalSurface::Concrete	},
		{"brick",       PhysicalSurface::Brick		},
		{"sand",		PhysicalSurface::Sand		},
		{"dirt",		PhysicalSurface::Dirt		},
		{"gravel",		PhysicalSurface::Gravel		},
		{"grass",		PhysicalSurface::Grass		},
		{"forest",		PhysicalSurface::Forest		},
		{"rock",		PhysicalSurface::Rock		},
		{"water",		PhysicalSurface::Water		},
		{"mud",			PhysicalSurface::Mud		},
		{"flesh",		PhysicalSurface::Flesh		},
		{"body",		PhysicalSurface::Body		},
		{"user16",		PhysicalSurface::User16		},
		{"user17",		PhysicalSurface::User17		},
		{"user18",		PhysicalSurface::User18		},
		{"user19",		PhysicalSurface::User19		},
		{"user20",		PhysicalSurface::User20		},
		{"user21",		PhysicalSurface::User21		},
		{"user22",		PhysicalSurface::User22		},
		{"user23",		PhysicalSurface::User23		},
		{"user24",		PhysicalSurface::User24		},
		{"user25",		PhysicalSurface::User25		},
		{"user26",		PhysicalSurface::User26		},
		{"user27",		PhysicalSurface::User27		},
		{"user28",		PhysicalSurface::User28		},
		{"user29",		PhysicalSurface::User29		},
		{"user30",		PhysicalSurface::User30		},
		{"user31",		PhysicalSurface::User31		},
		{"user32",		PhysicalSurface::User32		},
		{"user33",		PhysicalSurface::User33		},
		{"user34",		PhysicalSurface::User34		},
		{"user35",		PhysicalSurface::User35		},
		{"user36",		PhysicalSurface::User36		},
		{"user37",		PhysicalSurface::User37		},
		{"user38",		PhysicalSurface::User38		},
		{"user39",		PhysicalSurface::User39		},
		{"user40",		PhysicalSurface::User40		},
		{"user41",		PhysicalSurface::User41		},
		{"user42",		PhysicalSurface::User42		},
		{"user43",		PhysicalSurface::User43		},
		{"user44",		PhysicalSurface::User44		},
		{"user45",		PhysicalSurface::User45		},
		{"user46",		PhysicalSurface::User46		},
		{"user47",		PhysicalSurface::User47		},
		{"user48",		PhysicalSurface::User48		},
		{"user49",		PhysicalSurface::User49		},
		{"user50",		PhysicalSurface::User50		},
		{"user51",		PhysicalSurface::User51		},
		{"user52",		PhysicalSurface::User52		},
		{"user53",		PhysicalSurface::User53		},
		{"user54",		PhysicalSurface::User54		},
		{"user55",		PhysicalSurface::User55		},
		{"user56",		PhysicalSurface::User56		},
		{"user57",		PhysicalSurface::User57		},
		{"user58",		PhysicalSurface::User58		},
		{"user59",		PhysicalSurface::User59		},
		{"user60",		PhysicalSurface::User60		},
		{"user61",		PhysicalSurface::User61		},
		{"user62",		PhysicalSurface::User62		},
		{"user63",		PhysicalSurface::User63		},
		{"user64",		PhysicalSurface::User64		},
		{"user65",		PhysicalSurface::User65		},
		{"user66",		PhysicalSurface::User66		},
		{"user67",		PhysicalSurface::User67		},
		{"user68",		PhysicalSurface::User68		},
		{"user69",		PhysicalSurface::User69		},
		{"user70",		PhysicalSurface::User70		},
		{"user71",		PhysicalSurface::User71		},
		{"user72",		PhysicalSurface::User72		},
		{"user73",		PhysicalSurface::User73		},
		{"user74",		PhysicalSurface::User74		},
		{"user75",		PhysicalSurface::User75		},
		{"user76",		PhysicalSurface::User76		},
		{"user77",		PhysicalSurface::User77		},
		{"user78",		PhysicalSurface::User78		},
		{"user79",		PhysicalSurface::User79		},
		{"user80",		PhysicalSurface::User80		},
		{"user81",		PhysicalSurface::User81		},
		{"user82",		PhysicalSurface::User82		},
		{"user83",		PhysicalSurface::User83		},
		{"user84",		PhysicalSurface::User84		},
		{"user85",		PhysicalSurface::User85		},
		{"user86",		PhysicalSurface::User86		},
		{"user87",		PhysicalSurface::User87		},
		{"user88",		PhysicalSurface::User88		},
		{"user89",		PhysicalSurface::User89		},
		{"user90",		PhysicalSurface::User90		},
		{"user91",		PhysicalSurface::User91		},
		{"user92",		PhysicalSurface::User92		},
		{"user93",		PhysicalSurface::User93		},
		{"user94",		PhysicalSurface::User94		},
		{"user95",		PhysicalSurface::User95		},
		{"user96",		PhysicalSurface::User96		},
		{"user97",		PhysicalSurface::User97		},
		{"user98",		PhysicalSurface::User98		},
		{"user99",		PhysicalSurface::User99		},
		{"user100",		PhysicalSurface::User100	},
		{"user101",		PhysicalSurface::User101	},
		{"user102",		PhysicalSurface::User102	},
		{"user103",		PhysicalSurface::User103	},
		{"user104",		PhysicalSurface::User104	},
		{"user105",		PhysicalSurface::User105	},
		{"user106",		PhysicalSurface::User106	},
		{"user107",		PhysicalSurface::User107	},
		{"user108",		PhysicalSurface::User108	},
		{"user109",		PhysicalSurface::User109	},
		{"user110",		PhysicalSurface::User110	},
		{"user111",		PhysicalSurface::User111	},
		{"user112",		PhysicalSurface::User112	},
		{"user113",		PhysicalSurface::User113	},
		{"user114",		PhysicalSurface::User114	},
		{"user115",		PhysicalSurface::User115	},
		{"user116",		PhysicalSurface::User116	},
		{"user117",		PhysicalSurface::User117	},
		{"user118",		PhysicalSurface::User118	},
		{"user119",		PhysicalSurface::User119	},
		{"user120",		PhysicalSurface::User120	},
		{"user121",		PhysicalSurface::User121	},
		{"user122",		PhysicalSurface::User122	},
		{"user123",		PhysicalSurface::User123	},
		{"user124",		PhysicalSurface::User124	},
		{"user125",		PhysicalSurface::User125	},
		{"user126",		PhysicalSurface::User126	},
		{"user127",		PhysicalSurface::User127	},
		{"user128",		PhysicalSurface::User128	},
		{"user129",		PhysicalSurface::User129	},
		{"user130",		PhysicalSurface::User130	},
		{"user131",		PhysicalSurface::User131	},
		{"user132",		PhysicalSurface::User132	},
		{"user133",		PhysicalSurface::User133	},
		{"user134",		PhysicalSurface::User134	},
		{"user135",		PhysicalSurface::User135	},
		{"user136",		PhysicalSurface::User136	},
		{"user137",		PhysicalSurface::User137	},
		{"user138",		PhysicalSurface::User138	},
		{"user139",		PhysicalSurface::User139	},
		{"user140",		PhysicalSurface::User140	},
		{"user141",		PhysicalSurface::User141	},
		{"user142",		PhysicalSurface::User142	},
		{"user143",		PhysicalSurface::User143	},
		{"user144",		PhysicalSurface::User144	},
		{"user145",		PhysicalSurface::User145	},
		{"user146",		PhysicalSurface::User146	},
		{"user147",		PhysicalSurface::User147	},
		{"user148",		PhysicalSurface::User148	},
		{"user149",		PhysicalSurface::User149	},
		{"user150",		PhysicalSurface::User150	},
		{"user151",		PhysicalSurface::User151	},
		{"user152",		PhysicalSurface::User152	},
		{"user153",		PhysicalSurface::User153	},
		{"user154",		PhysicalSurface::User154	},
		{"user155",		PhysicalSurface::User155	},
		{"user156",		PhysicalSurface::User156	},
		{"user157",		PhysicalSurface::User157	},
		{"user158",		PhysicalSurface::User158	},
		{"user159",		PhysicalSurface::User159	},
		{"user160",		PhysicalSurface::User160	},
		{"user161",		PhysicalSurface::User161	},
		{"user162",		PhysicalSurface::User162	},
		{"user163",		PhysicalSurface::User163	},
		{"user164",		PhysicalSurface::User164	},
		{"user165",		PhysicalSurface::User165	},
		{"user166",		PhysicalSurface::User166	},
		{"user167",		PhysicalSurface::User167	},
		{"user168",		PhysicalSurface::User168	},
		{"user169",		PhysicalSurface::User169	},
		{"user170",		PhysicalSurface::User170	},
		{"user171",		PhysicalSurface::User171	},
		{"user172",		PhysicalSurface::User172	},
		{"user173",		PhysicalSurface::User173	},
		{"user174",		PhysicalSurface::User174	},
		{"user175",		PhysicalSurface::User175	},
		{"user176",		PhysicalSurface::User176	},
		{"user177",		PhysicalSurface::User177	},
		{"user178",		PhysicalSurface::User178	},
		{"user179",		PhysicalSurface::User179	},
		{"user180",		PhysicalSurface::User180	},
		{"user181",		PhysicalSurface::User181	},
		{"user182",		PhysicalSurface::User182	},
		{"user183",		PhysicalSurface::User183	},
		{"user184",		PhysicalSurface::User184	},
		{"user185",		PhysicalSurface::User185	},
		{"user186",		PhysicalSurface::User186	},
		{"user187",		PhysicalSurface::User187	},
		{"user188",		PhysicalSurface::User188	},
		{"user189",		PhysicalSurface::User189	},
		{"user190",		PhysicalSurface::User190	},
		{"user191",		PhysicalSurface::User191	},
		{"user192",		PhysicalSurface::User192	},
		{"user193",		PhysicalSurface::User193	},
		{"user194",		PhysicalSurface::User194	},
		{"user195",		PhysicalSurface::User195	},
		{"user196",		PhysicalSurface::User196	},
		{"user197",		PhysicalSurface::User197	},
		{"user198",		PhysicalSurface::User198	},
		{"user199",		PhysicalSurface::User199	},
		{"user200",		PhysicalSurface::User200	},
		{"user201",		PhysicalSurface::User201	},
		{"user202",		PhysicalSurface::User202	},
		{"user203",		PhysicalSurface::User203	},
		{"user204",		PhysicalSurface::User204	},
		{"user205",		PhysicalSurface::User205	},
		{"user206",		PhysicalSurface::User206	},
		{"user207",		PhysicalSurface::User207	},
		{"user208",		PhysicalSurface::User208	},
		{"user209",		PhysicalSurface::User209	},
		{"user210",		PhysicalSurface::User210	},
		{"user211",		PhysicalSurface::User211	},
		{"user212",		PhysicalSurface::User212	},
		{"user213",		PhysicalSurface::User213	},
		{"user214",		PhysicalSurface::User214	},
		{"user215",		PhysicalSurface::User215	},
		{"user216",		PhysicalSurface::User216	},
		{"user217",		PhysicalSurface::User217	},
		{"user218",		PhysicalSurface::User218	},
		{"user219",		PhysicalSurface::User219	},
		{"user220",		PhysicalSurface::User220	},
		{"user221",		PhysicalSurface::User221	},
		{"user222",		PhysicalSurface::User222	},
		{"user223",		PhysicalSurface::User223	},
		{"user224",		PhysicalSurface::User224	},
		{"user225",		PhysicalSurface::User225	},
		{"user226",		PhysicalSurface::User226	},
		{"user227",		PhysicalSurface::User227	},
		{"user228",		PhysicalSurface::User228	},
		{"user229",		PhysicalSurface::User229	},
		{"user230",		PhysicalSurface::User230	},
		{"user231",		PhysicalSurface::User231	},
		{"user232",		PhysicalSurface::User232	},
		{"user233",		PhysicalSurface::User233	},
		{"user234",		PhysicalSurface::User234	},
		{"user235",		PhysicalSurface::User235	},
		{"user236",		PhysicalSurface::User236	},
		{"user237",		PhysicalSurface::User237	},
		{"user238",		PhysicalSurface::User238	},
		{"user239",		PhysicalSurface::User239	},
		{"user240",		PhysicalSurface::User240	},
		{"user241",		PhysicalSurface::User241	},
		{"user242",		PhysicalSurface::User242	},
		{"user243",		PhysicalSurface::User243	},
		{"user244",		PhysicalSurface::User244	},
		{"user245",		PhysicalSurface::User245	},
		{"user246",		PhysicalSurface::User246	},
		{"user247",		PhysicalSurface::User247	},
		{"user248",		PhysicalSurface::User248	},
		{"user249",		PhysicalSurface::User249	},
		{"user250",		PhysicalSurface::User250	},
		{"user251",		PhysicalSurface::User251	},
		{"user252",		PhysicalSurface::User252	},
		{"user253",		PhysicalSurface::User253	},
		{"user254",		PhysicalSurface::User254	}
	}
);

PhysicalSurface StringToPhysicalSurface( const std::string& From )
{
	return ConvertSurfaceString.To( From );
}

std::string PhysicalSurfaceToString( const PhysicalSurface From )
{
	return ConvertSurfaceString.From( From );
}

Material ConfigureMaterial( const JSON::Container& Container )
{
	Material Material;

	JSON::Assign( Container.Tree, "name", Material.Name );
	JSON::Assign( Container.Tree, "shader", Material.Shader );

	// Check if the material wants to be double sided.
	JSON::Assign( Container.Tree, "twoside", Material.DoubleSided );
	JSON::Assign( Container.Tree, "doublesided", Material.DoubleSided );

	if( auto* Object = JSON::Find( Container.Tree, "surface" ) )
	{
		Material.Surface = StringToPhysicalSurface( Object->Value );
	}

	if( auto* Object = JSON::Find( Container.Tree, "textures" ) )
	{
		for( auto* Texture : Object->Objects )
		{
			Material.Textures.emplace_back();
			Material.Textures.back() = Texture->Key;
		}
	}

	if( auto* Object = JSON::Find( Container.Tree, "uniforms" ) )
	{
		for( auto* Uniform : Object->Objects )
		{
			// TODO: Figure out loading uniforms.
			Vector4D Temporary = {};
			const auto Count = Extract( Uniform->Value, Temporary );

			Material.Uniforms.emplace_back();
			Material.Uniforms.back().first = Uniform->Key;

			// Pick the uniform constructor based on the amount of components that were retrieved.
			switch( Count )
			{
			case 1: // float
				Material.Uniforms.back().second.Set( Temporary.X );
				break;
			case 2: // vec3 (with zero for Z)
				Material.Uniforms.back().second.Set( Vector3D( Temporary.X, Temporary.Y, 0.0 ) );
				break;
			case 3: // vec3
				Material.Uniforms.back().second.Set( Vector3D( Temporary.X, Temporary.Y, Temporary.Z ) );
				break;
			default: // assume vec4
				Material.Uniforms.back().second.Set( Temporary );
				break;
			}
		}
	}

	return Material;
}

Material LoadMaterial( const std::string& Location )
{
	CFile File( Location );
	if( !File.Exists() )
		return Material();

	// Load any asset entries that are specified in the material file.
	CAssets::Load( Location );

	// Set up the material itself.
	File.Load();
	auto Tree = JSON::Tree( File );
	return ConfigureMaterial( Tree );
}

void MaterialAsset::Reload()
{
	if( Location.empty() )
		return;

	Material = LoadMaterial( Location );
}
