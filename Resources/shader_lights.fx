shared float4x4 WorldViewProj;
shared Texture2D Texture;
shared float4 LightColor;
shared float3 LightDirection;
shared float3 CameraDirection;

struct VertexInput
{
	float3 Position   : POSITION;
	float2 UV  		  : TEXCOORD0;
	float3 Normal 	  : NORMAL;
};

struct VertexOutput
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

VertexOutput VertexMain(VertexInput input)
{
	VertexOutput output;

	output.Position = mul(float4(input.Position, 1.0f), WorldViewProj);
	output.UV = input.UV;
	output.Normal = input.Normal;

	return output;
}

float4 PixelMain(VertexOutput input) : COLOR0
{
	// Invert the light direction for calculations.
	float3 invertedLightDirection = -LightDirection;

	// Calculate the amount of light on this pixel.
	float4 diffuseLighting = dot(input.Normal, invertedLightDirection);

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	float4 color = saturate(LightColor * diffuseLighting);

	float4 diffuseColor = tex2D(MapSampler, input.UV);

	float4 ambiantLighting = float4(1, 1, 1, 0);

	float3 reflectionVector = -LightDirection + (2 * input.Normal * dot(input.Normal, LightDirection));
	float4 specularLighting = dot(reflectionVector, CameraDirection);
	specularLighting = pow(specularLighting, 64);

	float4 specularColor = float4(1, 1, 1, 0);

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