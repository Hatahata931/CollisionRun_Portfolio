#include "common.hlsl"

//ゼロ除算を避けるための最小値
static const float MinimumHalfSize = 0.0001f;

//面の縦横にテクスチャを何枚ずつ表示するか計算
float2 CalculateTileCount(float2 faceHalfSize)
{
	//短い辺を1枚分とし、長い辺は最も近い整数枚へ揃える
	const float minimumHalfSize = max(min(faceHalfSize.x, faceHalfSize.y), MinimumHalfSize);
	return max(round(faceHalfSize / minimumHalfSize), float2(1.0f, 1.0f));
}

//面の法線からその面上でUVに使う2軸を選ぶ
float2 CalculateTiledTexCoord(float3 localPosition, float3 localNormal, float3 boxHalfSize)
{
	const float3 normalAbs = abs(localNormal);
	float2 normalizedPosition;
	float2 faceHalfSize;

	//左右面
	if (normalAbs.x >= normalAbs.y && normalAbs.x >= normalAbs.z)
	{
		//面が向いている軸以外の2軸を使う
		normalizedPosition = float2(localPosition.z, -localPosition.y) * 0.5f + 0.5f;
		faceHalfSize = boxHalfSize.zy;
	}
	//上下面
	else if (normalAbs.y >= normalAbs.z)
	{
		normalizedPosition = float2(localPosition.x, localPosition.z) * 0.5f + 0.5f;
		faceHalfSize = boxHalfSize.xz;
	}
	//前後面
	else
	{
		normalizedPosition = float2(localPosition.x, -localPosition.y) * 0.5f + 0.5f;
		faceHalfSize = boxHalfSize.xy;
	}

	return normalizedPosition * CalculateTileCount(faceHalfSize);
}

void main(in VS_IN In, out PS_IN Out)
{
	Out = (PS_IN)0;

	matrix wvp = mul(World, View);
	wvp = mul(wvp, Projection);

	float4 normal = float4(In.Normal.xyz, 0.0f);
	float4 worldNormal = normalize(mul(normal, World));

	const float3 lightDirection = normalize(Light.Direction.xyz);
	float light = saturate(-dot(lightDirection, worldNormal.xyz));

	Out.Diffuse = In.Diffuse * Material.Diffuse * light * Light.Diffuse;
	Out.Diffuse += In.Diffuse * Material.Ambient * Light.Ambient;
	Out.Diffuse += Material.Emission;
	Out.Diffuse.a = In.Diffuse.a * Material.Diffuse.a;

	Out.Position = mul(In.Position, wvp);

	//ワールド行列の各軸の長さから、回転の影響を除いたBoxの半サイズを取得
	const float3 boxHalfSize = float3(length(World[0].xyz), length(World[1].xyz), length(World[2].xyz));
	Out.TexCoord = CalculateTiledTexCoord(In.Position.xyz, In.Normal.xyz, boxHalfSize);
}
