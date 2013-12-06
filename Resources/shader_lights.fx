#define LIGHTS_NUMBER 1

shared float4x4 WorldViewProj;
shared Texture2D Texture;
shared float4 DirectionalLightColor;
shared float3 DirectionalLightDirection;
shared float3 CameraDirection;

float4 OmniLightColors[LIGHTS_NUMBER];
float3 OmniLightPositions[LIGHTS_NUMBER];
shared float OmniLightDistance;

struct VertexInputType
{
	float3 Position   : POSITION;
	float2 UV  		  : TEXCOORD0;
	float3 Normal 	  : NORMAL;
};

struct PixelInputType
{
	float4 Position   : POSITION;
	float2 UV  		  : TEXCOORD0;
	float3 Normal     : TEXCOORD1;
};

struct PixelOutput
{

};

sampler2D  MapSampler = sampler_state
{
	Texture = Texture;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

PixelInputType VertexMain(VertexInputType input)
{
	PixelInputType output;

	output.Position = mul(float4(input.Position, 1.0f), WorldViewProj);
	output.UV = input.UV;
	output.Normal = input.Normal;

	return output;
}

float4 PixelMain(PixelInputType input) : COLOR0
{
	float4 ambiantColor = float4(1, 1, 1, 0);
	float4 ambiantLighting = float4(1, 1, 1, 0);

	int specularPower = 32;
	float4 specularColor = float4(0, 0, 0, 0);

	float4 color = ambiantColor;

	// Omnilight
	float omniLightIntensity = saturate(dot(input.Normal, OmniLightPositions[0]));
	float4 omniLightColor = OmniLightColors[0] * omniLightIntensity;

	// Invert the light direction for calculations.
	float3 invertedDirectionalLightDirection = -DirectionalLightDirection;

	float4 diffuseLighting = dot(input.Normal, invertedDirectionalLightDirection);

	// Calculate the amount of light on this pixel.
	float lightIntensity = saturate(diffuseLighting);

	if (lightIntensity > 0.0f)
	{
		// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
		color += (DirectionalLightColor * lightIntensity);
	}

	// Saturate the final light color.
	color = saturate(color + omniLightColor);

	float4 diffuseColor = color * tex2D(MapSampler, input.UV);

	// Specular computation
	float3 reflectionVector = normalize(invertedDirectionalLightDirection + (2 * input.Normal * dot(input.Normal, DirectionalLightDirection)));
	float4 specularLighting = saturate(dot(reflectionVector, CameraDirection));
	specularLighting = pow(specularLighting, specularPower);

	float4 finalColor =
	diffuseColor * ambiantLighting +
	diffuseColor * diffuseLighting +
	specularColor * specularLighting;

	return finalColor;
}

technique normal
{
	pass p0
	{
		VertexShader = compile vs_3_0 VertexMain();
		PixelShader = compile ps_3_0 PixelMain();
	}
}