
Texture2D _texture;
SamplerState _sampler;

float4 main(float4 position : SV_POSITION, float4 color : COLOR, float2 uv : UV) : SV_TARGET
{
    color.a = _texture.Sample(_sampler, uv);
    return color;
}