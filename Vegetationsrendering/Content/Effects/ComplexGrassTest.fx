#include "GeneralShaderFunctions.hlsl"

// *******
// Globals
// *******

static const int VertexCount = 56;
static const int IndexCount = 56;

Texture2D GrassLeafTexture;

int GeoIndices[IndexCount];

float4 GeoPositions[VertexCount];
float GeoNormals[VertexCount * 3];
float GeoTexCoords[VertexCount * 2];

float MinDistance;
float MaxDistance;
float TessellationFactor;

float BoundingBoxCenter[3];
float BoundingBoxExtents[3];

bool GSCulling = true;

bool ShowSavedCulling = false;
float4x4 SavedViewProjection;

// ************************************
// Grass Rendering with Geometry Shader
// ************************************

struct VertexShaderInput
{
	float3 position : POSITION0;
};

struct VertexShaderOutput
{
	float3 position : POSITION0;
};

struct GeometryShaderOutput
{
	float4 position : SV_POSITION;
	float3 viewDirection : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
	uint index : INDEX;
};


VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	output.position = input.position;

	return output;
}

[maxvertexcount(56)]
void GeometryShaderFunction(point VertexShaderOutput input[1], inout TriangleStream<GeometryShaderOutput> grassStream)
{
	GeometryShaderOutput output;

	bool inFrustum = true;
	int i, j;

	if(GSCulling)
	{
		float4 boundingBox[8];
		float3 boundingBoxCenter = float3(BoundingBoxCenter[0], BoundingBoxCenter[1], BoundingBoxCenter[2]);
		float3 boundingBoxExtents = float3(BoundingBoxExtents[0], BoundingBoxExtents[1], BoundingBoxExtents[2]);
		float4x4 cullingViewProjection = ShowSavedCulling ? SavedViewProjection : ViewProjection;

		boundingBox[0] = mul(float4(input[0].position + boundingBoxCenter +
			boundingBoxExtents, 1), cullingViewProjection);
		boundingBox[1] = mul(float4(input[0].position + boundingBoxCenter +
			boundingBoxExtents * float3(-1,1,1), 1), cullingViewProjection);
		boundingBox[2] = mul(float4(input[0].position + boundingBoxCenter +
			boundingBoxExtents * float3(1,-1,1), 1), cullingViewProjection);
		boundingBox[3] = mul(float4(input[0].position + boundingBoxCenter +
			boundingBoxExtents * float3(-1,-1,1), 1), cullingViewProjection);
		boundingBox[4] = mul(float4(input[0].position + boundingBoxCenter +
			boundingBoxExtents * float3(1,1,-1), 1), cullingViewProjection);
		boundingBox[5] = mul(float4(input[0].position + boundingBoxCenter +
			boundingBoxExtents * float3(-1,1,-1), 1), cullingViewProjection);
		boundingBox[6] = mul(float4(input[0].position + boundingBoxCenter +
			boundingBoxExtents * float3(1,-1,-1), 1), cullingViewProjection);
		boundingBox[7] = mul(float4(input[0].position + boundingBoxCenter +
			boundingBoxExtents * float3(-1,-1,-1), 1), cullingViewProjection);

		int outOfFrustum[6] = { 0, 0, 0, 0, 0, 0 };
		for(i = 0; i < 8; ++i)
		{
			if(boundingBox[i].x > boundingBox[i].w) ++outOfFrustum[0];
			if(boundingBox[i].x < -boundingBox[i].w) ++outOfFrustum[1];
			if(boundingBox[i].y > boundingBox[i].w) ++outOfFrustum[2];
			if(boundingBox[i].y < -boundingBox[i].w) ++outOfFrustum[3];
			if(boundingBox[i].z > boundingBox[i].w) ++outOfFrustum[4];
			if(boundingBox[i].z < -boundingBox[i].w) ++outOfFrustum[5];
		}

		for(i = 0; i < 6 && inFrustum; ++i)
			inFrustum = outOfFrustum[i] != 8;
	}

	if(inFrustum)
	{
		for(i = 0; i < 14; ++i)
		{
			for(j = 0; j < 4; ++j)
			{
				uint index = GeoIndices[i*4 + j];
			
				float4 worldPosition = mul(GeoPositions[index],World) + float4(input[0].position,0);
				output.position = mul(worldPosition, ViewProjection);
				output.texCoord = float2(GeoTexCoords[index*2], GeoTexCoords[index*2+1]);
				output.index = index;
				output.viewDirection = CameraPosition - worldPosition.xyz;

				grassStream.Append(output);
			}

			grassStream.RestartStrip();
		}
	}
}

float4 PixelShaderFunction(GeometryShaderOutput input) : SV_Target
{
	float3 viewDirection = normalize(input.viewDirection);
	float3 lightDirection = normalize(-LightVector);
	int index = input.index * 3;
	float3 normal = normalize(float3(GeoNormals[index], GeoNormals[index+1], GeoNormals[index+2]));

	float4 diffuseLight = CalculateDiffuseLight(lightDirection, normal);
	float4 specularLight = CalculateSpecularLight(LightVector, normal, viewDirection);
	float selfShadow = saturate(4 * diffuseLight.r);

	index = input.index * 2;
	float4 color = GrassLeafTexture.Sample(AnisotropicSampler, input.texCoord);
	clip(color.a - 0.9f);

	color = saturate(color * (float4(AmbientLight,1) + selfShadow * (diffuseLight + specularLight)));

	return color;
}


// *********************************
// Grass Rendering with Tessellation
// *********************************

struct VSTessInput
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 instancePosition : TEXCOORD1;
};

struct VSTessOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 viewDirection : TEXCOORD1;
};

VSTessOutput VSTessFunction(VSTessInput input)
{
	VSTessOutput output;

	float4 worldPosition = mul(float4(input.position,1),World) + float4(input.instancePosition,0);
	output.position = mul(worldPosition, ViewProjection);
	output.texCoord = input.texCoord;
	output.normal = input.normal;
	output.viewDirection = CameraPosition - worldPosition.xyz;

	return output;
}

float4 PSTessFunction(VSTessOutput input) : SV_Target
{
	float3 viewDirection = normalize(input.viewDirection);
	float3 lightDirection = normalize(-LightVector);
	float3 normal = normalize(input.normal);

	float4 diffuseLight = CalculateDiffuseLight(lightDirection, normal);
	float4 specularLight = CalculateSpecularLight(LightVector, normal, viewDirection);
	float selfShadow = saturate(4 * diffuseLight.r);

	float4 color = GrassLeafTexture.Sample(AnisotropicSampler, input.texCoord);
	//clip(color.a - 0.9f);

	color = saturate(color * (float4(AmbientLight,1) + selfShadow * (diffuseLight + specularLight)));

	return color;
}


// **********
// Techniques
// **********

technique11 RenderGrassTess
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_5_0, VSTessFunction()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSTessFunction()));
	}
}

technique11 RenderGrassGS
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_5_0, VertexShaderFunction()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(CompileShader( gs_5_0, GeometryShaderFunction()));
		SetPixelShader(CompileShader(ps_5_0, PixelShaderFunction()));
	}  
}