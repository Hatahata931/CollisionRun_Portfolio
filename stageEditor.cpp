//-----------------------------------------------------------
//  [ stageEditor.cpp ]
//  ImGuiを使用してステージオブジェクトを配置・編集するエディター
//-----------------------------------------------------------
#include "main.h"
#include "stageEditor.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

#include "imgui.h"
#include "external/imguizmo/ImGuizmo.h"

#include "camera.h"
#include "gameObject.h"
#include "imguiManager.h"
#include "input.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"
#include "scene.h"
#include "stageBase.h"
#include "stageObjectCatalog.h"

namespace
{
	//エディターで指定できるスケールの限界値
	constexpr float MinimumScale = 0.1f;
	constexpr float MaximumScale = 100.0f;

	constexpr float GimbalLockEpsilon = 0.0001f; //ジンバルロック判定に使う許容誤差
	constexpr float MinimumSelectionHalfSize = 0.03f; //薄いモデルにも保証する選択用OBBの最小半サイズ
	constexpr float RayParallelEpsilon = 0.000001f; //レイとOBB軸が平行か判断する許容誤差

	//選択判定で使うレイ
	struct SelectionRay
	{
		Vector3 Origin;
		Vector3 Direction;
	};

	//選択対象を囲むOBBの情報
	struct SelectionBounds
	{
		Vector3 Center;
		Vector3 Axis[3];
		Vector3 HalfSize;
	};

	//マウス位置をワールド空間のレイへ変換
	bool TryCreateSelectionRay(const Camera* camera, SelectionRay& ray)
	{
		if (!camera)
			return false;

		//マウス位置取得
		const int mouseX = Input::GetMouseX();
		const int mouseY = Input::GetMouseY();
		if (mouseX < 0 || mouseX >= SCREEN_WIDTH || mouseY < 0 || mouseY >= SCREEN_HEIGHT)
			return false;

		//カメラ行列取得
		const XMMATRIX projection = camera->GetProjectionMatrix();
		const XMMATRIX view = camera->GetViewMatrix();
		const XMMATRIX world = XMMatrixIdentity();
		const float viewportWidth = static_cast<float>(SCREEN_WIDTH);
		const float viewportHeight = static_cast<float>(SCREEN_HEIGHT);

		//ワールド座標に変換
		const XMVECTOR nearPoint = XMVector3Unproject(
			XMVectorSet(static_cast<float>(mouseX), static_cast<float>(mouseY), 0.0f, 1.0f),
			0.0f, 0.0f, viewportWidth, viewportHeight, 0.0f, 1.0f,
			projection, view, world);
		const XMVECTOR farPoint = XMVector3Unproject(
			XMVectorSet(static_cast<float>(mouseX), static_cast<float>(mouseY), 1.0f, 1.0f),
			0.0f, 0.0f, viewportWidth, viewportHeight, 0.0f, 1.0f,
			projection, view, world);

		XMFLOAT3 nearPosition;
		XMFLOAT3 farPosition;
		XMStoreFloat3(&nearPosition, nearPoint);
		XMStoreFloat3(&farPosition, farPoint);

		ray.Origin = { nearPosition.x, nearPosition.y, nearPosition.z };
		ray.Direction = Vector3(farPosition.x, farPosition.y, farPosition.z) - ray.Origin;
		if (ray.Direction.length() <= RayParallelEpsilon)
			return false;

		ray.Direction.normalize();
		return true;
	}

