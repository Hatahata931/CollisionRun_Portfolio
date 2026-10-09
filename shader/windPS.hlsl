//-----------------------------------------------------------
//  [ windPS.hlsl ]
//  風線の形と透明度を作るピクセルシェーダー
//-----------------------------------------------------------
#include "common.hlsl"

static const float AlphaDiscardThreshold = 0.001f; //描画しないアルファ値の上限

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//幅の中央を濃くし、両端を滑らかに透明にする
	const float widthPosition = abs(In.TexCoord.y * 2.0f - 1.0f);
	const float widthFade = 1.0f - smoothstep(0.2f, 1.0f, widthPosition);
	//風線の始点と終点を薄くして四角形の輪郭を隠す
	const float tailFade = smoothstep(0.0f, 0.35f, In.TexCoord.x);
	const float headFade = 1.0f - smoothstep(0.88f, 1.0f, In.TexCoord.x);
	const float headBrightness = lerp(0.55f, 1.0f, smoothstep(0.15f, 0.85f, In.TexCoord.x));
	const float alpha = In.Diffuse.a * widthFade * tailFade * headFade;

	if (alpha < AlphaDiscardThreshold)
		discard;

	outDiffuse = float4(In.Diffuse.rgb * headBrightness, alpha);
}
