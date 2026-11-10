#pragma once
#include "../SingleTon/SingletonBase.h"
#include <list>

class Object3D;
class ShadowMap;
class ShadowCamera;

class Object3DManager : public SingletonBase<Object3DManager>
{
public:

	void Push(Object3D* _obj);
	void Pop(Object3D* _obj);

	void DeleteAllObject2D();

	void SortByOrder();

private:

	std::list<Object3D*>* objects;
	bool needSortDraw;
	Object3D* running;

	friend class SingletonBase<Object3DManager>;

	Object3DManager();
	~Object3DManager();

	void Update();
	void Draw();

	// シャドウマップ用の描画
	void DrawShadow(const ShadowMap& _shadowMap,const ShadowCamera& _camera);

	friend class Application;
	friend class PostEffectCreater;
};