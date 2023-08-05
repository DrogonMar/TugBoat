#pragma once

namespace TugBoat{
/*
 * This class is gonna work in an intermediate way, so like:
 * Reflect::StartClassDef("Dock");
 * Reflect::AddMember("s_Instance", "Dock*");
 * Reflect::EndClassDef();
 *
 */
class Reflect{
public:
	Reflect();
	~Reflect();
};
}