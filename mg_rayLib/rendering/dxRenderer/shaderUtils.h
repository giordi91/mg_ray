#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <iostream>
#include <string>


inline void OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                              const WCHAR *shaderFilename) {
  char *compileErrors;
  unsigned long long bufferSize;
  std::ofstream fout;

  // Get a pointer to the error message text buffer.
  compileErrors = static_cast<char *>(errorMessage->GetBufferPointer());

  // Get the length of the message.
  bufferSize = errorMessage->GetBufferSize();

  // Open a file to write the error message to.
  fout.open("shader-error.txt");

  // Write out the error message.
  std::cout << "error in compiling shader: " << shaderFilename << std::endl;
  for (uint64_t i = 0; i < bufferSize; i++) {
    std::cout << compileErrors[i];
    fout << compileErrors[i];
  }
  // Close the file.
  fout.close();
  std::cout << "\n ================================" << std::endl;
  std::cout << " log dumped to shader-error.txt" << std::endl;

  // Release the error message.
  errorMessage->Release();
}

inline ID3D10Blob *compileShaderFromSource(const std::string &fileName,
                                    const std::string &entryPoint, 
									LPCSTR target) {

  ID3D10Blob *errorMessage;
  // Compile the vertex shader code.
  ID3D10Blob *buffer = nullptr;
  std::wstring stemp = std::wstring(fileName.begin(), fileName.end());
  LPCWSTR sw = stemp.c_str();

  UINT compileFlags = D3D10_SHADER_ENABLE_STRICTNESS;
  #if defined(DEBUG) || defined(_DEBUG)
  compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif


  LPCSTR entryPtr = entryPoint.c_str();
  HRESULT result = D3DCompileFromFile(sw, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPtr, target,
                                      compileFlags, 0,
                                      &buffer, &errorMessage);

  if (FAILED(result)) {
    // If the shader failed to compile it should have writen something to the
    // error message.
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, sw);
    }
    // If there was  nothing in the error message then it simply could not find
    // the shader file itself.
    else {
      // MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
      std::cout << "could not find shader: " << fileName << std::endl;
      return nullptr;
    }

    return nullptr;
  }
  return buffer;
}

struct VSBlob {
  ID3D11VertexShader *shader;
  ID3D10Blob *shaderBlob;

  inline bool isNull() const {
    return (shader == nullptr) | (shaderBlob == nullptr);
  }
  inline void releaseBlob() {
    shaderBlob->Release();
  }
};
struct PSBlob {
  ID3D11PixelShader *shader;
  ID3D10Blob *shaderBlob;

  inline bool isNull() const {
    return (shader == nullptr) | (shaderBlob == nullptr);
  }
  inline void releaseBlob() {
    shaderBlob->Release();
  }
};
struct CSBlob {
  ID3D11ComputeShader*shader;
  ID3D10Blob *shaderBlob;

  inline bool isNull() const {
    return (shader == nullptr) | (shaderBlob == nullptr);
  }
  inline void releaseBlob() {
    shaderBlob->Release();
  }
};

inline CSBlob compileComputeShader(const std::string &fileName,
                           const std::string &entryPoint,
                           ID3D11Device *device) {

  CSBlob data{nullptr, compileShaderFromSource(fileName, entryPoint, "cs_5_0")};

  // Create the vertex shader from the buffer.
  HRESULT result = device->CreateComputeShader(
      data.shaderBlob->GetBufferPointer(), data.shaderBlob->GetBufferSize(),
      NULL, &data.shader);
  if (FAILED(result)) {
    std::cout << "failing in creating compute shader for: " << fileName
              << std::endl;
  }
  return data;
}

inline VSBlob compileVertexShader(const std::string &fileName,
                           const std::string &entryPoint,
                           ID3D11Device *device) {

  VSBlob data{nullptr, compileShaderFromSource(fileName, entryPoint, "vs_5_0")};

  // Create the vertex shader from the buffer.
  HRESULT result = device->CreateVertexShader(
      data.shaderBlob->GetBufferPointer(), data.shaderBlob->GetBufferSize(),
      NULL, &data.shader);
  if (FAILED(result)) {
    std::cout << "failing in creating vertex shader for: " << fileName
              << std::endl;
  }
  return data;
}

inline PSBlob compilePixelShader(const std::string &fileName,
                          const std::string &entryPoint, ID3D11Device *device) {
  PSBlob data{nullptr, compileShaderFromSource(fileName, entryPoint, "ps_5_0")};
  // Create the vertex shader from the buffer.
  HRESULT result = device->CreatePixelShader(
      data.shaderBlob->GetBufferPointer(), data.shaderBlob->GetBufferSize(),
      NULL, &data.shader);
  if (FAILED(result)) {
    std::cout << "failing in creating pixel shader for: " << fileName
              << std::endl;
  }
  return data;
}

