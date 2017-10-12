
[RootSignature("RootFlags(0)")]
float4 main(uint vertexIdx : SV_VertexID) : SV_POSITION
{
	const float2 positions[3] = { float2(-1.0f, -1.0f), float2(3.0f, -1.0f), float2(-1.0f, 3.0f) };
	return float4(positions[vertexIdx], 0.0f, 1.0f);
}
