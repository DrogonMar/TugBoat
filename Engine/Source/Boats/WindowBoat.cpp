#include <TugBoat/Boats/WindowBoat.h>

using namespace TugBoat;

BOAT_IMPL(WindowBoat);

std::vector<std::string> WindowBoat::GetInstanceExtensions() const
{
	return {};
}
