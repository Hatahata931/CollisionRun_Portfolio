
#include "common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
	Out = (PS_IN)0;

    //ここで頂点変換
    //頂点座標を出力
    //頂点変換処理 この処理は必ず必要
    matrix wvp; //行列変数を作成
    wvp = mul(World, View); //wvp = ワールド行列 * カメラ行列
    wvp = mul(wvp, Projection); //wvp = wvp * プロジェクション行列
    
    //頂点法線をワールド行列で回転させる(頂点と同じ回転をさせる)
    float4 worldNormal, normal;
    normal = float4(In.Normal.xyz, 0.0f); //入力法線ベクトルのwを0としてコピー(平行移動しないため)
    worldNormal = mul(normal, World); //コピーされた法線をワールド行列で回転する
    worldNormal = normalize(worldNormal); //回転後の法線を正規化する
    
    float3 lightDirection = normalize(Light.Direction.xyz);
    
    //明るさの計算
    //光ベクトルと法線の内積  XYZ要素のみで計算
    float light = -dot(lightDirection, worldNormal.xyz);
    light = saturate(light); //明るさを0と1の間で飽和化する
    
    Out.Diffuse = In.Diffuse * Material.Diffuse * light * Light.Diffuse;
    Out.Diffuse += In.Diffuse * Material.Ambient * Light.Ambient;
    Out.Diffuse += Material.Emission;
    Out.Diffuse.a = In.Diffuse.a * Material.Diffuse.a;
    
    Out.Position = mul(In.Position, wvp); //変換結果を出力する
    //受け取ったこの頂点のUV座標をそのまま出力
    Out.TexCoord = In.TexCoord;
}
