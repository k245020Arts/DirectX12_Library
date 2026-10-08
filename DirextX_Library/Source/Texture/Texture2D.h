#pragma once
#include "../Object2D/Object2D.h"
#include "../DescriptorHeap/DescriptorHeap.h"
#include "TextureLoader.h"
#include "../Window/Window.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;
struct Size;

class Texture2D : public Object2D
{
public:
	Texture2D();
	~Texture2D();

	bool Load(std::string _path);

    void SetBlendMode(BlendState _blendState);

    void SetAlpha(float _alphaValue);

    /// <summary>
    /// 画像の切り抜き位置とそこからの描画範囲を設定する関数
    /// *** 注意！！ 切り抜きは元画像のサイズを参照しているため、SetSizeで画像の大きさを変更しても元画像のサイズを参照します
    /// 画面に描画される画像の大きさはSetSizeで参照した値のまま表示されます
    /// </summary>
    /// <param name="_startX">開始座標</param>
    /// <param name="_startY">開始座標</param>
    /// <param name="_width">横の長さ</param>
    /// <param name="_height">縦の長さ</param>
    void SetRect(float _startX,float _startY,float _width,float _height);
    /// <summary>
    /// 画像の切り抜き位置とそこからの描画範囲を設定する関数
    ///  *** 注意！！ 切り抜きは元画像のサイズを参照しているため、SetSizeで画像の大きさを変更しても元画像のサイズを参照します
    /// 画面に描画される画像の大きさはSetSizeで参照した値のまま表示されます
    /// </summary>
    /// <param name="_rect">開始座標X,開始座標Y,横の長さ、縦の長さの順番</param>
    void SetRect(const Vector4& _rect);

    void SetRectUV(const Vector4& _rect);

    /// <summary>
    /// 画像の反転をするかしないかX版
    /// </summary>
    void SetFlipX(bool flip);
    /// <summary>
    /// 画像の反転をするかしないかY版
    /// </summary>
    void SetFlipY(bool flip);

    /// <summary>
    /// 画像の大きさを変えたいときに使う関数
    /// 元画像サイズが、小さいと感じたり大きいと感じたら使用してください
    /// transform.scaleでも指定が出来るが、こちらはピクセル指定ではないのでピクセル指定をしたい人はこちらの関数を使用
    /// </summary>
    /// <param name="_width">横幅</param>
    /// <param name="_height">縦幅</param>
    void SetSize(float _width,float _height);
    /// <summary>
    /// 画像の大きさを変えたいときに使う関数
    /// 元画像サイズが、小さいと感じたり大きいと感じたら使用してください
    /// transform.scaleでも指定が出来るが、こちらはピクセル指定ではないのでピクセル指定をしたい人はこちらの関数を使用
    /// </summary>
    /// <param name="_width">横幅</param>
    /// <param name="_height">縦幅</param>
    void SetSize(int _width,int _height);

    /// <summary>
    /// 引数なしは画像のサイズに戻す
    /// </summary>
    void SetSize();

    /// <summary>
    /// 色の変更
    /// </summary>
    /// <param name="_rect"></param>
    void SetColor(const Vector4& _color);

private:

	void Update()override;
	void Draw()override;

    void Set2DMatrix();

    void SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);

	TextureData textureData;

    // SRV
    DescriptorHandle* descriptorHandle = nullptr;

    // 描画用
    std::unique_ptr<VertexBuffer> vertexBuffer;
    std::unique_ptr<IndexBuffer> indexBuffer;

    // 描画用定数
    std::vector<std::shared_ptr<ConstBuffer>> constBuffer;

    // 描画用パイプライン
    std::unique_ptr<PipelineState> pipelineState;
    std::unique_ptr<RootSignature> rootSignature;

    std::vector<Vertex>vertices;

    Size textureSize = Size();

    Vector4 uvRect = Vector4();
    bool flipX = false;
    bool flipY = false;
    void UpdateUV();

    bool defalutSize;
};