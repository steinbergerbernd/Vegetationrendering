#include "GeneralShaderFunctions.hlsl"

Texture2D ModelTexture;

struct VertexShaderInput
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
};


struct VertexShaderOutput
{
	float4 position : SV_Position;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 viewDirection : TEXCOORD1;
};


VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 worldPosition = mul(float4(input.position,1), World);
	output.position = mul(worldPosition, ViewProjection);
	output.texCoord = input.texCoord;
	output.normal = normalize(mul(input.normal, (float3x3)World));
	output.viewDirection = CameraPosition - worldPosition.xyz;

	return output;
}


float4 PixelShaderFunction(VertexShaderOutput input) : SV_Target
{
	float3 viewDirection = normalize(input.viewDirection);
	float3 lightDirection = normalize(-LightVector);
	float3 normal = normalize(input.normal);

	float4 diffuseLight = CalculateDiffuseLight(lightDirection, normal);
	float4 specularLight = CalculateSpecularLight(LightVector, normal, viewDirection);
	float selfShadow = saturate(4 * diffuseLight.r);

	float4 color = ModelTexture.Sample(AnisotropicSampler, input.texCoord);
	clip(color.a - 0.9f);

	color = saturate(color * (float4(AmbientLight,1) + selfShadow * (diffuseLight + specularLight)));

	return color;
}


technique11 RenderModel
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_5_0, VertexShaderFunction()));
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PixelShaderFunction()));
	}
}