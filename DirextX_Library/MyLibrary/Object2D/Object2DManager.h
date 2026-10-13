#pragma once
#include "../SingleTon/SingletonBase.h"
#include <list>

class Object2D;

class Object2DManager : public SingletonBase<Object2DManager>
{
public:

	void Push(Object2D* _obj);
	void Pop(Object2D* _obj);

	void DeleteAllObject2D();

	void SortByOrder();

private:

	std::list<Object2D*>* objects;
	bool needSortDraw;
	Object2D* running;

	friend class SingletonBase<Object2DManager>;

	Object2DManager();
	~Object2DManager();

	void Update();
	void Draw();

	friend class Main;
};