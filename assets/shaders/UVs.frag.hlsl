float4 main(float2 uv : TEXCOORD0) : SV_TARGET0
{
    return float4(uv, 0, 1);
}