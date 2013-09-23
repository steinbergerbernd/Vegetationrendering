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
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 instancePosition : TEXCOORD1;
	float distanceFactor : DISTANCEFACTOR;
};

struct HSTessConstantOutput
{
	float edges[4]  : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

struct HSTessOutput
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
};

struct DSTessOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 viewDirection : TEXCOORD1;
};

VSTessOutput VSTessFunction(VSTessInput input)
{
	VSTessOutput output;

	output.position = mul(float4(input.position,1),World).xyz + input.instancePosition;
	output.texCoord = input.texCoord;
	output.normal = input.normal;
	output.instancePosition = input.instancePosition;

	// for distance adaptive tessellation
	float distanceToCamera = distance(output.position, CameraPosition);
	output.distanceFactor = 1.0f - clamp((distanceToCamera - MinDistance) / (MaxDistance - MinDistance), 0.0f, 1.0f);

	return output;
}

HSTessConstantOutput HSTessConstantFunction(InputPatch<VSTessOutput, 4> patch, uint patchID : SV_PrimitiveID)
{
	HSTessConstantOutput output;
	
	float edge;
	float inside;

	bool inFrustum = true;
	if(GSCulling && patch[0].distanceFactor > 0.0f)
	{
		float4 boundingBox[8];
		float3 boundingBoxCenter = float3(BoundingBoxCenter[0], BoundingBoxCenter[1], BoundingBoxCenter[2]);
		float3 boundingBoxExtents = float3(BoundingBoxExtents[0], BoundingBoxExtents[1], BoundingBoxExtents[2]);
		float4x4 cullingViewProjection = ShowSavedCulling ? SavedViewProjection : ViewProjection;

		boundingBox[0] = mul(float4(patch[0].instancePosition + boundingBoxCenter +
			boundingBoxExtents, 1), cullingViewProjection);
		boundingBox[1] = mul(float4(patch[0].instancePosition + boundingBoxCenter +
			boundingBoxExtents * float3(-1,1,1), 1), cullingViewProjection);
		boundingBox[2] = mul(float4(patch[0].instancePosition + boundingBoxCenter +
			boundingBoxExtents * float3(1,-1,1), 1), cullingViewProjection);
		boundingBox[3] = mul(float4(patch[0].instancePosition + boundingBoxCenter +
			boundingBoxExtents * float3(-1,-1,1), 1), cullingViewProjection);
		boundingBox[4] = mul(float4(patch[0].instancePosition + boundingBoxCenter +
			boundingBoxExtents * float3(1,1,-1), 1), cullingViewProjection);
		boundingBox[5] = mul(float4(patch[0].instancePosition + boundingBoxCenter +
			boundingBoxExtents * float3(-1,1,-1), 1), cullingViewProjection);
		boundingBox[6] = mul(float4(patch[0].instancePosition + boundingBoxCenter +
			boundingBoxExtents * float3(1,-1,-1), 1), cullingViewProjection);
		boundingBox[7] = mul(float4(patch[0].instancePosition + boundingBoxCenter +
			boundingBoxExtents * float3(-1,-1,-1), 1), cullingViewProjection);

		int outOfFrustum[6] = { 0, 0, 0, 0, 0, 0 };
		int i;
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
		edge = inside = TessellationFactor * patch[0].distanceFactor;
	else
		edge = inside = 0;

	if (!GSCulling)
	{
		edge = max(edge, 1.0f);
		inside = max(inside, 1.0f);
	}

	output.edges[0] = edge;
	output.edges[1] = edge;
	output.edges[2] = edge;
	output.edges[3] = edge;

	output.inside[0] = inside;
	output.inside[1] = inside;

	return output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSTessConstantFunction")]
HSTessOutput HSTessFunction(InputPatch<VSTessOutput, 4> patch, uint controlPointID : SV_OutputControlPointID)
{
	HSTessOutput output;

	output.position = patch[controlPointID].position;
	output.normal = patch[controlPointID].normal;
	output.texCoord = patch[controlPointID].texCoord;

	return output;
}

[domain("quad")]
DSTessOutput DSTessFunction(HSTessConstantOutput input, float2 uv : SV_DomainLocation, const OutputPatch<HSTessOutput, 4> patch)
{
	DSTessOutput output;

	float3 topMidPos = lerp(patch[0].position, patch[1].position, uv.x);
	float3 bottomMidPos = lerp(patch[3].position, patch[2].position, uv.x);
	float4 worldPosition = float4(lerp(topMidPos, bottomMidPos, uv.y),1);

	//float3 topMidNormal = lerp(patch[0].normal, patch[1].normal, uv.x);
	//float3 bottomMidNormal = lerp(patch[3].normal, patch[2].normal, uv.x);
	//output.normal = normalize(lerp(topMidNormal, bottomMidNormal, uv.y));

	float distanceHorizontal = distance(patch[0].position, patch[1].position);
	float distanceVertical = distance(patch[0].position, patch[3].position);

	if(distanceHorizontal > distanceVertical)
	{
		if(patch[0].position.y > patch[1].position.y)
			output.texCoord = float2(-uv.x + 1, uv.y);
		else
			output.texCoord = float2(uv.x, -uv.y + 1);
	}
	else
	{
		if(patch[0].position.y > patch[3].position.y)
			output.texCoord = float2(uv.y, uv.x) * -1 + float2(1,1);
		else
			output.texCoord = float2(uv.y, uv.x);
	}

	output.texCoord.y = lerp(0.4f, 0.6f, output.texCoord.y);

	float factor = (abs(output.texCoord.x * 2 - 1) * -1 + 1) * 0.5f;// * 0.1f;
	float3 tangent = normalize(patch[1].position - patch[0].position);
	float3 bitangent = normalize(patch[3].position - patch[0].position);
	float3 normal = normalize(cross(tangent, bitangent));
	if (normal.y < 0)
		normal *= -1;
	output.normal = normal;
	worldPosition += float4(normal,0) * factor;

	factor = abs(output.texCoord.y - 0.5f) * 6.0f;//0.2f;
	worldPosition += float4(-normal,0) * factor;

	output.position = mul(worldPosition, ViewProjection);

	output.viewDirection = CameraPosition - worldPosition.xyz;

	return output;
}

float4 PSTessFunction(DSTessOutput input) : SV_Target
{
	float3 viewDirection = normalize(input.viewDirection);
	float3 lightDirection = normalize(-LightVector);
	float3 normal = normalize(input.normal);

	float4 diffuseLight = CalculateDiffuseLight(lightDirection, normal);
	float4 specularLight = CalculateSpecularLight(LightVector, normal, viewDirection);
	float selfShadow = saturate(4 * diffuseLight.r);

	float4 color = GrassLeafTexture.Sample(AnisotropicSampler, input.texCoord);
	clip(color.a - 0.9f);

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
		SetHullShader(CompileShader(hs_5_0, HSTessFunction()));
		SetDomainShader(CompileShader(ds_5_0, DSTessFunction()));
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