
#include "common.hlsl"


void main(in VS_IN In, out PS_IN Out)
{
	Out = (PS_IN)0;

	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);

	Out.Position = mul(In.Position, wvp);
	Out.TexCoord = In.TexCoord;
	Out.Diffuse = In.Diffuse * Material.Diffuse;
    Out.Depth = Out.Position.z;
}

