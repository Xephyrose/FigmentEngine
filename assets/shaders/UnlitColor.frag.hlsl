cbuffer PushConstants : register(b0, space3)
{
    float4 color;
}

float4 main() : SV_TARGET
{
	return color;
}
