cbuffer CameraBuffer : register(b0) {
  matrix MVP;
  matrix normalMatrix;
};

struct VertexInputType {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
};

struct PixelInputType {
  float4 position : SV_POSITION;
  float4 wolrdPosition : POSITION;
  float4 normal : NORMAL;
  float4 uv : TEXCOORD;
};

PixelInputType main(VertexInputType input) {
  PixelInputType output;

  // Calculate the position of the vertex against the world, view, and
  // projection matrices.
  output.position = mul(float4(input.position,1.0f), MVP);
  output.wolrdPosition = float4(input.position,1.0f);
  output.normal = float4(input.normal,0.0f);
  output.uv.xy = input.uv;
  return output;
}
