// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)
Texture2D grassTexture : register(t0);//can be other textures than grass, slope and rock
Texture2D slopeTexture : register(t1);
Texture2D rockTexture : register(t2);
SamplerState SampleType : register(s0);


cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;
};

float4 main(InputType input) : SV_TARGET
{
	float4  textureColor;
	float4	grassColor;
	float4  slopeColor;
	float4  rockColor;
	float4	color;
	float3	lightDir;
	float   slope;
	float   blendAmount;
    float	lightIntensity;
    

	// Invert the light direction for calculations.
	lightDir = normalize(input.position3D - lightPosition);

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, -lightDir));

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	color = ambientColor + (diffuseColor * lightIntensity); //adding ambient
	color = saturate(color);

	if (padding == 0) 
	{
		grassColor = grassTexture.Sample(SampleType, input.tex);
		slopeColor = slopeTexture.Sample(SampleType, input.tex);
		rockColor = rockTexture.Sample(SampleType, input.tex);

		// Sample the pixel color from the texture using the sampler at this texture coordinate location.
		slope = 1.0 - input.normal.y;

		if (slope < 0.2)
		{
			blendAmount = slope / 0.2f;
			textureColor = lerp(grassColor, slopeColor, blendAmount);
		}

		if ((slope < 0.7) && (slope >= 0.2))
		{
			blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
			textureColor = lerp(slopeColor, rockColor, blendAmount);
		}

		if (slope >= 0.7)
		{
			textureColor = rockColor;
		}
	}
	else 
	{
		textureColor = grassTexture.Sample(SampleType, input.tex);
	}
		

	
	color = color * textureColor;

    return color;
}

