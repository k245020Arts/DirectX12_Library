#include "Object2DManager.h"
#include "Object2D.h"

Object2DManager::Object2DManager()
{
	objects = new std::list<Object2D*>;
	objects->clear();
	needSortDraw = false;
	running = nullptr;
}

Object2DManager::~Object2DManager()
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

void Object2DManager::Update()
{
	for (auto itr = objects->begin(); itr != objects->end(); itr++)
	{
		Object2D* obj = *itr;
		if (obj == nullptr)
			continue;
		if (not obj->IsDestory())
		{
			running = obj;
			obj->Update();
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

void Object2DManager::Draw()
{
	if (needSortDraw)
	{
		objects->sort([](Object2D* a, Object2D* b) {return a->GetDrawOrder() > b->GetDrawOrder(); });
		needSortDraw = false;
	}
	for (Object2D* obj : *objects)
	{
		if (obj == nullptr || obj->IsDestory())
			continue;
		obj->Draw();
	}
}

void Object2DManager::Push(Object2D* _obj)
{
	objects->emplace_back(_obj);
	needSortDraw = true;
}

void Object2DManager::Pop(Object2D* _obj)
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

void Object2DManager::DeleteAllObject2D()
{
	assert(running == nullptr);

	for (auto itr = objects->begin(); itr != objects->end(); itr++)
	{
		Object2D* obj = *itr;
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

void Object2DManager::SortByOrder()
{
	needSortDraw = true;
}
