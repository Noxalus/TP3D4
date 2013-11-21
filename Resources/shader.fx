shared float4x4 WorldViewProj;
shared Texture2D Texture;

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
	return float4(input.Normal, 1.0f) * 0.5 + 0.5f;
	
}

technique normal
{
	pass p0
	{
		VertexShader = compile vs_3_0 VertexMain();
		PixelShader  = compile ps_3_0 PixelMain();
	}
}