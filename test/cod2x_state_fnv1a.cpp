#include "../src/libcod/cod2x_state.hpp"

#include <cassert>

int main()
{
	const int hash = Cod2x_HwidHash("0123456789abcdef0123456789abcdef");

	assert(hash != 0);
	assert(hash == Cod2x_HwidHash("0123456789abcdef0123456789abcdef"));
	assert(hash != Cod2x_HwidHash("fedcba9876543210fedcba9876543210"));

	assert(Cod2x_IsValidHwid2("0123456789abcdef0123456789abcdef"));
	assert(Cod2x_IsValidHwid2("ABCDEF0123456789ABCDEF0123456789"));
	assert(!Cod2x_IsValidHwid2(""));
	assert(!Cod2x_IsValidHwid2("short"));
	assert(!Cod2x_IsValidHwid2("0123456789abcdef0123456789abcdeg"));

	return 0;
}
