#include "Object3DManager.h"
#include "Object3D.h"
#include "../Camera3D/Camera3D.h"

Object3DManager::Object3DManager()
{
	objects = new std::list<Object3D*>;
	objects->clear();
	needSortDraw = false;
	running = nullptr;
}

Object3DManager::~Object3DManager()
{
	while (objects->size() > 0)
	{
		auto itr = objects->begin();
		if (*itr != nullptr)
		{
			delete* itr;
		}
		objects->erase(itr);
	}
	objects->clear();
	delete objects;
	objects = nullptr;
}

void Object3DManager::Update()
{
	Camera3D::GetInstance()->Update(); //3D描画で使うカメラの更新
	for (auto itr = objects->begin(); itr != objects->end(); itr++)
	{
		Object3D* obj = *itr;
		if (obj == nullptr)
			continue;
		if (not obj->IsDestory())
		{
			running = obj;
			obj->Update();
			obj->VertexMapping(Camera3D::GetInstance());
			running = nullptr;

		}
		if (obj->IsDestory())
		{
			delete obj;
			*itr = nullptr;
		}
	}
	for (auto itr = objects->begin(); itr != objects->end();)
	{
		if (*itr == nullptr)
		{
			itr = objects->erase(itr);
		}
		else
		{
			itr++;
		}
	}
}

void Object3DManager::Draw()
{
	if (needSortDraw)
	{
		objects->sort([](Object3D* a, Object3D* b) {return a->GetDrawOrder() > b->GetDrawOrder(); });
		needSortDraw = false;
	}
	for (Object3D* obj : *objects)
	{
		if (obj == nullptr || obj->IsDestory())
			continue;
		obj->Draw();
	}
}

void Object3DManager::Push(Object3D* _obj)
{
	objects->emplace_back(_obj);
	needSortDraw = true;
}

void Object3DManager::Pop(Object3D* _obj)
{
	assert(running != _obj);

	for (auto itr = objects->begin(); itr != objects->end(); itr++)
	{
		if (*itr == _obj)
		{
			*itr = nullptr;
		}
	}
}

void Object3DManager::DeleteAllObject2D()
{
	assert(running == nullptr);

	for (auto itr = objects->begin(); itr != objects->end(); itr++)
	{
		Object3D* obj = *itr;
		if (not obj->IsDestory())
		{
			delete obj;
			*itr = nullptr;
		}
	}
	for (auto itr = objects->begin(); itr != objects->end();)
	{
		if (*itr == nullptr) {
			itr = objects->erase(itr);
		}
		else
		{
			itr++;
		}
	}
}

void Object3DManager::SortByOrder()
{
	needSortDraw = true;
}
