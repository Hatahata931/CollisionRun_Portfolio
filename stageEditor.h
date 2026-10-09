//-----------------------------------------------------------
//  [ stageEditor.h ]
//  ImGuiを使用してステージオブジェクトを配置・編集するエディター
//-----------------------------------------------------------
#pragma once

#include <string>

#include "stageObjectData.h"

class Camera;
class Scene;
class StageBase;

class StageEditor
{
private:
	//ギズモで編集するTransformの種類
	enum class GizmoOperation
	{
		Move,
		Rotate,
		Scale,
	};

	static bool m_IsOpen; //エディター画面を表示するか
	static int m_SelectedAssetIndex; //カタログで選択している配置アセット番号
	static float m_Position[3];
	static float m_RotationDegrees[3]; //エディターでは度数法
	static float m_Scale[3];
	static float m_RotationVelocityDegrees[3]; //1フレームごとの回転量
	static char m_FilePath[260]; //現在のシーンが使用する保存・読み込み先
	static std::string m_StatusMessage; //エディター操作結果の表示メッセージ
	static Scene* m_CurrentScene;
	static int m_SelectedObjectIndex; //選択中の配置オブジェクト番号
	static GizmoOperation m_GizmoOperation; //現在のギズモ操作

	//ImGuiの入力値からStageObjectDataを組み立てる
	static StageObjectData MakeObjectData();
	//配置データをImGuiの入力欄へ反映
	static void SetInputValues(const StageObjectData& data);
	//選択中の配置オブジェクトが有効か確認
	static bool HasValidSelection(const StageBase* stage);
	static void SelectObject(int objectIndex, const StageObjectData& data);
	//マウス位置から最も手前にある配置オブジェクトを選択
	static void SelectObjectAtMouse(StageBase* stage, const Camera* camera);
	//選択中の配置オブジェクトを複製
	static void DuplicateSelected(StageBase* stage);
	//選択中の配置オブジェクトへギズモ操作を反映
	static bool UpdateSelectedGizmo(StageBase* stage, const Camera* camera);

	//カタログに登録された配置アセットの選択欄を表示
	static void DrawAssetSelector();
	//Move・Rotate・Scaleの選択欄を表示
	static void DrawGizmoControls();
	//エディターウィンドウを描画
	static void DrawEditorWindow(StageBase* stage);

public:
	static bool IsOpen() { return m_IsOpen; }
	static void Update(Scene* scene);
	//シーン更新後のカメラ行列を使ってギズモ操作とクリック選択を処理
	static void UpdateSceneInteraction(Scene* scene);
};
