#include "D3D11Utils.h"

namespace My
{
	void CheckResult(HRESULT hr, ID3DBlob* errorBlob) {
		if (FAILED(hr)) {
			// 파일이 없을 경우
			if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
				std::cout << "File not found.\n";
			}

			// 에러 메시지가 있으면 출력
			if (errorBlob) 
			{
				OutputDebugStringA("Shader compile error:\n");

				const char* errorMessage = static_cast<const char*>(errorBlob->GetBufferPointer());

				OutputDebugStringA(errorMessage);
				OutputDebugStringA("\n");
			}
		}
	}
	bool D3D11Utils::CreateDepthBuffer(ComPtr<ID3D11Device>& device, int screenWidth, int screenHeight, ComPtr<ID3D11DepthStencilView>& depthStencilView)
	{
		D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
		depthStencilBufferDesc.Width = screenWidth;
		depthStencilBufferDesc.Height = screenHeight;
		depthStencilBufferDesc.MipLevels = 1;
		depthStencilBufferDesc.ArraySize = 1;
		depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		//if (numQualityLevels > 0) {
		//    depthStencilBufferDesc.SampleDesc.Count = 4; // how many multisamples
		//    depthStencilBufferDesc.SampleDesc.Quality = numQualityLevels - 1;
		//}
		//else {
		//    depthStencilBufferDesc.SampleDesc.Count = 1; // how many multisamples
		//    depthStencilBufferDesc.SampleDesc.Quality = 0;
		//}
		depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilBufferDesc.CPUAccessFlags = 0;
		depthStencilBufferDesc.MiscFlags = 0;

		ComPtr<ID3D11Texture2D> depthStencilBuffer;

		if (FAILED(device->CreateTexture2D(&depthStencilBufferDesc, 0, depthStencilBuffer.GetAddressOf())))
		{
			OutputDebugStringW(L"CreateTexture2D() failed");
		}
		if (FAILED(device->CreateDepthStencilView(depthStencilBuffer.Get(), 0, depthStencilView.GetAddressOf())
		))
		{
			OutputDebugStringW(L"CreateDepthStencilView() failed");
		}
		return true;
	}
	bool D3D11Utils::CreateVertexShaderAndInputLayout(ComPtr<ID3D11Device>& device, const wstring& fileName, const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements, ComPtr<ID3D11VertexShader>& m_vertexShader, ComPtr<ID3D11InputLayout>& m_inputLayout)
	{
		ComPtr<ID3DBlob> shaderBlob;
		ComPtr<ID3DBlob> errorBlob;

		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT hr = D3DCompileFromFile(
			fileName.c_str(), 0, 0, "main", "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

		CheckResult(hr, errorBlob.Get());

		if (FAILED(hr))
		{
			OutputDebugStringW(L"Shader Compile() failed");
			return false;
		}

		if (FAILED(device->CreateVertexShader(shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize(), NULL, &m_vertexShader)))
		{
			OutputDebugStringW(L"CreateVertexShader() failed");
			return false;
		}

		if (FAILED(device->CreateInputLayout(inputElements.data(), static_cast<UINT>(inputElements.size()),
			shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &m_inputLayout)))
		{
			OutputDebugStringW(L"CreateInputLayout() failed");
			return false;
		}

		return true;
	}
	bool D3D11Utils::CreatePixelShader(ComPtr<ID3D11Device>& device, const wstring& fileName, ComPtr<ID3D11PixelShader>& m_pixelShader)
	{
		ComPtr<ID3DBlob> shaderBlob;
		ComPtr<ID3DBlob> errorBlob;

		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		HRESULT hr = D3DCompileFromFile(
			fileName.c_str(), 0, 0, "main", "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

		CheckResult(hr, errorBlob.Get());

		if (FAILED(hr))
		{
			OutputDebugStringW(L"Shader Compile() failed");
			return false;
		}

		if (FAILED(device->CreatePixelShader(shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize(), NULL, &m_pixelShader)))
		{
			OutputDebugStringW(L"CreatePixelShader() failed");
			return false;
		}

		return true;
	}

	bool D3D11Utils::CreateIndexBuffer(ComPtr<ID3D11Device>& device, const vector<uint32_t>& indices, ComPtr<ID3D11Buffer>& indexBuffer)
	{
		if (!device || !indices.size())
		{
			return false;
		}

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
		bufferDesc.ByteWidth = UINT(sizeof(uint32_t) * indices.size());
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
		bufferDesc.StructureByteStride = sizeof(uint32_t);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = indices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;

		if (FAILED(device->CreateBuffer(&bufferDesc, &indexBufferData,
			indexBuffer.GetAddressOf())))
		{
			OutputDebugStringW(L"CreateIndexBuffer failed");
			return false;
		}

		return true;
	}
}


