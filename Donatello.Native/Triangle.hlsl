struct VertexInput
{
    float3 position : POSITION;
    float3 colour : COLOR0;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 colour : COLOR0;
};

VertexOutput VSMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 1.0f);
    output.colour = input.colour;
    return output;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{
    return float4(input.colour, 1.0f);
}