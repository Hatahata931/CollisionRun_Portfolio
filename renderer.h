//-----------------------------------------------------------
//  [ renderer.h ]
//  DirectX11の描画基盤
//-----------------------------------------------------------
#pragma once

struct VERTEX_3D
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Diffuse;
	XMFLOAT2 TexCoord;
};

struct MATERIAL
{
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT4 Emission;
	float Shininess;
	BOOL TextureEnable;
	float Dummy[2];
};

struct LIGHT
{
	BOOL Enable;
	BOOL Dummy[3];
	XMFLOAT4 Direction;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Ambient;
};

class Renderer
{
private:
	static D3D_FEATURE_LEVEL m_FeatureLevel;

	static ID3D11Device* m_Device;
	static ID3D11DeviceContext* m_DeviceContext;
	static IDXGISwapChain* m_SwapChain;
	static ID3D11RenderTargetView* m_RenderTargetView;
	static ID3D11DepthStencilView* m_DepthStencilView;

	static ID3D11Buffer* m_WorldBuffer;
	static ID3D11Buffer* m_ViewBuffer;
	static ID3D11Buffer* m_ProjectionBuffer;
	static ID3D11Buffer* m_MaterialBuffer;
	static ID3D11Buffer* m_LightBuffer;
	static ID3D11Buffer* m_CameraBuffer;
	static ID3D11Buffer* m_UVScrollBuffer;
	static LIGHT m_CurrentLight;

	static ID3D11DepthStencilState* m_DepthStateEnable;
	static ID3D11DepthStencilState* m_DepthStateDisable;
	static ID3D11SamplerState* m_SamplerStateWrap;
	static ID3D11SamplerState* m_SamplerStateVClamp;

	static ID3D11BlendState* m_BlendState;
	static ID3D11BlendState* m_BlendStateAdd;

	static ID3D11RasterizerState* m_RasterizerStateCullBack;
	static ID3D11RasterizerState* m_RasterizerStateCullNone;

public:
	static void Init();
	static void Uninit();
	static void Begin();
	static void End();

	//深度バッファへの書き込みを切り替える
	static void SetDepthEnable(bool enable);
	//加算合成を一時的に切り替える
	static void SetAddBlendEnable(bool enable);
	//テクスチャのV方向を端で固定するか切り替える
	static void SetTextureVClamp(bool enable);

	static void SetWorldViewProjection2D();
	static void SetWorldMatrix(XMMATRIX worldMatrix);
	static void SetViewMatrix(XMMATRIX viewMatrix);
	static void SetProjectionMatrix(XMMATRIX projectionMatrix);
	static void SetCameraPosition(Vector3 cameraPosition);
	static void SetMaterial(MATERIAL material);
	static void SetLight(LIGHT light);
	static LIGHT GetLight() { return m_CurrentLight; }
	//シェーダーで使用するUVスクロール量を設定
	static void SetUVOffset(float u, float v);
	static void SetCullBack();
	static void SetCullNone();

	static ID3D11Device* GetDevice() { return m_Device; }
	static ID3D11DeviceContext* GetDeviceContext() { return m_DeviceContext; }

	static void CreateVertexShader(ID3D11VertexShader** vertexShader,
		ID3D11InputLayout** vertexLayout,const char* fileName);
	static void CreatePixelShader(ID3D11PixelShader** pixelShader, const char* fileName);
};