struct Input
{
	float x : POSITION0;
	float y : POSITION1;
	float z : POSITION2;
};

struct Output
{
	float4 Position : SV_Position;
};

Output main(Input input)
{
	Output output;
	output.Position = float4(input.x, input.y, input.z, 1.0f);

	return output;
}