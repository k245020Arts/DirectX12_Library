#pragma once
#include "../Object3D/Object3D.h"
#include "../ShaderStruct/ShaderStruct.h"
#include <memory>
#include "../Texture/TextureLoader.h"
#include "../DescriptorHeap/DescriptorHeap.h"
#include "../Window/Window.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;
struct Size;

class Billboard : public Object3D
{
public:
	Billboard();
	~Billboard();

	/// <summary>
	/// ビルボードに使う画像のロードや設定をする
	/// </summary>
	/// <param name="_path">ファイルパス</param>
	/// <returns>結果 : true : 成功 false : 失敗</returns>
	bool Load(const std::string& _path);

	/// <summary>
	///	ビルボードのタイプを設定することが出来る関数
	/// </summary>
	/// <param name="_yBillboard">true : Y軸回転ビルボードにする false : 全ての方向に対してビルボードをする</param>
	void SetBillBoardType(bool _yBillboard);

	/// <summary>
	/// 画像の切り抜き位置とそこからの描画範囲を設定する関数
	/// *** 注意！！ 切り抜きは元画像のサイズを参照しているため、SetSizeで画像の大きさを変更しても元画像のサイズを参照します
	/// 画面に描画される画像の大きさはSetSizeで参照した値のまま表示されます
	/// </summary>
	/// <param name="_startX">開始座標</param>
	/// <param name="_startY">開始座標</param>
	/// <param name="_width">横の長さ</param>
	/// <param name="_height">縦の長さ</param>
	void SetRect(float _startX, float _startY, float _width, float _height);
	/// <summary>
	/// 画像の切り抜き位置とそこからの描画範囲を設定する関数
	///  *** 注意！！ 切り抜きは元画像のサイズを参照しているため、SetSizeで画像の大きさを変更しても元画像のサイズを参照します
	/// 画面に描画される画像の大きさはSetSizeで参照した値のまま表示されます
	/// </summary>
	/// <param name="_rect">開始座標X,開始座標Y,横の長さ、縦の長さの順番</param>
	void SetRect(const Vector4& _rect);
	/// <summary>
	/// uv指定でSetRectが出来るようにする。
	/// あまり使う機会はないと思われる
	/// </summary>
	/// <param name="_rect">開始座標X,開始座標Y,横の長さ、縦の長さの順番 : これのピクセル指定ではなくUV指定ver</param>
	void SetRectUV(const Vector4& _rect);

private:

	void Update()override;
	void Draw()override;

	void VertexMapping(const Camera3D* _camera)override;

	void SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);


	DirectX::XMMATRIX YBillBoardMatrix(const Camera3D* _camera);
	DirectX::XMMATRIX AllBillBoardMatrix(const Camera3D* _camera);

	std::unique_ptr<VertexBuffer> vertexBuffer;
	std::unique_ptr<RootSignature> rootSignature;
	std::unique_ptr<PipelineState> pipelineState;
	std::unique_ptr<PipelineState> wireFramePipelineState;
	std::unique_ptr<IndexBuffer> indexBuffer;
	std::vector<Vertex>vertices;

	std::vector<std::shared_ptr<ConstBuffer>> constBuffer;

	Vector4 uvRect;
	Size textureSize = Size();

	TextureData textureData;

	// SRV
	DescriptorHandle* descriptorHandle = nullptr;

	bool yBillBoard;

};
