#include "GeneralShaderFunctions.hlsl"

static const float scaleFactor = 4.5f;

Texture2D TrunkTexture;
Texture2D BumpMapTexture;
Texture2D NormalMapTexture;
Texture2D BillboardFront;
Texture2D BillboardSide;
Texture2D BillboardTop;

float EdgeFactor;
float InsideFactor;

float MinDistance;
float MaxDistance;

float BoundingBoxCenter[3];
float BoundingBoxExtents[3];
bool GSCulling = true;

bool ShowSavedCulling = false;
float4x4 SavedViewProjection;

struct VertexShaderInput
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 instancePosition : TEXCOORD1;
};


struct VertexShaderOutput
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 instancePosition : TEXCOORD1;
	float distanceFactor : DISTANCEFACTOR;
};


struct HSConstantOutput
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};


struct HSControlPointOutput
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
};


struct DomainShaderOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 viewDirection : TEXCOORD1;
};


VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float3 worldPosition = mul(float4(input.position,1), World).xyz + input.instancePosition;
	output.position = worldPosition;
	output.texCoord = input.texCoord;
	output.normal = normalize(mul(input.normal, (float3x3)World));
	output.instancePosition = input.instancePosition;

	// for distance adaptive tessellation
	float distanceToCamera = distance(worldPosition, CameraPosition);
	output.distanceFactor = 1.0f - clamp((distanceToCamera - MinDistance) / (MaxDistance - MinDistance), 0.0f, 1.0f);

	return output;
}


HSConstantOutput HSConstantFunction(InputPatch<VertexShaderOutput, 3> patch, uint patchID : SV_PrimitiveID)
{
	HSConstantOutput output;
	float4 tessellationFactors = float4(0,0,0,0);

	bool inFrustum = true;
	if(GSCulling && (patch[0].distanceFactor + patch[1].distanceFactor + patch[2].distanceFactor) > 0.0f)
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
	{
		// distance adaptive tessellation
		tessellationFactors.x = patch[1].distanceFactor + patch[2].distanceFactor;
		tessellationFactors.y = patch[2].distanceFactor + patch[0].distanceFactor;
		tessellationFactors.z = patch[0].distanceFactor + patch[1].distanceFactor;
		tessellationFactors.w = patch[0].distanceFactor + patch[1].distanceFactor + patch[2].distanceFactor;
		float edgeFactor = EdgeFactor * 0.5f;
		float insideFactor = InsideFactor * 0.33f;
		tessellationFactors *= float4(edgeFactor, edgeFactor, edgeFactor, insideFactor);
	}

	if (GSCulling)
	{
		output.edges[0] = tessellationFactors.x;
		output.edges[1] = tessellationFactors.y;
		output.edges[2] = tessellationFactors.z;
		output.inside = tessellationFactors.w;
	}
	else
	{
		output.edges[0] = max(tessellationFactors.x, 1.0f);
		output.edges[1] = max(tessellationFactors.y, 1.0f);
		output.edges[2] = max(tessellationFactors.z, 1.0f);
		output.inside = max(tessellationFactors.w, 1.0f);
	}

	return output;
}