// this api are based on reflector to extract data from the shader
inline ID3D11InputLayout *loadLayoutFromShader(ID3D10Blob *shaderBlob,
                                        ID3D11Device *device) {

  ID3D11ShaderReflection *pReflector = nullptr;
  HRESULT result =
      D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
                 IID_ID3D11ShaderReflection, (void **)&pReflector);

  if (FAILED(result)) {
    // MessageBox(hwnd, L"Failed to create the reflection interface", L"Error",
    //	MB_OK);
    return nullptr;
  }
  int byteOffset = 0;
  D3D11_SHADER_DESC desc;
  pReflector->GetDesc(&desc);

  D3D11_INPUT_ELEMENT_DESC *polygonLayout =
      new D3D11_INPUT_ELEMENT_DESC[desc.InputParameters];
  for (unsigned int i = 0; i < desc.InputParameters; ++i) {
    D3D11_SIGNATURE_PARAMETER_DESC inSign;
    pReflector->GetInputParameterDesc(i, &inSign);
    polygonLayout[i].SemanticName = inSign.SemanticName;
    polygonLayout[i].SemanticIndex = inSign.SemanticIndex;
    polygonLayout[i].InputSlot = 0;
    polygonLayout[i].AlignedByteOffset = byteOffset;
    polygonLayout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[i].InstanceDataStepRate = 0;

    auto *p = &polygonLayout[i];
    // determine DXGI format
    if (inSign.Mask == 1) {
      if (inSign.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
        p->Format = DXGI_FORMAT_R32_UINT;
      else if (inSign.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
        p->Format = DXGI_FORMAT_R32_SINT;
      else if (inSign.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
        p->Format = DXGI_FORMAT_R32_FLOAT;
      byteOffset += 4;
    } else if (inSign.Mask <= 3) {
      if (inSign.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
        p->Format = DXGI_FORMAT_R32G32_UINT;
      else if (inSign.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
        p->Format = DXGI_FORMAT_R32G32_SINT;
      else if (inSign.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
        p->Format = DXGI_FORMAT_R32G32_FLOAT;
      byteOffset += 8;
    } else if (inSign.Mask <= 7) {
      if (inSign.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
        p->Format = DXGI_FORMAT_R32G32B32_UINT;
      else if (inSign.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
        p->Format = DXGI_FORMAT_R32G32B32_SINT;
      else if (inSign.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
        p->Format = DXGI_FORMAT_R32G32B32_FLOAT;
      byteOffset += 12;
    } else if (inSign.Mask <= 15) {
      if (inSign.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
        p->Format = DXGI_FORMAT_R32G32B32A32_UINT;
      else if (inSign.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
        p->Format = DXGI_FORMAT_R32G32B32A32_SINT;
      else if (inSign.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
        p->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      byteOffset += 16;
    }
  }

  // Create the vertex input layout.
  int numElements = desc.InputParameters;
  ID3D11InputLayout *layout;
  result = device->CreateInputLayout(polygonLayout, numElements,
                                     shaderBlob->GetBufferPointer(),
                                     shaderBlob->GetBufferSize(), &layout);
  if (FAILED(result)) {
    return nullptr;
  }

  delete[] polygonLayout;
  return layout;
}

inline ID3D11Buffer **loadConstantBuffersFromShader(ID3D10Blob *shaderBlob,
                                             ID3D11Device *device,
                                             int &const_buffer_size) {

  ID3D11ShaderReflection *pReflector = nullptr;
  HRESULT result =
      D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
                 IID_ID3D11ShaderReflection, (void **)&pReflector);

  if (FAILED(result)) {
    // MessageBox(hwnd, L"Failed to create the reflection interface", L"Error",
    //	MB_OK);
    return nullptr;
  }
  int byteOffset = 0;
  D3D11_SHADER_DESC desc;
  pReflector->GetDesc(&desc);

  D3D11_BUFFER_DESC *matrixBufferDesc =
      new D3D11_BUFFER_DESC[desc.ConstantBuffers];
  auto *const_buffers = new ID3D11Buffer *[desc.ConstantBuffers];
  const_buffer_size = desc.ConstantBuffers;
  for (unsigned int i = 0; i < desc.ConstantBuffers; ++i) {
    ID3D11ShaderReflectionConstantBuffer *buff =
        pReflector->GetConstantBufferByIndex(i);
    D3D11_SHADER_BUFFER_DESC buffDesc;
    buff->GetDesc(&buffDesc);

    // TODO(giordi) any way to know this stuff?
    matrixBufferDesc[i].Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc[i].ByteWidth = buffDesc.Size;
    matrixBufferDesc[i].BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc[i].CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc[i].MiscFlags = 0;
    matrixBufferDesc[i].StructureByteStride = 0;

    result =
        device->CreateBuffer(&matrixBufferDesc[i], NULL, &const_buffers[i]);
    if (FAILED(result)) {
      return nullptr;
    }
  }
  delete[] matrixBufferDesc;
  return const_buffers;
}
