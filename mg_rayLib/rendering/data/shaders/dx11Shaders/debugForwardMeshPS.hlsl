cbuffer Material : register(b0) {
  float4 cameraPosition;
  float4 lightPosition;
  float4 ambient;
  float4 specular;
  float4 diffuse;
  float shiness;
  float3 padding;
};

struct PixelInputType {
  float4 position : SV_POSITION;
  float4 wolrdPosition : POSITION;
  float4 normal : NORMAL;
  float4 uv : TEXCOORD;
};

float4 main(PixelInputType input, bool isFrontFace : SV_IsFrontFace) : SV_TARGET {

  float4 eyeDir = normalize(cameraPosition - input.wolrdPosition);
  float4 normal = input.normal;
  float4 lightDir = -normalize(lightPosition);
  
  float diffuseAmount = max(dot(-lightDir, normal), 0.0f);
  float4 outColor = diffuse * diffuseAmount;
  outColor += ambient;

  // computing specular
  float4 reflected = normalize(reflect(lightDir, input.normal));
  // need to use proper light and eye dir
  float4 specularAmount = pow(max(dot(reflected, eyeDir), 0.0f), shiness);
  outColor += specular * specularAmount;
  outColor.w = 1.0f;
  return outColor;
}
