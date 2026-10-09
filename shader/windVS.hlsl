//-----------------------------------------------------------
//  [ windVS.hlsl ]
//  風線を描画する頂点シェーダー
//-----------------------------------------------------------
#include "common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
	//Windで使用しない出力要素も0で初期化する
	Out = (PS_IN)0;

	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	Out.Diffuse = In.Diffuse * Material.Diffuse;

	Out.Position = mul(In.Position, wvp);
	Out.TexCoord = In.TexCoord;
}