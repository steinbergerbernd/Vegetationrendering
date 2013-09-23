float4x4 World;
float4x4 ViewProjection;

Texture2D <float4> Heightmap;
Texture2D <float4> Normalmap;

Texture2D <float4> Texture0;
Texture2D <float4> Texture1;
Texture2D <float4> Texture2;
Texture2D <float4> TextureCliff;

float2 TextureResolution;

float4 TextureRange;
float4 CliffRange;

int TerrainWidth;
int TerrainLength;

float Bumpiness;

struct TLight
{
	float3 Direction;
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
};

struct TMaterial
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Emissive;
	float Power;
};

struct TCamera
{
	float3 Position;
};

TMaterial Material;
TLight Light;
TCamera Camera;

int LevelOfDetail;
int MinLevelOfDetail;

sampler HeightmapSampler;
sampler NormalmapSampler;

sampler TextureSampler0;
sampler TextureSampler1;
sampler TextureSampler2;
sampler TextureSamplerCliff;


struct VSInstance
{
	float2 PatchOffset	: TEXCOORD0;
	float Blending			: TEXCOORD1;
};


struct VertexShaderInput
{
	float4 Position : POSITION;
};


struct VertexShaderOutput
{
	float4 Position		: SV_POSITION;
	float2 TexCoord		: TEXCOORD0;
	float3 View				: TEXCOORD1;
	float Depth				: TEXCOORD2;
	float4 WorldPos		: TEXCOORD3;
	float4 ScreenPos		: TEXCOORD4;
};


float2 getTexCoord(float2 position)
{
	return float2(position.x / TerrainWidth, 1.0f - position.y / TerrainLength);
}


float getHeight(float2 position)
{
	return tex2Dlod(HeightmapSampler, float4(getTexCoord(position), 0, 0)).r;
}


float getMorphedHeight(float2 position, float blending, int levelOfDetail)
{
	float height = getHeight(position);

	if (levelOfDetail == MinLevelOfDetail)
		return height;

	int delta = pow(2, levelOfDetail);

	int a = ((position.x / delta) % 2) * delta;
	int b = ((position.y / delta) % 2) * delta;

	float h0 = getHeight(position + float2(a, b));
	float h1 = getHeight(position - float2(a, b));

	return height + blending * ((h0 + h1) / 2.0f - height);
}


VertexShaderOutput VertexShaderFunction(VertexShaderInput input)//, VSInstance instance)
{
	VertexShaderOutput output;

	float4 position = input.Position;

	//position.xz += instance.PatchOffset;

	float2 texCoord = getTexCoord(position.xz);

	//float height = getMorphedHeight(position.xz, instance.Blending, LevelOfDetail) * Bumpiness;

	position.y = 1;//(LevelOfDetail + 1) * position.y + height;

	float4 worldPos = mul(position, World);

	output.Position = mul(worldPos, ViewProjection);
	output.TexCoord = texCoord;
	output.View = Camera.Position - worldPos.xyz;
	output.Depth = output.Position.z;
	output.WorldPos = worldPos;
	output.ScreenPos = output.Position;

	return output;
}


float4 PixelShaderFunction(VertexShaderOutput input) : SV_Target
{    	
	float3 normal = float3(0,1.0,0);//normalize(2.0f * tex2D(NormalmapSampler, input.TexCoord).xzy - 1.0f);
	float3 light = normalize(-Light.Direction);
	float3 view = normalize(input.View);

	float3 reflection = reflect(-light, normal);

	float diffuse = saturate(dot(normal, light));
	float specular = pow(saturate(dot(view, reflection)), Material.Power);
	
	float height = 1;//input.WorldPos.y / Bumpiness;

	float2 texCoord = input.TexCoord * TextureResolution;

	float4 tex0 = Texture0.Sample(TextureSampler0, texCoord);
	float4 tex1 = Texture1.Sample(TextureSampler1, texCoord);
	float4 tex2 = Texture2.Sample(TextureSampler2, texCoord);
	float4 texCliff = TextureCliff.Sample(TextureSamplerCliff, texCoord);

	float weight0 = saturate(1.0f - (height - TextureRange.y) / (TextureRange.x - TextureRange.y));
	float weight2 = saturate((height - TextureRange.w) / (TextureRange.z - TextureRange.w));
	float weight1 = 1.0f - weight0 - weight2;

	float weightCliff = saturate(1.0f - (normal.y - CliffRange.x) / (CliffRange.y - CliffRange.x));

	float4 colorTexture = lerp(tex0 * weight0 + tex1 * weight1 + tex2 * weight2, texCliff, weightCliff);

	float4 colorAmbient = Material.Ambient * Light.Ambient;
	float4 colorDiffuse = Material.Diffuse * Light.Diffuse * diffuse;
	float4 colorSpecular = Material.Specular * Light.Specular * specular * weightCliff;

	return colorTexture * saturate(colorAmbient + colorDiffuse) + colorSpecular;
}


technique11 Default
{
	pass Pass0
	{
		SetVertexShader(CompileShader(vs_4_0,VertexShaderFunction()));
		SetPixelShader(CompileShader(ps_4_0,PixelShaderFunction()));
	}
}
