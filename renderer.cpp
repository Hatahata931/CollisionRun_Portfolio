//-----------------------------------------------------------
//  [ renderer.cpp ]
//  DirectX11の描画基盤
//-----------------------------------------------------------
#include "main.h"
#include "renderer.h"

#include <io.h>

D3D_FEATURE_LEVEL Renderer::m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11Device* Renderer::m_Device{};
ID3D11DeviceContext* Renderer::m_DeviceContext{};
IDXGISwapChain* Renderer::m_SwapChain{};
ID3D11RenderTargetView* Renderer::m_RenderTargetView{};
ID3D11DepthStencilView* Renderer::m_DepthStencilView{};

ID3D11Buffer* Renderer::m_WorldBuffer{};
ID3D11Buffer* Renderer::m_ViewBuffer{};
ID3D11Buffer* Renderer::m_ProjectionBuffer{};
ID3D11Buffer* Renderer::m_MaterialBuffer{};
ID3D11Buffer* Renderer::m_LightBuffer{};
ID3D11Buffer* Renderer::m_CameraBuffer{};
ID3D11Buffer* Renderer::m_UVScrollBuffer{};
LIGHT Renderer::m_CurrentLight{};

ID3D11DepthStencilState* Renderer::m_DepthStateEnable{};
ID3D11DepthStencilState* Renderer::m_DepthStateDisable{};
ID3D11SamplerState* Renderer::m_SamplerStateWrap{};
ID3D11SamplerState* Renderer::m_SamplerStateVClamp{};

ID3D11BlendState* Renderer::m_BlendState{};
ID3D11BlendState* Renderer::m_BlendStateAdd{};

ID3D11RasterizerState* Renderer::m_RasterizerStateCullBack{};
ID3D11RasterizerState* Renderer::m_RasterizerStateCullNone{};

void Renderer::Init()
{
	//デバイスとスワップチェーンを作成
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = GetWindow();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	const HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_SwapChain,
		&m_Device,
		&m_FeatureLevel,
		&m_DeviceContext);
	assert(SUCCEEDED(result));

	//レンダーターゲットビューを作成
	ID3D11Texture2D* renderTarget = nullptr;
	m_SwapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&renderTarget));
	m_Device->CreateRenderTargetView(renderTarget, nullptr, &m_RenderTargetView);
	renderTarget->Release();

	//深度ステンシルバッファを作成
	ID3D11Texture2D* depthStencil = nullptr;
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = swapChainDesc.BufferDesc.Width;
	textureDesc.Height = swapChainDesc.BufferDesc.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc = swapChainDesc.SampleDesc;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	m_Device->CreateTexture2D(&textureDesc, nullptr, &depthStencil);

	//深度ステンシルバッファへアクセスするビューを作成
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	m_Device->CreateDepthStencilView(depthStencil, &depthStencilViewDesc, &m_DepthStencilView);
	depthStencil->Release();

	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

	//描画範囲を画面全体に設定
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<FLOAT>(SCREEN_WIDTH);
	viewport.Height = static_cast<FLOAT>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	m_DeviceContext->RSSetViewports(1, &viewport);

	//背面カリングありと両面描画のラスタライザステートを作成
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;

	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerStateCullBack);

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerStateCullNone);

	m_DeviceContext->RSSetState(m_RasterizerStateCullBack);

	//通常のアルファ合成と加算合成のブレンドステートを作成
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_Device->CreateBlendState(&blendDesc, &m_BlendState);

	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	m_Device->CreateBlendState(&blendDesc, &m_BlendStateAdd);

	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff);

	//深度書き込みの有効・無効を切り替えるステートを作成
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;
	m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStateEnable);

	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStateDisable);

	m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);

	//通常の繰り返し用とV方向固定用のサンプラーステートを作成
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	m_Device->CreateSamplerState(&samplerDesc, &m_SamplerStateWrap);
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	m_Device->CreateSamplerState(&samplerDesc, &m_SamplerStateVClamp);
	SetTextureVClamp(false);

	//GPUへ描画情報を渡す定数バッファを作成
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	m_Device->CreateBuffer(&bufferDesc, nullptr, &m_WorldBuffer);
	m_DeviceContext->VSSetConstantBuffers(0, 1, &m_WorldBuffer);

	m_Device->CreateBuffer(&bufferDesc, nullptr, &m_ViewBuffer);
	m_DeviceContext->VSSetConstantBuffers(1, 1, &m_ViewBuffer);

	m_Device->CreateBuffer(&bufferDesc, nullptr, &m_ProjectionBuffer);
	m_DeviceContext->VSSetConstantBuffers(2, 1, &m_ProjectionBuffer);

	bufferDesc.ByteWidth = sizeof(XMFLOAT4);

	m_Device->CreateBuffer(&bufferDesc, nullptr, &m_CameraBuffer);
	m_DeviceContext->PSSetConstantBuffers(5, 1, &m_CameraBuffer);

	m_Device->CreateBuffer(&bufferDesc, nullptr, &m_UVScrollBuffer);
	m_DeviceContext->VSSetConstantBuffers(6, 1, &m_UVScrollBuffer);
	m_DeviceContext->PSSetConstantBuffers(6, 1, &m_UVScrollBuffer);

	bufferDesc.ByteWidth = sizeof(MATERIAL);

	m_Device->CreateBuffer(&bufferDesc, nullptr, &m_MaterialBuffer);
	m_DeviceContext->VSSetConstantBuffers(3, 1, &m_MaterialBuffer);
	m_DeviceContext->PSSetConstantBuffers(3, 1, &m_MaterialBuffer);

	bufferDesc.ByteWidth = sizeof(LIGHT);

	m_Device->CreateBuffer(&bufferDesc, nullptr, &m_LightBuffer);
	m_DeviceContext->VSSetConstantBuffers(4, 1, &m_LightBuffer);
	m_DeviceContext->PSSetConstantBuffers(4, 1, &m_LightBuffer);

	//ライトを初期化
	LIGHT light{};
	light.Enable = true;
	light.Direction = XMFLOAT4(0.4f, -1.0f, 0.6f, 0.0f);
	light.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	light.Diffuse = XMFLOAT4(1.1f, 1.1f, 1.1f, 1.0f);
	SetLight(light);

	//マテリアルを初期化
	MATERIAL material{};
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	SetUVOffset(0.0f, 0.0f);
}