	//描画モデルを優先し、なければコライダーから選択用OBBを取得
	bool TryGetSelectionBounds(GameObject* object, SelectionBounds& bounds)
	{
		if (!object)
			return false;

		auto* modelRenderer = object->GetComponent<ModelRendererComponent>();
		Vector3 localCenter;
		Vector3 localHalfSize;

		//モデル境界を取得
		if (modelRenderer && modelRenderer->GetLocalBounds(localCenter, localHalfSize))
		{
			bounds.Axis[0] = object->GetRight();
			bounds.Axis[1] = object->GetUp();
			bounds.Axis[2] = object->GetForward();

			//ワールド座標へ変換
			const Vector3 scale = object->GetScale();
			bounds.Center = object->GetPosition() +
				bounds.Axis[0] * (localCenter.x * scale.x) +
				bounds.Axis[1] * (localCenter.y * scale.y) +
				bounds.Axis[2] * (localCenter.z * scale.z);
			bounds.HalfSize = {
				std::max(std::fabs(localHalfSize.x * scale.x), MinimumSelectionHalfSize),
				std::max(std::fabs(localHalfSize.y * scale.y), MinimumSelectionHalfSize),
				std::max(std::fabs(localHalfSize.z * scale.z), MinimumSelectionHalfSize)
			};
			return true;
		}

		//モデル境界を取得できなかった場合、既存のコライダーを取得
		auto* collider = object->GetComponent<OBBColliderComponent>();
		if (!collider)
			return false;

		collider->GetWorldOBB(bounds.Center, bounds.Axis, bounds.HalfSize);
		bounds.HalfSize.x = std::max(bounds.HalfSize.x, MinimumSelectionHalfSize);
		bounds.HalfSize.y = std::max(bounds.HalfSize.y, MinimumSelectionHalfSize);
		bounds.HalfSize.z = std::max(bounds.HalfSize.z, MinimumSelectionHalfSize);
		return true;
	}

	//マウスから飛ばしたレイがオブジェクトを囲むOBBに当たるか調べる
	bool IntersectsSelectionBounds(const SelectionRay& ray,
		const SelectionBounds& bounds, float& hitDistance)
	{
		const Vector3 centerToOrigin = ray.Origin - bounds.Center;
		const float halfSizes[3] = {
			bounds.HalfSize.x,
			bounds.HalfSize.y,
			bounds.HalfSize.z
		};

		float minimumDistance = 0.0f;
		float maximumDistance = std::numeric_limits<float>::max();

		//OBBの各軸にレイを投影
		for (int i = 0; i < 3; ++i)
		{
			const float originOnAxis = Vector3::dot(centerToOrigin, bounds.Axis[i]);
			const float directionOnAxis = Vector3::dot(ray.Direction, bounds.Axis[i]);
			const float halfSize = halfSizes[i];

			if (std::fabs(directionOnAxis) <= RayParallelEpsilon)
			{
				if (originOnAxis < -halfSize || originOnAxis > halfSize)
					return false;
				continue;
			}

			float nearDistance = (-halfSize - originOnAxis) / directionOnAxis;
			float farDistance = (halfSize - originOnAxis) / directionOnAxis;
			if (nearDistance > farDistance)
				std::swap(nearDistance, farDistance);

			minimumDistance = std::max(minimumDistance, nearDistance);
			maximumDistance = std::min(maximumDistance, farDistance);
			if (minimumDistance > maximumDistance)
				return false;
		}

		hitDistance = minimumDistance;
		return true;
	}

	//値をasinの有効範囲へ収める
	float ClampUnit(float value)
	{
		if (value < -1.0f)
			return -1.0f;
		if (value > 1.0f)
			return 1.0f;
		return value;
	}

	//拡大率をエディターの数値入力欄と同じ範囲へ収める
	float ClampScale(float value)
	{
		if (value < MinimumScale)
			return MinimumScale;
		if (value > MaximumScale)
			return MaximumScale;
		return value;
	}

	//ワールド行列を分解し、拡大率とクォータニオン形式の回転を返す
	bool TryDecomposeWorld(const XMFLOAT4X4& world, XMVECTOR& outScale, XMVECTOR& outRotation)
	{
		XMVECTOR translation;
		return XMMatrixDecompose(
			&outScale,
			&outRotation,
			&translation,
			XMLoadFloat4x4(&world));
	}

