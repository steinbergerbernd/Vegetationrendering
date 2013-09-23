float4x4 World;
float4x4 ViewProjection;

float3 CameraPosition;

float3 LightVector;
float3 AmbientLight;
float3 DiffuseLight;
float3 SpecularLight;
float Shininess;

SamplerState LinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState AnisotropicSampler
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState PointSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};


// Berechnung des diffusen Lichtes
float4 CalculateDiffuseLight(float3 lightDirection, float3 normal)
{
	// zurückgeworfene Intensität des diffusen Lichtes
	float diffuseLightAttenuation = saturate(dot(lightDirection, normal));
	
	// Berechnung der diffusen Komponente
	return float4(saturate(DiffuseLight * diffuseLightAttenuation),1);
}

// Berechnung des spekulären Lichtes
float4 CalculateSpecularLight(float3 lightDirection, float3 normal, float3 view)
{
	// Berechnung des Reflexionsvektors
	float3 reflectionVector = reflect(lightDirection, normal);
	
	// zurückgeworfene Intensität des spekulären Lichtes
	float specularLightAttenuation = saturate(dot(reflectionVector, view));

	// Berechnung der spekulären Komponente
	return float4(SpecularLight * pow(specularLightAttenuation, Shininess), 1);
}