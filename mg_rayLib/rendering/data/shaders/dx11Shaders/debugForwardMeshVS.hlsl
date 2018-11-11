cbuffer CameraBuffer : register(b0) {
  matrix MVP;
  matrix normalMatrix;
};

struct VertexInputType {
  float4 position : POSITION;
  float4 normal : NORMAL;
  float4 uv : TEXCOORD;
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
  output.position = mul(input.position, MVP);
  output.wolrdPosition = input.position;
  output.normal = input.normal;
  output.uv = input.uv;
  return output;
}