	//ワールド行列からXMMatrixRotationRollPitchYawと同じ順序の回転を取り出す
	bool TryExtractRollPitchYaw(const XMFLOAT4X4& world, Vector3& outRotation)
	{
		//拡大率や平行移動が混ざったワールド行列から、純粋な回転だけをクォータニオンで取得する
		XMVECTOR scaleVector;
		XMVECTOR rotationQuaternion;
		if (!TryDecomposeWorld(world, scaleVector, rotationQuaternion))
			return false;

		XMFLOAT4X4 matrix;
		XMStoreFloat4x4(&matrix, XMMatrixRotationQuaternion(rotationQuaternion));

		const float pitch = asinf(ClampUnit(-matrix._32));
		const float cosPitch = cosf(pitch);

		float yaw;
		float roll;
		if (fabsf(cosPitch) > GimbalLockEpsilon)
		{
			yaw = atan2f(matrix._31, matrix._33);
			roll = atan2f(matrix._12, matrix._22);
		}
		else
		{
			//ジンバルロック時はRollを0に固定し、同じ姿勢になるYawを採用する
			yaw = atan2f(-matrix._13, matrix._11);
			roll = 0.0f;
		}

		outRotation = { pitch, yaw, roll };
		return true;
	}

	//ワールド行列からエディターで使用できる範囲の拡大率を取り出す
	bool TryExtractScale(const XMFLOAT4X4& world, Vector3& outScale)
	{
		XMVECTOR scaleVector;
		XMVECTOR rotationQuaternion;
		if (!TryDecomposeWorld(world, scaleVector, rotationQuaternion))
			return false;

		XMFLOAT3 extractedScale;
		XMStoreFloat3(&extractedScale, scaleVector);

		//0以下の拡大率による潰れや反転を防ぎ、数値入力欄と同じ範囲に収める
		outScale = {
			ClampScale(extractedScale.x),
			ClampScale(extractedScale.y),
			ClampScale(extractedScale.z)
		};
		return true;
	}
}

bool StageEditor::m_IsOpen = true;
int StageEditor::m_SelectedAssetIndex = static_cast<int>(StageObjectCatalog::FindIndex("Crate"));
float StageEditor::m_Position[3] = { 0.0f, 1.0f, 0.0f };
float StageEditor::m_RotationDegrees[3] = { 0.0f, 0.0f, 0.0f };
float StageEditor::m_Scale[3] = { 1.0f, 1.0f, 1.0f };
float StageEditor::m_RotationVelocityDegrees[3] = { 0.0f, 0.0f, 0.0f };
char StageEditor::m_FilePath[260]{};
std::string StageEditor::m_StatusMessage;
Scene* StageEditor::m_CurrentScene = nullptr;
int StageEditor::m_SelectedObjectIndex = -1;
StageEditor::GizmoOperation StageEditor::m_GizmoOperation = StageEditor::GizmoOperation::Move;

void StageEditor::Update(Scene* scene)
{
	StageBase* stage = dynamic_cast<StageBase*>(scene);

	//前回受け取ったSceneと違っていたらシーン遷移と判断
	if (m_CurrentScene != scene)
	{
		m_CurrentScene = scene;
		m_SelectedObjectIndex = -1;
		m_StatusMessage.clear();

		//シーン固有の保存先へ切り替える
		m_FilePath[0] = '\0';
		if (stage)
		{
			const char* filePath = stage->GetStageObjectDataPath();
			if (filePath)
				snprintf(m_FilePath, IM_ARRAYSIZE(m_FilePath), "%s", filePath);
		}
	}

	if (Input::GetKeyTrigger(VK_F2))
		m_IsOpen = !m_IsOpen;

	//配置データを管理できるStageBaseとしてエディターへ渡す
	if (m_IsOpen)
		DrawEditorWindow(stage);
}

