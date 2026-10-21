#pragma once
#include <iostream>
#include <algorithm>
#include <cmath>
#include "../Helper/Vector/VectorHelper.h"

struct Rgb
{
    Rgb() {
        Rgb(255.0f, 255.0f, 255.0f, 255.0f);
    }
    Rgb(float _r, float _g, float _b, float _a) {
        r = _r;
        g = _g;
        b = _b;
        a = _a;
    }
    float r, g, b, a;
};

struct Hsv
{
    Hsv() {
        Hsv(360.0f, 1.0f, 1.0f);
    }
    Hsv(float _h, float _s, float _v) {
        h = _h;
        s = _s;
        v = _v;
    }
    float h, s, v;
};

class Color{
public:

    Color() {}

    ~Color() {}

    static Rgb Vector4ToRGB(const Vector4& _color) {
        Rgb rgb;
        rgb.a = _color.w * 255.0f;
        rgb.g = _color.y * 255.0f;
        rgb.b = _color.z * 255.0f;
        rgb.r = _color.x * 255.0f;
        return rgb;
    }

    //rgbをCOLOR_F型に変換できる。   
    static Vector4 RGBToVector4(const Rgb& rgb) {
        Vector4 color;
        color.x = (rgb.r) / 255.0f;
        color.y = (rgb.g) / 255.0f;
        color.z = (rgb.b) / 255.0f;
        color.w = (rgb.a) / 255.0f;
        return color;
    }

    void Vector3ToRGB(const Vector3& _color) {
        Rgb rgb;
        rgb.g = _color.y * 255.0f;
        rgb.b = _color.z * 255.0f;
        rgb.r = _color.x * 255.0f;
    }

    //rgbをCOLOR_F型に変換できる。   
    static Vector3 RGBToVector3(const Rgb& rgb) {
        Vector3 color;
        color.x = (rgb.r) / 255.0f;
        color.y = (rgb.g) / 255.0f;
        color.z = (rgb.b) / 255.0f;
        return color;
    }
private:
    
};
//-----------------------色の定数----------------------

const Rgb WHITE =       Rgb(255, 255, 255, 255);
const Rgb RED =         Rgb(255, 0, 0, 255);
const Rgb GREEN =       Rgb(0, 255, 0, 255);
const Rgb BLUE =        Rgb(0, 0, 255, 255);
const Rgb YELLOW =      Rgb(255, 255, 0, 255);
const Rgb MAGENTA =     Rgb(255, 0, 255, 255);
const Rgb CYAN =        Rgb(0, 255, 255, 255);
const Rgb BLACK =       Rgb(0, 0, 0, 255);
const Rgb SAKURA =      Rgb(253, 238, 239, 255);
const Rgb ORANGE =      Rgb(255, 165, 0, 255);
const Rgb PURPLE =      Rgb(128, 0, 128, 255);
const Rgb PINK =        Rgb(255, 192, 203, 255);
const Rgb BROWN =       Rgb(165, 42, 42, 255);
const Rgb GOLD =        Rgb(255, 215, 0, 255);
const Rgb SILVER =      Rgb(192, 192, 192, 255);
const Rgb SKY_BLUE =    Rgb(135, 206, 235, 255);
const Rgb DARK_BLUE =   Rgb(0, 0, 139, 255);
const Rgb LIGHT_BLUE =  Rgb(173, 216, 230, 255);
const Rgb DARK_RED =    Rgb(139, 0, 0, 255);
const Rgb LIGHT_GREEN = Rgb(144, 238, 144, 255);
const Rgb DARK_GREEN =  Rgb(0, 100, 0, 255);
const Rgb GRAY =        Rgb(128, 128, 128, 255);
const Rgb DARK_GRAY =   Rgb(64, 64, 64, 255);
const Rgb LIGHT_GRAY =  Rgb(192, 192, 192, 255);

//-----------------------------------------------------------

//透明と不透明で使う定数(DxLibのColorF型verと通常で使うverの2つ用意)

static const int OPAQUE_COLOR = 255; //不透明
static const int TRANSPARENT_COLOR = 0; //透明

static const float OPAQUE_COLOR_F = 1.0f; //不透明
static const float TRANSPARENT_COLOR_F = 0.0f; //透明