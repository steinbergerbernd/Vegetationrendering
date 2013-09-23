// Grass Rendering with Geometry Shader

#include "GeneralShaderFunctions.hlsl"

uint GrassCoverage;
float GrassWidth;
float GrassHeight;
float GrassMessiness;

float3 GrassPositions[4] =
{
	float3( -1, 0, 0 ),
	float3( -1, 2, 0 ),
	float3( 1, 0, 0 ),
	float3( 1, 2, 0 )
};

float2 GrassTexCoords[4] = 
{ 
	float2(0,1), 
	float2(0,0),
	float2(1,1),
	float2(1,0)
};

// textures
Texture2DArray GrassTextureArray;
Texture1D RandomTexture;

// in- and output structures

struct VSInput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD0;
	uint VertexID : SV_VertexID;
};

struct VSOutput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
	uint VertexID : VERTID;
};

struct PSInput
{
	float4 Position : SV_Position;
	float3 TexCoord : TEXCOORD0;
	float4 Color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Midpoint of the three vertices A,B,C
//--------------------------------------------------------------------------------------
VSOutput CalcMidPoint(VSOutput a, VSOutput b, VSOutput c)
{
	VSOutput midPoint;
	
	midPoint.Position = (a.Position + b.Position + c.Position)/3.0f;
	midPoint.Normal = (a.Normal + b.Normal + c.Normal)/3.0f;
	midPoint.TexCoord = (a.TexCoord + b.TexCoord + c.TexCoord)/3.0f;
	midPoint.VertexID = a.VertexID + b.VertexID + c.VertexID;
	
	return midPoint;
}

float3 RandomDir(float offset)
{   
	float texCoord = (offset) / 300.0;
	return RandomTexture.SampleLevel(PointSampler, texCoord, 0);
}

float4x4 GetRandomOrientation(float3 position, float3 normal, float randomOffset)
{
	float3 tangent = RandomDir(randomOffset);
	
	float3 bitangent = normalize(cross(tangent, normal));
	tangent = normalize(cross(bitangent, normal));
	
	float4x4 world = { float4(tangent, 0), float4(normal, 0), float4(bitangent, 0), float4(position, 1) };
	return world;
}


VSOutput VSGrass(VSInput input)
{
	// simple transform into the instance space
	VSOutput output;
	output.Position = mul(input.Position, (float3x3)World);
	output.Normal = mul(input.Normal, (float3x3)World);
	output.TexCoord = input.TexCoord;
	output.VertexID = input.VertexID;
	
	return output;
}

void OutputGrassBlade(VSOutput midPoint, inout TriangleStream<PSInput> grassStream, int grassTexture)
{
	PSInput output;
	
	float4x4 world = GetRandomOrientation(midPoint.Position, midPoint.Normal, (float)midPoint.VertexID);
	
	for(int i=0; i<4; i++)
	{
		float3 position = GrassPositions[i];
		position.x *= GrassWidth;
		position.y *= GrassHeight;
		
		output.Position = mul(float4(position,1), world);
		output.Position = mul(output.Position, ViewProjection);
		output.TexCoord = float3(GrassTexCoords[i], grassTexture);
		output.Color = float4(1,1,1,1);
	
		grassStream.Append(output);
	}
	
	grassStream.RestartStrip();
}

[maxvertexcount(90)]
void GSGrass(triangle VSOutput input[3], inout TriangleStream<PSInput> grassStream )
{
	VSOutput midPoint = CalcMidPoint(input[0], input[1], input[2]);
	
	float4 coverageMask = GrassTextureArray.SampleLevel(PointSampler, float3(midPoint.TexCoord,3), 0);
	float cm[4];
	cm[0] = coverageMask.r;
	cm[1] = coverageMask.g;
	cm[2] = coverageMask.b;
	cm[3] = coverageMask.a;
	
	for(int i=0; i<4; i++)
	{
		float maxBlades = float(GrassCoverage)*cm[i];
		for(float j=0; j<maxBlades; j++)
		{	
			float randOffset = i*5 + (j+1);
			float3 tangent = RandomDir(midPoint.Position.x + randOffset);
			float3 len = normalize(RandomDir( midPoint.Position.z + randOffset));
			float3 shift = len.x * GrassMessiness*normalize(cross(tangent, midPoint.Normal));
			VSOutput grassPoint = midPoint;
			grassPoint.VertexID += randOffset;
			grassPoint.Position += shift; 
				
			//uncomment this to make the grass strictly conform to the mesh
			//if(IsInTriangle(grassPoint.Position, input[0].Position, input[1].Position, input[2].Position))
			{
				OutputGrassBlade(grassPoint, grassStream, i%3);
			}
		}
	}
}

float4 PSGrass(PSInput input) : SV_Target
{
	float4 color = GrassTextureArray.Sample(AnisotropicSampler, input.TexCoord);
	color.xyz *= input.Color.xyz;
	return color;
}


technique11 RenderGrass
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_5_0, VSGrass()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(CompileShader( gs_5_0, GSGrass()));
		SetPixelShader(CompileShader(ps_5_0, PSGrass()));
	}  
}