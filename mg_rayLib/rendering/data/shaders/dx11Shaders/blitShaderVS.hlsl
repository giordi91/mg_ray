
struct PixelInputType {
 
	float4 position : SV_Position; // vertex position 
	float2 cpPos	: TEXCOORD0;
	float2 uv	: TEXCOORD1;
};

static const float4 arrBasePos[4] =
{
    float4(-1.0f, 1.0f,0.0f,0.0f), 
	float4(1.0f, 1.0f,1.0f,0.0f), 
	float4(-1.0f, -1.0f,0.0f,1.0f), 
	float4(1.0f, -1.0f,1.0f,1.0f)
};

PixelInputType main(uint vid : SV_VertexID) {
  PixelInputType output;

  // Calculate the position of the vertex against the world, view, and
  // projection matrices.
  float4 p = arrBasePos[vid];
  output.position.xy = p.xy;
  output.position.z = 0.0f;
  output.position.w = 1.0f;
  output.cpPos = p.xy; 
  output.uv = p.zw;
  return output;
}
