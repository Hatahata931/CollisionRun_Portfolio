//-----------------------------------------------------------
//  [ stageObjectCatalog.cpp ]
//  ステージエディターで配置できるアセットの登録情報
//-----------------------------------------------------------
#include "main.h"
#include "stageObjectCatalog.h"

#include <limits>

//登録されている全配置アセット情報を取得
const std::vector<StageObjectDefinition>& StageObjectCatalog::GetAll()
{
	//新しい配置アセットはこの一覧へ追加する
	static const std::vector<StageObjectDefinition> definitions =
	{
		{ "Cube", "Cube", StageObjectKind::Box, "asset\\model\\cube.obj", { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, {} },
		{ "RotatingBox", "Rotating Box", StageObjectKind::Box, "asset\\model\\cube.obj",
			{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, {},
			"asset\\texture\\mechanical_stone_block.png", { 0.0f, 0.005f, 0.0f } },
		{ "Crate", "Crate", StageObjectKind::Box, "asset\\model\\crate.obj", { 0.25f, 0.25f, 0.25f }, { 0.0f, 0.0f, 0.0f }, {} },
		{ "Fence", "Fence", StageObjectKind::Box, "asset\\model\\fence.obj", { 0.475f, 0.15f, 0.1f }, { 0.0f, 0.15f, 0.0f }, {} },
		{ "FenceSegment", "Fence Segment", StageObjectKind::Box, "asset\\model\\fence_segment.obj", { 0.475f, 0.15f, 0.1f }, { 0.0f, 0.15f, 0.0f }, {} },
		{ "Tree", "Tree", StageObjectKind::Box, "asset\\model\\tree-pine.obj", { 0.25f, 0.95f, 0.25f }, { 0.0f, 0.95f, 0.0f }, {} },
		{ "PlatformRamp", "Platform Ramp", StageObjectKind::Box, "asset\\model\\platform-ramp.obj", { 0.5f, 0.08f, 0.55f }, { 0.0f, 0.0f, 0.0f }, {} },
		{ "Arrow", "Arrow Right", StageObjectKind::Box, "asset\\model\\arrow.obj", { 0.23f, 0.28f, 0.07f }, { -0.02f, 0.28f, 0.015f }, {} },
		{ "ArrowLeft", "Arrow Left", StageObjectKind::Box, "asset\\model\\arrow_left.obj", { 0.23f, 0.28f, 0.07f }, { 0.005f, 0.28f, 0.015f }, {} },
		{ "TerrainGrass", "Terrain - Grass", StageObjectKind::Terrain, "", {}, {},
			{ "asset\\model\\grass_top.png", "asset\\model\\grass_side.png",
			  "asset\\model\\dirt_side_repeat.png", "asset\\model\\grass_bottom.png" } },
		{ "BreakPlatform", "Break Platform", StageObjectKind::BreakPlatform, "", {}, {}, {} },
		{ "Checkpoint", "Checkpoint", StageObjectKind::Checkpoint, "", {}, {}, {} },
		{ "JumpPad", "Jump Pad", StageObjectKind::JumpPad, "", {}, {}, {} },
		{ "Spike", "Spike", StageObjectKind::Spike, "", {}, {}, {} },
		{ "Star", "Star", StageObjectKind::Star, "", {}, {}, {} },
		{ "Heart", "Heart", StageObjectKind::Heart, "", {}, {}, {} },
		{ "Flowers", "Flowers", StageObjectKind::Prop, "asset\\model\\flowers.obj", {}, {}, {} },
		{ "FlowersTall", "Flowers Tall", StageObjectKind::Prop, "asset\\model\\flowers-tall.obj", {}, {}, {} },
		{ "Grass", "Grass", StageObjectKind::Prop, "asset\\model\\grass.obj", {}, {}, {} },
		{ "Plant", "Plant", StageObjectKind::Prop, "asset\\model\\plant.obj", {}, {}, {} },
		{ "Mushrooms", "Mushrooms", StageObjectKind::Prop, "asset\\model\\mushrooms.obj", {}, {}, {} },
	};

	return definitions;
}

//保存用IDに一致する配置アセット情報を取得
const StageObjectDefinition* StageObjectCatalog::Find(const std::string& id)
{
	const std::vector<StageObjectDefinition>& definitions = GetAll();
	for (const StageObjectDefinition& definition : definitions)
	{
		if (id == definition.Id)
			return &definition;
	}

	//一致するモデルが登録されていない
	return nullptr;
}

//保存用IDに一致する配置アセットの登録番号を取得
std::size_t StageObjectCatalog::FindIndex(const std::string& id)
{
	const std::vector<StageObjectDefinition>& definitions = GetAll();
	for (std::size_t i = 0; i < definitions.size(); ++i)
	{
		if (id == definitions[i].Id)
			return i;
	}

	//有効な登録番号と重ならない最大値を未検出として返す
	return std::numeric_limits<std::size_t>::max();
}
