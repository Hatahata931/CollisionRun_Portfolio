//-----------------------------------------------------------
//  [ texture.cpp ]
//  テクスチャの読み込みとキャッシュ管理
//-----------------------------------------------------------
#include "main.h"
#include "texture.h"

#include "renderer.h"

//補助関数
namespace
{
	using TexturePool = std::unordered_map<std::string, ID3D11ShaderResourceView*>;

	//キャッシュ内のDirectXリソースを解放して空にする
	void ReleaseTexturePool(TexturePool& texturePool)
	{
		for (auto& textureEntry : texturePool)
		{
			if (textureEntry.second != nullptr)
				textureEntry.second->Release();
		}

		texturePool.clear();
	}
}

std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture::m_TexturePool;
std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture::m_IgnoreSRGBTexturePool;

ID3D11ShaderResourceView* Texture::Load(const char* texturePath, TextureLoadMode mode)
{
	//読み込みモードごとにキャッシュを分け、先に読み込んだ形式が使い回されるのを防ぐ
	auto& texturePool = mode == TextureLoadMode::IgnoreSRGB
		? m_IgnoreSRGBTexturePool : m_TexturePool;

	const auto textureIt = texturePool.find(texturePath);
	if (textureIt != texturePool.end())
		return textureIt->second;

	wchar_t wideFileName[512]{};
	mbstowcs(wideFileName, texturePath, strlen(texturePath) + 1);

	TexMetadata metadata;
	ScratchImage image;
	ScratchImage mipChain;
	ID3D11ShaderResourceView* texture = nullptr;

	//モードによって読み込み方法を変える
	const WIC_FLAGS wicFlags = mode == TextureLoadMode::IgnoreSRGB
		? WIC_FLAGS_IGNORE_SRGB : WIC_FLAGS_NONE;
	LoadFromWICFile(wideFileName, wicFlags, &metadata, image);
	GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	CreateShaderResourceView(Renderer::GetDevice(), mipChain.GetImages(),
		mipChain.GetImageCount(), mipChain.GetMetadata(), &texture);
	assert(texture);

	texturePool.emplace(texturePath, texture);

	return texture;
}

void Texture::UnloadAll()
{
	ReleaseTexturePool(m_TexturePool);
	ReleaseTexturePool(m_IgnoreSRGBTexturePool);
}
