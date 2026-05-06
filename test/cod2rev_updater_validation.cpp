#include "../src/libcod/cod2rev_updater.hpp"

#include <cassert>

int main()
{
	assert(Cod2revUpdater_IsValidDecimalSize("1"));
	assert(Cod2revUpdater_IsValidDecimalSize("123456789"));
	assert(!Cod2revUpdater_IsValidDecimalSize(""));
	assert(!Cod2revUpdater_IsValidDecimalSize("0"));
	assert(!Cod2revUpdater_IsValidDecimalSize("01"));
	assert(!Cod2revUpdater_IsValidDecimalSize("-1"));
	assert(!Cod2revUpdater_IsValidDecimalSize("12a"));

	assert(Cod2revUpdater_IsValidSha256("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"));
	assert(Cod2revUpdater_IsValidSha256("ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789"));
	assert(!Cod2revUpdater_IsValidSha256(""));
	assert(!Cod2revUpdater_IsValidSha256("short"));
	assert(!Cod2revUpdater_IsValidSha256("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdeg"));

	assert(Cod2revUpdater_ShouldSendRequestForTest(1000, -1, false, false, true, true));
	assert(Cod2revUpdater_ShouldSendRequestForTest(1000, 900, false, true, true, true));
	assert(!Cod2revUpdater_ShouldSendRequestForTest(1000, 900, false, false, true, true));
	assert(Cod2revUpdater_ShouldSendRequestForTest(601000, 1000, false, false, true, true));
	assert(!Cod2revUpdater_ShouldSendRequestForTest(30000, 1000, true, false, true, true));
	assert(Cod2revUpdater_ShouldSendRequestForTest(31000, 1000, true, false, true, true));
	assert(!Cod2revUpdater_ShouldSendRequestForTest(1000, -1, false, true, false, true));
	assert(!Cod2revUpdater_ShouldSendRequestForTest(1000, -1, false, true, true, false));

	return 0;
}
