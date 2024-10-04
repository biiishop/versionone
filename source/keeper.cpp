#include "keeper.hpp"
#include "helpers.hpp"

Keeper& getKeeper()
{
	static Keeper i; // Returns a constant Keeper, can be accessed from anywhere
	return i;
}

bool Keeper::tryRegisterShine(Shine* shine) {
	if (!getKeeper().mShineArray.isFull()) {
		if (!shine->isGot()) {
			getKeeper().mShineArray.pushBack(shine);
			return true;
		}
	}
    return false;
}