void Renderer::Uninit()
{
	m_DeviceContext->ClearState();

	m_WorldBuffer->Release();
	m_ViewBuffer->Release();
	m_ProjectionBuffer->Release();
	m_LightBuffer->Release();
	m_MaterialBuffer->Release();
	m_CameraBuffer->Release();
	m_UVScrollBuffer->Release();

	m_RenderTargetView->Release();
	m_DepthStencilView->Release();

	m_DepthStateEnable->Release();
	m_DepthStateDisable->Release();
	m_SamplerStateWrap->Release();
	m_SamplerStateVClamp->Release();

	m_BlendState->Release();
	m_BlendStateAdd->Release();

	m_RasterizerStateCullBack->Release();
	m_RasterizerStateCullNone->Release();

	m_SwapChain->Release();
	m_DeviceContext->Release();
	m_Device->Release();
}

void Renderer::Begin()
{
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, clearColor);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::End()
{
	m_SwapChain->Present(1, 0);
}

//深度バッファへの書き込みを切り替える
void Renderer::SetDepthEnable(bool enable)
{
	if (enable)
		m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);
	else
		m_DeviceContext->OMSetDepthStencilState(m_DepthStateDisable, 0);
}

//加算合成を一時的に切り替える
void Renderer::SetAddBlendEnable(bool enable)
{
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (enable)
		m_DeviceContext->OMSetBlendState(m_BlendStateAdd, blendFactor, 0xffffffff);
	else
		m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff);
}

//テクスチャのV方向を端で固定するか切り替える
void Renderer::SetTextureVClamp(bool enable)
{
	ID3D11SamplerState* samplerState = enable ? m_SamplerStateVClamp : m_SamplerStateWrap;
	m_DeviceContext->PSSetSamplers(0, 1, &samplerState);
}

void Renderer::SetWorldViewProjection2D()
{
	SetWorldMatrix(XMMatrixIdentity());
	SetViewMatrix(XMMatrixIdentity());

	const XMMATRIX projection = XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f);
	SetProjectionMatrix(projection);
}

void Renderer::SetWorldMatrix(XMMATRIX worldMatrix)
{
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(worldMatrix));
	m_DeviceContext->UpdateSubresource(m_WorldBuffer, 0, nullptr, &world, 0, 0);
}

void Renderer::SetViewMatrix(XMMATRIX viewMatrix)
{
	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(viewMatrix));
	m_DeviceContext->UpdateSubresource(m_ViewBuffer, 0, nullptr, &view, 0, 0);
}

void Renderer::SetProjectionMatrix(XMMATRIX projectionMatrix)
{
	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixTranspose(projectionMatrix));
	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer, 0, nullptr, &projection, 0, 0);
}

void Renderer::SetCameraPosition(Vector3 cameraPosition)
{
	const XMFLOAT4 position = XMFLOAT4(
		cameraPosition.x,
		cameraPosition.y,
		cameraPosition.z,
		0.0f);
	m_DeviceContext->UpdateSubresource(m_CameraBuffer, 0, nullptr, &position, 0, 0);
}

void Renderer::SetMaterial(MATERIAL material)
{
	m_DeviceContext->UpdateSubresource(m_MaterialBuffer, 0, nullptr, &material, 0, 0);
}

void Renderer::SetLight(LIGHT light)
{
	m_CurrentLight = light;
	m_DeviceContext->UpdateSubresource(m_LightBuffer, 0, nullptr, &m_CurrentLight, 0, 0);
}

//シェーダーで使用するUVスクロール量を設定
void Renderer::SetUVOffset(float u, float v)
{
	const XMFLOAT4 uvOffset = XMFLOAT4(u, v, 0.0f, 0.0f);
	m_DeviceContext->UpdateSubresource(m_UVScrollBuffer, 0, nullptr, &uvOffset, 0, 0);
}

void Renderer::SetCullBack()
{
	m_DeviceContext->RSSetState(m_RasterizerStateCullBack);
}

void Renderer::SetCullNone()
{
	m_DeviceContext->RSSetState(m_RasterizerStateCullNone);
}

void Renderer::CreateVertexShader(ID3D11VertexShader** vertexShader,
	ID3D11InputLayout** vertexLayout, const char* fileName)
{
	FILE* file = fopen(fileName, "rb");
	assert(file);

	const long fileSize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fileSize];
	fread(buffer, fileSize, 1, file);
	fclose(file);

	m_Device->CreateVertexShader(buffer, fileSize, nullptr, vertexShader);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 10, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	const UINT elementCount = ARRAYSIZE(layout);

	m_Device->CreateInputLayout(
		layout,
		elementCount,
		buffer,
		fileSize,
		vertexLayout);

	delete[] buffer;
}

void Renderer::CreatePixelShader(ID3D11PixelShader** pixelShader, const char* fileName)
{
	FILE* file = fopen(fileName, "rb");
	assert(file);

	const long fileSize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fileSize];
	fread(buffer, fileSize, 1, file);
	fclose(file);

	m_Device->CreatePixelShader(buffer, fileSize, nullptr, pixelShader);

	delete[] buffer;
}