# こりじょんらん！ / CollisionRun

C++・DirectX 11で制作した、全3ステージの3Dアクションゲームです。
本リポジトリは、採用選考でソースコードと設計内容をご覧いただくための閲覧用です。

- 作品の画像や詳細な解説は、応募時に添付するポートフォリオにも記載しています。

## 制作情報・実行環境

| 項目 | 内容 |
| --- | --- |
| 制作者 | 中島 寛樹 |
| 制作形態 | 個人制作 |
| 制作期間 | 約5か月 |
| ジャンル | 3Dアクションゲーム |
| 開発言語・描画API | C++ / DirectX 11 |
| 開発環境 | Visual Studio 2022 / MSVC v143 / Windows SDK |
| 実行環境 | Windows(64bit)、DirectX 11対応のグラフィックス環境 |
| 構成 | 全3ステージ、1人プレイ |

## ゲームのコンセプト

様々なギミックを攻略しながらステージに散らばる星を集め、
必要数に達すると開くゲートを通過して全3ステージのゴールを目指します。

ステージ1では基本操作とギミックを体験し、ステージ2では風や回転する障害物、
ステージ3では動く足場の乗り継ぎや高低差を使った構成へ発展させています。
新しいギミックは比較的安全な場所で試せるようにし、その後の地帯で難易度を上げています。

## 設計のコンセプト

回転・傾斜した足場に対応する衝突処理と、ステージ制作を効率化するエディターを中心に実装しました。

- **見た目に近い当たり判定：** OBBとSATを用い、向きの異なるオブジェクト同士の交差を判定します。押し戻し、斜面への接地、移動・回転足場への追従も扱います。
- **配置と実装の分離：** エディターで編集した配置情報をJSONへ保存します。アセット定義、オブジェクト生成、保存・読み込みをそれぞれ分け、同じ生成処理をステージとエディターで利用しています。
- **処理の責務を分ける構成：** 描画・当たり判定などをコンポーネントとして扱い、プレイヤーは移動、衝突、アニメーション、復帰処理にファイルを分けています。

## 担当範囲

学校配布のフレームワークを基に、個人でゲーム制作を行いました。
プレイヤー制御、OBB衝突判定と補正、ステージエディター、ギミック、
画面遷移・UI、音とエフェクトの管理、描画・当たり判定などのコンポーネント方式での実装、ステージの構成・配置などを実装・調整しました。
外部ライブラリの実装は自身の担当実装として扱っていません。

## 主な実装・コードの読みどころ

最初に、OBB衝突判定とステージエディターをご覧いただけますと幸いです。

| 実装 | 主なファイル | 確認できる内容 |
| --- | --- | --- |
| OBB衝突判定 | [obbColliderComponent.cpp](obbColliderComponent.cpp) / [obbColliderComponent.h](obbColliderComponent.h) | SATによる交差判定、押し戻し方向・量の計算、判定範囲の可視化 |
| プレイヤーの衝突・接地 | [playerCollision.cpp](playerCollision.cpp) / [box.cpp](box.cpp) | 移動の分割、斜面や端での接地補正、移動・回転足場への追従 |
| ステージエディター | [stageEditor.cpp](stageEditor.cpp) / [stageEditor.h](stageEditor.h) | クリック選択、数値入力・ギズモ編集、複製・削除、配置データの保存・読み込み |
| 配置データと生成処理 | [stageObjectData.h](stageObjectData.h) / [stageObjectCatalog.cpp](stageObjectCatalog.cpp) / [stageObjectFactory.cpp](stageObjectFactory.cpp) / [stageDataSerializer.cpp](stageDataSerializer.cpp) / [stageBase.cpp](stageBase.cpp) | アセット定義、共通の生成処理、JSONの入出力、配置オブジェクトの管理 |
| 操作・アニメーション・復帰 | [playerMovement.cpp](playerMovement.cpp) / [playerAnimation.cpp](playerAnimation.cpp) / [playerRespawn.cpp](playerRespawn.cpp) | ジャンプ受付猶予、移動、アニメーション切り替え、落下とチェックポイント復帰 |
| 地形の描画 | [terrainBlock.cpp](terrainBlock.cpp) / [shader/tiledBoxVS.hlsl](shader/tiledBoxVS.hlsl) | 地形の上下に分けた側面表示、スケールに応じたUVの繰り返し |
| 風の描画と処理 | [wind.cpp](wind.cpp) / [windRendererComponent.cpp](windRendererComponent.cpp) / [shader/windPS.hlsl](shader/windPS.hlsl) | 風力、OBB内のストリーク移動、動的頂点バッファを使ったまとめ描画 |
| 音・エフェクト | [audioManager.cpp](audioManager.cpp) / [effectManager.cpp](effectManager.cpp) / [effectSprite.cpp](effectSprite.cpp) | BGM・SE・ループ音の管理、時間制限付きSE、着地・ダッシュなどの演出 |
| シーン・メニュー | [manager.cpp](manager.cpp) / [stageSelect.cpp](stageSelect.cpp) / [pauseManager.cpp](pauseManager.cpp) / [clear.cpp](clear.cpp) / [gameOver.cpp](gameOver.cpp) | フェード遷移、マウス・キー操作、選択と決定、再開・リトライ |

## 外部ライブラリ・クレジット

| ライブラリ | ライセンスファイル |
| --- | --- |
| Dear ImGui | [licenses/imgui/LICENSE.txt](licenses/imgui/LICENSE.txt) |
| ImGuizmo | [licenses/imguizmo/LICENSE.txt](licenses/imguizmo/LICENSE.txt) |
| nlohmann/json | [licenses/nlohmann-json/LICENSE.txt](licenses/nlohmann-json/LICENSE.txt) |
| DirectXTex | [licenses/directxtex/LICENSE.txt](licenses/directxtex/LICENSE.txt) |
| Assimp | [licenses/assimp/LICENSE.txt](licenses/assimp/LICENSE.txt) |

各ライブラリの著作権表示・ライセンスを保持しています。
使用した素材のクレジットは[CREDITS.txt](CREDITS.txt)をご参照ください。