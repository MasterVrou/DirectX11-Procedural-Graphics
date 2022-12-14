#include "pch.h"
#include "SkyboxEffect.h"
#include "ReadData.h"

using namespace DirectX;

namespace
{
	constexpr uint32_t DirtyConstantBuffer = 0x1;
	constexpr uint32_t DirtyWVPMatrix = 0x2;
}

DX::SkyboxEffect::SkyboxEffect(ID3D11Device* device) : m_dirtyFlags(uint32_t(-1)), m_constantBuffer(device)
{
	static_assert((sizeof(SkyboxEffect::SkyboxEffectConstants) % 16) == 0, "CB size alignment");

	m_vsBlob = DX::ReadData(L"SkyboxEffect_VS.cso");

	DX::ThrowIfFailed(device->CreateVertexShader(m_vsBlob.data(), m_vsBlob.size(), nullptr, m_vs.ReleaseAndGetAddressOf()));

	auto psBlob = DX::ReadData(L"SkyboxEffect_PS.cso");

	DX::ThrowIfFailed(device->CreatePixelShader(psBlob.data(), psBlob.size(), nullptr, m_ps.ReleaseAndGetAddressOf()));
}

void __cdecl DX::SkyboxEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	if (m_dirtyFlags & DirtyWVPMatrix)
	{
		DirectX::XMMATRIX view = m_view;
		view.r[3] = DirectX::g_XMIdentityR3;
		m_worldViewProj = DirectX::XMMatrixMultiply(view, m_proj);
		
		m_dirtyFlags &= ~DirtyWVPMatrix;
		m_dirtyFlags |= DirtyConstantBuffer;
	}

	if (m_dirtyFlags & DirtyConstantBuffer)
	{
		SkyboxEffectConstants constants;
		constants.worldViewProj = DirectX::XMMatrixTranspose(m_worldViewProj);
		m_constantBuffer.SetData(deviceContext, constants);

		m_dirtyFlags &= ~DirtyConstantBuffer;
	}

	auto cb = m_constantBuffer.GetBuffer();
	deviceContext->VSSetConstantBuffers(0, 1, &cb);
	deviceContext->PSSetShaderResources(0, 1, m_texture.GetAddressOf());

	deviceContext->VSSetShader(m_vs.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_ps.Get(), nullptr, 0);
}

void __cdecl DX::SkyboxEffect::GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength)
{
	assert(pShaderByteCode != nullptr && pByteCodeLength != nullptr);
	*pShaderByteCode = m_vsBlob.data();
	*pByteCodeLength = m_vsBlob.size();
}

void DX::SkyboxEffect::SetTexture(ID3D11ShaderResourceView* value)
{
	m_texture = value;
}

void XM_CALLCONV DX::SkyboxEffect::SetWorld(DirectX::FXMMATRIX value)
{
	//Skybox does not need a world matrix
}

void XM_CALLCONV DX::SkyboxEffect::SetView(DirectX::FXMMATRIX value)
{
	m_view = value;
	m_dirtyFlags |= DirtyWVPMatrix;
}

void XM_CALLCONV DX::SkyboxEffect::SetProjection(DirectX::FXMMATRIX value)
{
	m_proj = value;
	m_dirtyFlags |= DirtyWVPMatrix;
}

void XM_CALLCONV DX::SkyboxEffect::SetMatrices(DirectX::FXMMATRIX world, DirectX::CXMMATRIX view, DirectX::CXMMATRIX projection)
{
	m_view = view;
	m_proj = projection;
	m_dirtyFlags |= DirtyWVPMatrix;
}
