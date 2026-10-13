#pragma once
#include <string>

#include <Windows.h>
#include <vector>

#include <DirectXMath.h>

struct Size
{
	LONG width;
	LONG height;

	Size() {
		width = 0;
		height = 0;
	}

	Size(const LONG _width, const LONG _height) {
		width = _width;
		height = _height;
	}
};

struct MatriceData
{
	DirectX::XMMATRIX world; //モデル本体を回転させたり移動させたりする行列
	DirectX::XMMATRIX viewproj; //ビューとプロジェクション合成行列
};

#pragma pack(push, 1)
struct PMDVertex
{
	DirectX::XMFLOAT3 pos;//頂点座標 : 12バイト
	DirectX::XMFLOAT3 normal;//法線ベクトル : 12バイト
	DirectX::XMFLOAT2 uv; //uv座標 : 8バイト
	unsigned short boneNo[2]; //ボーン番号 : 4バイト
	unsigned char boneWeight;//ボーン影響度 : 1バイト
	unsigned char edgeFlg; //輪郭線フラグ : 1バイト
	unsigned short dummy; //ダミーがないと38バイトで終わってしまうので、ダミーを入れて40バイトになるようにする

	PMDVertex()
	{
		pos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		uv = DirectX::XMFLOAT2(0.0f, 0.0f);
		boneNo[0] = 0;
		boneNo[1] = 0;
		boneWeight = 0;
		edgeFlg = 0;
		dummy = 0;
	}
};
#pragma pack(pop)

int AlignmentedSize(size_t size, size_t alignment);

class Window
{
public:
	~Window();
	///<summary>
	///	ウィンドウの作成
	///	< / summary>
	///	<param name = "clientWidth">#< / param>
	///	<param name = "clientHeight"> = < / param>
	///	<param name = "titleName"> < l & < / param>
	///	<param name = "windowClassName">< / param>
	bool Create(const Size& _size,const std::wstring& _titleName,const std::wstring& _windowClassName);

	bool ProcessMessage();

	const Size& GetWindowSize()const { return windowSize; }

	const HWND& GetHwnd() const{ return hwnd; }

private:

	Size windowSize = Size();
	HWND hwnd{};
	WNDCLASSEX w{};
};