[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSConstantFunction")]
HSControlPointOutput HSControlPointFunction(InputPatch<VertexShaderOutput, 3> patch, uint controlPointID : SV_OutputControlPointID)
{
	HSControlPointOutput output;

	output.position = patch[controlPointID].position;
	output.normal = patch[controlPointID].normal;
	output.texCoord = patch[controlPointID].texCoord;

	return output;
}


[domain("tri")]
DomainShaderOutput DomainShaderFunction(HSConstantOutput input, float3 barycentricCoordinates : SV_DomainLocation,
	const OutputPatch<HSControlPointOutput, 3> patch)
{
	DomainShaderOutput output;

	float3 worldPosition =
		barycentricCoordinates.x * patch[0].position +
		barycentricCoordinates.y * patch[1].position +
		barycentricCoordinates.z * patch[2].position;

	float3 normal =
		barycentricCoordinates.x * patch[0].normal +
		barycentricCoordinates.y * patch[1].normal +
		barycentricCoordinates.z * patch[2].normal;
	normal = normalize(normal);

	float2 texCoord =
		barycentricCoordinates.x * patch[0].texCoord +
		barycentricCoordinates.y * patch[1].texCoord +
		barycentricCoordinates.z * patch[2].texCoord;

	//float mipLevel = clamp((distance(worldPosition, CameraPosition) - 100.0f) / 100.0f, 0.0f, 3.0f);
	float4 bumpMap = BumpMapTexture.SampleLevel(LinearSampler, texCoord, 0);
	//float3 normalMap = normalize(NormalMapTexture.SampleLevel(LinearSampler, texCoord, 0).xyz * 2 - 1);
	worldPosition += normal * bumpMap.r * 0.1f * (1.0f / max(worldPosition.y, 1.0f));

	output.position = mul(float4(worldPosition,1), ViewProjection);
	output.texCoord = texCoord;
	output.normal = normal;
	output.viewDirection = CameraPosition - worldPosition;

	return output;
}


float4 PixelShaderFunction(DomainShaderOutput input) : SV_Target
{
	float3 viewDirection = normalize(input.viewDirection);
	float3 lightDirection = normalize(-LightVector);
	float3 normal = normalize(input.normal);

	float4 diffuseLight = CalculateDiffuseLight(lightDirection, normal);
	float4 specularLight = CalculateSpecularLight(LightVector, normal, viewDirection);
	float selfShadow = saturate(4 * diffuseLight.r);

	float4 color = float4(AmbientLight,1) + selfShadow * (diffuseLight + specularLight);
	color = saturate(color * TrunkTexture.Sample(LinearSampler, input.texCoord));

	//return float4(normal,1);
	return color;
}


struct VSBillboardInput
{
	float3 position : POSITION0;
};


struct VSBillboardOutput
{
	float3 position : POSITION0;
};


struct GSBillboardOutput
{
	float4 position : SV_Position;
	float3 normal : NORMAL0;
	float3 texCoord : TEXCOORD0;
};


VSBillboardOutput VSBillboardFunction(VSBillboardInput input)
{
	VSBillboardOutput output;

	output.position = input.position;

	return output;
}


[maxvertexcount(8)]
void GSBillboardFunction(point VSBillboardOutput input[1], inout TriangleStream<GSBillboardOutput> outputStream)
{
	GSBillboardOutput output;

	//output.normal = float3(0,1,0);
	//output.position = mul(mul(float4(-0.5f, 0, 0.5f, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	//output.texCoord = float3(0,0,0);
	//outputStream.Append(output);
	//output.position = mul(mul(float4(0.5f, 0, 0.5f, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	//output.texCoord = float3(1,0,0);
	//outputStream.Append(output);
	//output.position = mul(mul(float4(-0.5f, 0, -0.5f, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	//output.texCoord = float3(0,1,0);
	//outputStream.Append(output);
	//output.position = mul(mul(float4(0.5f, 0, -0.5f, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	//output.texCoord = float3(1,1,0);
	//outputStream.Append(output);
	//outputStream.RestartStrip();

	output.normal = float3(0,-1,0);
	output.position = mul(mul(float4(-0.5f, 0.5f, 0, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	output.texCoord = float3(0,0,1);
	outputStream.Append(output);
	output.position = mul(mul(float4(0.5f, 0.5f, 0, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	output.texCoord = float3(1,0,1);
	outputStream.Append(output);
	output.position = mul(mul(float4(-0.5f, -0.5f, 0, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	output.texCoord = float3(0,1,1);
	outputStream.Append(output);
	output.position = mul(mul(float4(0.5f, -0.5f, 0, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	output.texCoord = float3(1,1,1);
	outputStream.Append(output);
	outputStream.RestartStrip();

	output.normal = float3(1,0,0);
	output.position = mul(mul(float4(0, 0.5f, -0.5f, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	output.texCoord = float3(0,0,2);
	outputStream.Append(output);
	output.position = mul(mul(float4(0, 0.5f, 0.5f, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	output.texCoord = float3(1,0,2);
	outputStream.Append(output);
	output.position = mul(mul(float4(0, -0.5f, -0.5f, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	output.texCoord = float3(0,1,2);
	outputStream.Append(output);
	output.position = mul(mul(float4(0, -0.5f, 0.5f, 1), World) + float4(input[0].position,0) + float4(0, scaleFactor, 0, 0), ViewProjection);
	output.texCoord = float3(1,1,2);
	outputStream.Append(output);
	outputStream.RestartStrip();
}


float4 PSBillboardFunction(GSBillboardOutput input) : SV_Target
{
	float4 color;
	
	if(input.texCoord.z == 0)
		color = BillboardTop.Sample(LinearSampler, input.texCoord.xy);
	else if(input.texCoord.z == 1)
		color = BillboardFront.Sample(LinearSampler, input.texCoord.xy);
	else
		color = BillboardSide.Sample(LinearSampler, input.texCoord.xy);

	clip(color.a - 0.1f);

	return color;
}


RasterizerState CullNone
{
	CullMode=NONE;
};

technique11 TrunkLOD1
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_5_0, VSBillboardFunction()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(CompileShader(gs_5_0, GSBillboardFunction()));
		SetPixelShader(CompileShader(ps_5_0, PSBillboardFunction()));

		//SetRasterizerState(CullNone);
	}
}


technique11 Trunk
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_5_0, VertexShaderFunction()));
		SetHullShader(CompileShader(hs_5_0, HSControlPointFunction()));
		SetDomainShader(CompileShader(ds_5_0, DomainShaderFunction()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PixelShaderFunction()));
	}
}