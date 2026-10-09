//-----------------------------------------------------------
//  [ texture.h ]
//  テクスチャの読み込みとキャッシュ管理
//-----------------------------------------------------------
#pragma once

#include <string>
#include <unordered_map>

//テクスチャ読み込みモード
enum class TextureLoadMode
{
	Default,
	IgnoreSRGB, //sRGB無視(2D画像用)
};

class Texture
{
private:
	static std::unordered_map<std::string, ID3D11ShaderResourceView*> m_TexturePool;
	static std::unordered_map<std::string, ID3D11ShaderResourceView*> m_IgnoreSRGBTexturePool;

public:
	static ID3D11ShaderResourceView* Load(const char* texturePath,
		TextureLoadMode mode = TextureLoadMode::Default);
	static void UnloadAll();
};
