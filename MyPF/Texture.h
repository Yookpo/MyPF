#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <string>

namespace My
{
	using Microsoft::WRL::ComPtr;

	class Texture
	{
	public:
		Texture() = default;
		Texture(const Texture&) = delete;
		Texture& operator = (const Texture&) = delete;
		
		bool Initialize(ID3D11Device*, const std::string&);

		ID3D11Texture2D* GetTexture() const { return m_texture.Get(); }
		ID3D11ShaderResourceView* GetShaderResourceView() const { return m_textureResourceView.Get(); }

	private:
		ComPtr<ID3D11Texture2D> m_texture;
		ComPtr<ID3D11ShaderResourceView> m_textureResourceView;
	};
}


