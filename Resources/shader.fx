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
	return tex2D(MapSampler, input.UV);
}

technique normal
{
	pass p0
	{
		VertexShader = compile vs_3_0 VertexMain();
		PixelShader = compile ps_3_0 PixelMain();
	}
}