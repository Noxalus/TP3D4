shared float4x4 WorldViewProj;
shared Texture2D Texture;
shared float4 LightColor;
shared float3 LightDirection;
shared float3 LookAt;

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
	float3 Normal     : NORMAL;
};

struct PixelOutput
{
	
};

sampler2D  MapSampler = sampler_state
{
	Texture  =  Texture;
	MinFilter  =  LINEAR;
	MagFilter  =  LINEAR;
	MipFilter  =  LINEAR;
	AddressU  =  WRAP;
	AddressV  =  WRAP;
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
	float4 diffuseColor = LightColor;
	float4 ambiantLighting = float4(0.5, 0.5, 0.5, 0);
	float4 diffuseLighting = dot(input.Normal, LightDirection);
	
	float3 reflectionVector = -LightDirection + (2 * input.Normal * dot(input.Normal, LightDirection));
	float4 specularLighting = dot(reflectionVector, LookAt);
	specularLighting = pow(specularLighting, 64);
	float4 specularColor = float4(1, 0, 0, 0);

	return tex2D(MapSampler, input.UV) * (2.0 * (
	diffuseColor * diffuseLighting + 
	specularColor * specularLighting
	) + 0.5 + ambiantLighting);
}

technique normal
{
	pass p0
	{
		VertexShader = compile vs_3_0 VertexMain();
		PixelShader  = compile ps_3_0 PixelMain();
	}
}