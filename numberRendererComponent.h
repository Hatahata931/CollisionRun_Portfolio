//-----------------------------------------------------------
//  [ numberRendererComponent.h ]
//  等間隔の数字スプライトを使った2D数値描画
//-----------------------------------------------------------
#pragma once

#include <array>
#include <d3d11.h>
#include <DirectXMath.h>

#include "objectComponent.h"

//数字全体をどの位置基準で横に揃えるか
enum class NumberTextAlign
{
	Left,
	Center,
	Right,
};

class NumberRendererComponent : public ObjectComponent
{
private:
	static constexpr int VertexCount = 4; //1文字を描画する四角形の頂点数

	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11ShaderResourceView* m_Texture = nullptr;

	//0～9をスプライトシート上のどのセルから描くかを保持する
	std::array<int, 10> m_DigitSpriteIndices{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	//スプライトシートの縦横の分割数
	int m_Columns = 10;
	int m_Rows = 1;

	//単独表示または「現在値 / 最大値」形式で使用する数値と区切り記号
	int m_PrimaryValue = 0;
	int m_SecondaryValue = 0;
	int m_SeparatorSpriteIndex = -1; //区切り記号のスプライト番号

	int m_MinimumDigits = 1; //表示する最小桁数(足りない桁は0埋め)

	//1文字の表示サイズ・文字間隔・描画基準位置
	float m_SpriteWidth = 50.0f;
	float m_SpriteHeight = 50.0f;
	float m_Spacing = 40.0f; //次の文字までのX方向の移動量
	float m_OffsetX = 0.0f;
	float m_OffsetY = 0.0f;

	NumberTextAlign m_Alignment = NumberTextAlign::Left;
	bool m_ShowSecondaryValue = false; //区切り記号と2つ目の数値を表示するか
	bool m_Visible = true;
	DirectX::XMFLOAT4 m_Color{ 1.0f, 1.0f, 1.0f, 1.0f };

	//0埋めを含む表示桁数を取得
	int GetDigitCount(int value) const;
	//最初の文字を描画するX座標を計算
	float GetStartX(int glyphCount) const;
	//数値を描画し、次の描画開始X座標を返す
	float DrawNumber(int value, float x, float y);
	//指定されたスプライト番号の1文字を描画
	void DrawSprite(int spriteIndex, float x, float y);

public:
	//ObjectComponentのコンストラクタを引き継ぐ
	using ObjectComponent::ObjectComponent;

	void Init() override {}
	void Init(const char* texturePath, int columns, int rows);
	void Uninit() override;
	void Draw() override;

	void SetTexture(const char* texturePath);
	void SetSpriteSheet(int columns, int rows);
	void SetDigitSpriteIndex(int digit, int spriteIndex); //指定した数字に対応するスプライト番号を設定
	void SetDigitSpriteIndices(const std::array<int, 10>& spriteIndices); //0～9に対応するスプライト番号をまとめて設定
	void SetSpriteSize(float width, float height);
	void SetSpacing(float spacing);
	void SetOffset(float xOffset, float yOffset);
	void SetAlignment(NumberTextAlign alignment);
	void SetMinimumDigits(int digits);
	void SetValue(int value); //数値を1つだけ表示
	void SetPairValues(int primaryValue, int secondaryValue, int separatorSpriteIndex); //2つの数値を指定した記号で区切って表示
	void SetColor(const DirectX::XMFLOAT4& color);
	void SetVisible(bool visible);
};
