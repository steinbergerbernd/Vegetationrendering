float4x4 World;
float4x4 ViewProjection;

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
	VOut output;

	float4 worldPos = mul(position, World);
	output.position = mul(worldPos, ViewProjection);
	//output.position = worldPos;
	output.color = color;

	return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	return color;
}


technique11 Default
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_5_0,VShader()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0,PShader()));
	}
}