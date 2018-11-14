Texture2D<float4> inputTexture : register(t0);
SamplerState LinearSampler : register(s0);

struct PixelInputType {

  float4 position : SV_Position; // vertex position
  float2 cpPos : TEXCOORD0;
  float2 uv : TEXCOORD1;
};

float4 main(PixelInputType input, bool isFrontFace
            : SV_IsFrontFace)
    : SV_TARGET {
  return  inputTexture.Sample(LinearSampler, float2(input.uv.x,(1.0f-input.uv.y)));
  //return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
