#include "GeneralShaderFunctions.hlsl"

struct VertexShaderInput
{
	float3 position : POSITION0;
	float2 texCoord : TEXCOORD0;
};

struct VertexShaderOutput
{
	float3 position : POSITION0;
	float2 texCoord : TEXCOORD0;
};

struct HSConstantOutput
{
	float edges[4]  : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

struct HullShaderOutput
{
    float3 position  : POSITION0;
	 float2 texCoord : TEXCOORD0;
};

struct DomainShaderOutput
{
    float4 position  : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	output.position = input.position;
	output.texCoord = input.texCoord;

	return output;
}


HSConstantOutput HSConstantFunction( InputPatch<VertexShaderOutput, 4> ip, uint pid : SV_PrimitiveID )
{
	HSConstantOutput output;
	
	float edge = 4;
	float inside = 4;

	output.edges[0] = edge;
	output.edges[1] = edge;
	output.edges[2] = edge;
	output.edges[3] = edge;

	output.inside[0] = inside;
	output.inside[1] = inside;

	return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSConstantFunction")]
HullShaderOutput HullShaderFunction( InputPatch<VertexShaderOutput, 4> ip, uint cpid : SV_OutputControlPointID, uint pid : SV_PrimitiveID )
{
	HullShaderOutput output;
	output.position = ip[cpid].position;
	output.texCoord = ip[cpid].texCoord;
	return output;
}

[domain("quad")]
DomainShaderOutput DomainShaderFunction( HSConstantOutput input, float2 UV : SV_DomainLocation, const OutputPatch<HullShaderOutput, 4> patch )
{
	DomainShaderOutput output;
    
	float3 topMidpoint = lerp(patch[0].position, patch[1].position, UV.x);
	float3 bottomMidpoint = lerp(patch[3].position, patch[2].position, UV.x);
    
	output.position = float4(lerp(topMidpoint, bottomMidpoint, UV.y), 1);
	output.position = mul(mul(output.position, World), ViewProjection);
	output.texCoord = float2(0.5f, 0.5f);
    
	return output;
}


float4 PixelShaderFunction(DomainShaderOutput input) : SV_TARGET
{
	return float4(1,1,1,1);
}


technique11 Default
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_5_0,VertexShaderFunction()));
		SetHullShader( CompileShader( hs_5_0, HullShaderFunction() ) );
		SetDomainShader( CompileShader( ds_5_0, DomainShaderFunction() ) );
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0,PixelShaderFunction()));
	}
}