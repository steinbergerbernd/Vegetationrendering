#include "GeneralShaderFunctions.hlsl"

Texture2D SurfaceTexture;

struct VertexShaderInput
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float4 color : COLOR0;
	float2 texCoord : TEXCOORD0;
	float3 instancePosition : TEXCOORD1;
};

struct VertexShaderOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 texCoord : TEXCOORD0;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 worldPos = float4(input.position + input.instancePosition, 1);

	worldPos = mul(worldPos, World);
	output.position = mul(worldPos, ViewProjection);
	output.color = input.color;
	output.texCoord = input.texCoord;

	return output;
}


float4 PixelShaderFunction(VertexShaderOutput input) : SV_TARGET
{
	return SurfaceTexture.Sample(LinearSampler, input.texCoord);
	//return input.color;
}


technique11 Default
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_5_0,VertexShaderFunction()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0,PixelShaderFunction()));
	}
}