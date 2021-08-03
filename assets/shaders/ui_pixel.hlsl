
float4 main(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    //color = max(1.055 * pow(color, 0.416666667) - 0.055, 0);
    
    return color;
}