//ImGuiの入力値からStageObjectDataを組み立てる
StageObjectData StageEditor::MakeObjectData()
{
	StageObjectData data;
	//選択番号に対応するカタログのIDを保存データへ設定
	const std::vector<StageObjectDefinition>& definitions = StageObjectCatalog::GetAll();
	const StageObjectDefinition* definition = nullptr;
	if (m_SelectedAssetIndex >= 0 &&
		static_cast<size_t>(m_SelectedAssetIndex) < definitions.size())
	{
		definition = &definitions[m_SelectedAssetIndex];
		data.AssetId = definition->Id;
	}

	data.Position = { m_Position[0], m_Position[1], m_Position[2] };
	data.Rotation = {
		XMConvertToRadians(m_RotationDegrees[0]),
		XMConvertToRadians(m_RotationDegrees[1]),
		XMConvertToRadians(m_RotationDegrees[2])
	};
	data.Scale = { m_Scale[0], m_Scale[1], m_Scale[2] };
	if (definition && definition->Kind == StageObjectKind::Box)
	{
		data.RotationVelocity = {
			XMConvertToRadians(m_RotationVelocityDegrees[0]),
			XMConvertToRadians(m_RotationVelocityDegrees[1]),
			XMConvertToRadians(m_RotationVelocityDegrees[2])
		};
	}
	return data;
}

//配置データをImGuiの入力欄へ反映
void StageEditor::SetInputValues(const StageObjectData& data)
{
	//配置物のIDからカタログ上の選択番号を復元
	const size_t assetIndex = StageObjectCatalog::FindIndex(data.AssetId);
	if (assetIndex < StageObjectCatalog::GetAll().size())
		m_SelectedAssetIndex = static_cast<int>(assetIndex);

	m_Position[0] = data.Position.x;
	m_Position[1] = data.Position.y;
	m_Position[2] = data.Position.z;
	m_RotationDegrees[0] = XMConvertToDegrees(data.Rotation.x);
	m_RotationDegrees[1] = XMConvertToDegrees(data.Rotation.y);
	m_RotationDegrees[2] = XMConvertToDegrees(data.Rotation.z);
	m_Scale[0] = data.Scale.x;
	m_Scale[1] = data.Scale.y;
	m_Scale[2] = data.Scale.z;
	m_RotationVelocityDegrees[0] = XMConvertToDegrees(data.RotationVelocity.x);
	m_RotationVelocityDegrees[1] = XMConvertToDegrees(data.RotationVelocity.y);
	m_RotationVelocityDegrees[2] = XMConvertToDegrees(data.RotationVelocity.z);
}

//選択中の配置オブジェクトが有効か確認
bool StageEditor::HasValidSelection(const StageBase* stage)
{
	return stage && m_SelectedObjectIndex >= 0 &&
		static_cast<size_t>(m_SelectedObjectIndex) < stage->GetStageObjects().size();
}

void StageEditor::SelectObject(int objectIndex, const StageObjectData& data)
{
	m_SelectedObjectIndex = objectIndex;
	SetInputValues(data);
}

//マウス位置から最も手前にある配置オブジェクトを選択
void StageEditor::SelectObjectAtMouse(StageBase* stage, const Camera* camera)
{
	SelectionRay ray;
	if (!TryCreateSelectionRay(camera, ray))
		return;

	const std::vector<StageObjectInstance>& objects = stage->GetStageObjects();
	int nearestObjectIndex = -1;
	float nearestDistance = std::numeric_limits<float>::max();

	for (size_t i = 0; i < objects.size(); ++i)
	{
		GameObject* object = objects[i].Object;
		SelectionBounds bounds;
		if (!TryGetSelectionBounds(object, bounds))
			continue;

		//より近いものが見つかったら更新
		float hitDistance;
		if (IntersectsSelectionBounds(ray, bounds, hitDistance) &&
			hitDistance < nearestDistance)
		{
			nearestDistance = hitDistance;
			nearestObjectIndex = static_cast<int>(i);
		}
	}

	if (nearestObjectIndex >= 0)
	{
		const StageObjectData& data = objects[static_cast<size_t>(nearestObjectIndex)].Data;
		SelectObject(nearestObjectIndex, data);
	}
	else
		m_SelectedObjectIndex = -1; //何もない場所をクリックで選択解除
}

