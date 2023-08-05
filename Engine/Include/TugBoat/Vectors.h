#pragma once
#include <TugBoat/Core.h>

struct IVec2{
	i32 x;
	i32 y;

	IVec2(i32 x, i32 y) : x(x), y(y) {}
	IVec2() : x(0), y(0) {}

	IVec2 operator+(const IVec2& other) const {
		return {x + other.x, y + other.y};
	}

	IVec2 operator-(const IVec2& other) const {
		return {x - other.x, y - other.y};
	}

	IVec2 operator*(const IVec2& other) const {
		return {x * other.x, y * other.y};
	}

	IVec2 operator/(const IVec2& other) const {
		return {x / other.x, y / other.y};
	}

	bool operator==(const IVec2& other) const {
		return x == other.x && y == other.y;
	}
};

struct UIVec{
	ui32 x;
	ui32 y;

	UIVec(ui32 x, ui32 y) : x(x), y(y) {}
	UIVec() : x(0), y(0) {}

	UIVec operator+(const UIVec& other) const {
		return {x + other.x, y + other.y};
	}

	UIVec operator-(const UIVec& other) const {
		return {x - other.x, y - other.y};
	}

	UIVec operator*(const UIVec& other) const {
		return {x * other.x, y * other.y};
	}

	UIVec operator/(const UIVec& other) const {
		return {x / other.x, y / other.y};
	}

	bool operator==(const UIVec& other) const {
		return x == other.x && y == other.y;
	}
};