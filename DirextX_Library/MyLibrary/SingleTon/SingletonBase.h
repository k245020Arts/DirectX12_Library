#pragma once

/// <summary>
/// シングルトンのベースクラス
/// </summary>
template<typename Derived_Class>
class SingletonBase {
public:
	/// <summary>
	/// Instanceの取得
	/// </summary>
	/// <returns></returns>
	static Derived_Class* GetInstance() {
		if (instance == nullptr) {
			instance = new Derived_Class();
		}
		return instance;
	}
	/// <summary>
	/// Instanceの破棄 今はApp.cpp のReleaseで呼ばれている
	/// </summary>
	/// <returns></returns>
	static void Destroy() {
		delete instance;
		instance = nullptr;
	}

protected:
	SingletonBase() = default;
	virtual ~SingletonBase() = default;

	virtual void Update() { ; }
	virtual void Draw() { ; }

private:
	static Derived_Class* instance;
};

template<typename Derived_Class>
Derived_Class* SingletonBase<Derived_Class>::instance = nullptr;