//選択中の配置オブジェクトを複製
void StageEditor::DuplicateSelected(StageBase* stage)
{
	if (!HasValidSelection(stage))
	{
		m_StatusMessage = "Select an object to duplicate.";
		return;
	}

	//配置データをコピー
	const StageObjectData duplicateData =
		stage->GetStageObjects()[static_cast<size_t>(m_SelectedObjectIndex)].Data;

	//複製
	if (!stage->CreateStageObject(duplicateData))
	{
		m_StatusMessage = "Failed to duplicate.";
		return;
	}

	//複製されたオブジェクトを選択状態にする
	m_SelectedObjectIndex = static_cast<int>(stage->GetStageObjects().size()) - 1;
	SetInputValues(duplicateData);
	m_GizmoOperation = GizmoOperation::Move;
	m_StatusMessage = "Duplicated.";
}

//カタログに登録された配置アセットの選択欄を表示
void StageEditor::DrawAssetSelector()
{
	const std::vector<StageObjectDefinition>& definitions = StageObjectCatalog::GetAll();
	//登録アセットがなければ選択欄を作れない
	if (definitions.empty())
	{
		ImGui::TextUnformatted("No stage assets are registered.");
		return;
	}

	//カタログ変更などで選択番号が範囲外になった場合は先頭へ戻す
	if (m_SelectedAssetIndex < 0 || static_cast<size_t>(m_SelectedAssetIndex) >= definitions.size())
		m_SelectedAssetIndex = 0;

	//カタログの表示名から選択用コンボボックスを作成
	const char* preview = definitions[m_SelectedAssetIndex].DisplayName;
	if (ImGui::BeginCombo("Asset", preview))
	{
		for (size_t i = 0; i < definitions.size(); ++i)
		{
			const bool isSelected = m_SelectedAssetIndex == static_cast<int>(i);
			ImGui::PushID(static_cast<int>(i));
			if (ImGui::Selectable(definitions[i].DisplayName, isSelected) && !isSelected)
			{
				m_SelectedAssetIndex = static_cast<int>(i);
				const Vector3& velocity = definitions[i].DefaultRotationVelocity;
				m_RotationVelocityDegrees[0] = XMConvertToDegrees(velocity.x);
				m_RotationVelocityDegrees[1] = XMConvertToDegrees(velocity.y);
				m_RotationVelocityDegrees[2] = XMConvertToDegrees(velocity.z);
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
}

//Move・Rotate・Scaleの選択欄を表示
void StageEditor::DrawGizmoControls()
{
	ImGui::SeparatorText("Gizmo");

	const bool isMove = m_GizmoOperation == GizmoOperation::Move;
	if (ImGui::RadioButton("Move", isMove))
		m_GizmoOperation = GizmoOperation::Move;

	ImGui::SameLine();
	const bool isRotate = m_GizmoOperation == GizmoOperation::Rotate;
	if (ImGui::RadioButton("Rotate", isRotate))
		m_GizmoOperation = GizmoOperation::Rotate;

	ImGui::SameLine();
	const bool isScale = m_GizmoOperation == GizmoOperation::Scale;
	if (ImGui::RadioButton("Scale", isScale))
		m_GizmoOperation = GizmoOperation::Scale;
}

//エディターウィンドウを描画
void StageEditor::DrawEditorWindow(StageBase* stage)
{
	ImGui::SetNextWindowSize(ImVec2(380.0f, 620.0f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Stage Editor", &m_IsOpen))
	{
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("F2: Show / Hide");
	ImGui::Separator();

	//StageBaseを継承していないシーンの場合終了
	if (!stage)
	{
		ImGui::TextUnformatted("Stage editor is available in StageBase scenes.");
		ImGui::End();
		return;
	}

	DrawAssetSelector();
	ImGui::DragFloat3("Position", m_Position, 0.1f);
	ImGui::DragFloat3("Rotation (deg)", m_RotationDegrees, 1.0f);
	ImGui::DragFloat3("Scale", m_Scale, 0.1f, MinimumScale, MaximumScale);
	const std::vector<StageObjectDefinition>& definitions = StageObjectCatalog::GetAll();
	if (m_SelectedAssetIndex >= 0 &&
		static_cast<size_t>(m_SelectedAssetIndex) < definitions.size() &&
		definitions[m_SelectedAssetIndex].Kind == StageObjectKind::Box)
	{
		ImGui::DragFloat3("Rotation Velocity (deg/frame)", m_RotationVelocityDegrees, 0.05f);
	}
	DrawGizmoControls();

	//入力内容から新しいオブジェクトを生成
	if (ImGui::Button("Create", ImVec2(-1.0f, 0.0f)))
	{
		const StageObjectData data = MakeObjectData();
		if (stage->CreateStageObject(data))
		{
			m_SelectedObjectIndex = static_cast<int>(stage->GetStageObjects().size()) - 1;
			m_StatusMessage = "Created.";
		}
		else
			m_StatusMessage = "Failed to create.";
	}

	//選択中のオブジェクトへ入力内容を反映
	if (ImGui::Button("Apply Selected", ImVec2(-1.0f, 0.0f)))
	{
		if (HasValidSelection(stage) && stage->UpdateStageObject(static_cast<size_t>(m_SelectedObjectIndex), MakeObjectData()))
			m_StatusMessage = "Applied.";
		else
			m_StatusMessage = "Select an object to apply.";
	}

	//選択中のオブジェクトを同じTransformで複製
	if (ImGui::Button("Duplicate Selected", ImVec2(-1.0f, 0.0f)))
		DuplicateSelected(stage);

	//選択中のオブジェクトを削除
	if (ImGui::Button("Delete Selected", ImVec2(-1.0f, 0.0f)))
	{
		if (HasValidSelection(stage) && stage->DeleteStageObject(static_cast<size_t>(m_SelectedObjectIndex)))
		{
			m_SelectedObjectIndex = -1;
			m_StatusMessage = "Deleted.";
		}
		else
			m_StatusMessage = "Select an object to delete.";
	}

	ImGui::SeparatorText("Stage Data");
	ImGui::InputText("File", m_FilePath, IM_ARRAYSIZE(m_FilePath));

	if (ImGui::Button("Save", ImVec2(-1.0f, 0.0f)))
		stage->SaveStageObjects(m_FilePath, &m_StatusMessage);

	//追加生成ではなく、JSONで管理している配置だけを置き換える
	if (ImGui::Button("Reload", ImVec2(-1.0f, 0.0f)))
	{
		if (stage->LoadStageObjects(m_FilePath, &m_StatusMessage))
			m_SelectedObjectIndex = -1;
	}

	if (!m_StatusMessage.empty())
		ImGui::TextUnformatted(m_StatusMessage.c_str());

	ImGui::SeparatorText("Stage Objects");
	const std::vector<StageObjectInstance>& objects = stage->GetStageObjects();
	ImGui::Text("Count: %d", static_cast<int>(objects.size()));

	//管理中オブジェクトを選択可能な一覧として表示
	if (ImGui::BeginChild("CreatedObjectList", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders))
	{
		for (size_t i = 0; i < objects.size(); ++i)
		{
			const StageObjectData& data = objects[i].Data;
			//保存用IDを人が読みやすい表示名へ変換
			const StageObjectDefinition* definition = StageObjectCatalog::Find(data.AssetId);
			const char* displayName = definition ? definition->DisplayName : data.AssetId.c_str();

			char label[160];
			snprintf(
				label,
				IM_ARRAYSIZE(label),
				"%d: %s  (%.1f, %.1f, %.1f)",
				static_cast<int>(i + 1),
				displayName,
				data.Position.x,
				data.Position.y,
				data.Position.z);

			ImGui::PushID(static_cast<int>(i));
			if (ImGui::Selectable(label, m_SelectedObjectIndex == static_cast<int>(i)))
				SelectObject(static_cast<int>(i), data);
			ImGui::PopID();
		}
	}

	ImGui::EndChild();
	ImGui::End();
}

//選択中の配置オブジェクトへギズモ操作を反映
bool StageEditor::UpdateSelectedGizmo(StageBase* stage, const Camera* camera)
{
	if (!HasValidSelection(stage))
		return false;

	//一覧で選択中の配置データと実物を取得
	const std::vector<StageObjectInstance>& objects = stage->GetStageObjects();
	const StageObjectInstance& instance = objects[static_cast<size_t>(m_SelectedObjectIndex)];
	if (!instance.Object)
		return false;

	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&view, camera->GetViewMatrix());
	XMStoreFloat4x4(&projection, camera->GetProjectionMatrix());
	XMStoreFloat4x4(&world, instance.Object->GetTransform()->GetWorldMatrix());

	const ImGuiIO& io = ImGui::GetIO();
	if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
		return false;

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetRect(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y);
	//配置物ごとに操作状態が混ざらないよう固有IDを設定
	ImGuizmo::PushID(m_SelectedObjectIndex + 1);

	//操作ごとにImGuizmoの種類と基準にする座標系を決める
	ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::WORLD;
	switch (m_GizmoOperation)
	{
	case GizmoOperation::Move:
		//移動はステージ全体の軸に沿わせる
		operation = ImGuizmo::TRANSLATE;
		mode = ImGuizmo::WORLD;
		break;
	case GizmoOperation::Rotate:
		//回転はオブジェクト自身の軸に沿わせる
		operation = ImGuizmo::ROTATE;
		mode = ImGuizmo::LOCAL;
		break;
	case GizmoOperation::Scale:
		//拡大縮小もオブジェクト自身の軸に沿わせる
		operation = ImGuizmo::SCALE;
		mode = ImGuizmo::LOCAL;
		break;
	}

	const bool manipulated = ImGuizmo::Manipulate(&view._11,
		&projection._11, operation, mode, &world._11);
	//マウスがギズモ上にあるか、ギズモをドラッグ操作していたらtrue
	const bool blocksSelection = ImGuizmo::IsOver() || ImGuizmo::IsUsing();

	ImGuizmo::PopID();
	if (!manipulated)
		return blocksSelection;

	//操作していないTransform要素を維持するため、現在の配置データを基準にする
	StageObjectData updatedData = instance.Data;
	if (m_GizmoOperation == GizmoOperation::Move)
	{
		//移動
		updatedData.Position = { world._41, world._42, world._43 };
	}
	else if (m_GizmoOperation == GizmoOperation::Rotate)
	{
		//回転
		Vector3 rotation;
		if (!TryExtractRollPitchYaw(world, rotation))
		{
			m_StatusMessage = "Failed to decompose rotation.";
			return blocksSelection;
		}
		//StageObjectDataの回転値は度数法ではなくラジアンで保持
		updatedData.Rotation = rotation;
	}
	else
	{
		//拡縮
		Vector3 scale;
		if (!TryExtractScale(world, scale))
		{
			m_StatusMessage = "Failed to decompose scale.";
			return blocksSelection;
		}
		updatedData.Scale = scale;
	}

	//実物・保存データ・数値入力欄を同じTransformへ同期
	if (stage->UpdateStageObject(static_cast<size_t>(m_SelectedObjectIndex), updatedData))
	{
		SetInputValues(updatedData);
		m_StatusMessage = "Modified.";
	}

	return blocksSelection;
}

//シーン更新後のカメラ行列を使ってギズモ操作とクリック選択を処理
void StageEditor::UpdateSceneInteraction(Scene* scene)
{
	if (!m_IsOpen || !scene)
		return;

	//JSON管理の配置物を持つステージのみを対象にする
	StageBase* stage = dynamic_cast<StageBase*>(scene);
	if (!stage)
		return;

	//シーン更新後に確定した描画用カメラ行列を取得
	Camera* camera = scene->GetGameObject<Camera>();
	if (!camera)
		return;

	const bool gizmoBlocksSelection = UpdateSelectedGizmo(stage, camera);
	if (!Input::GetKeyTrigger(VK_LBUTTON) || ImGuiManager::WantsMouseCapture() || gizmoBlocksSelection)
		return;

	SelectObjectAtMouse(stage, camera);
}