#include "stdafx.h"
#include "MultiBee.h"


MultiBee::MultiBee(vector<Bee>& bees, int orginalBeeTag, int currentBeeTag): beeArray(bees), initialBeeTag(orginalBeeTag)
{
	tags.push_back(initialBeeTag);
	tags.push_back(currentBeeTag);
}

void MultiBee::addTag(int tag) {
	tags.push_back(tag);
}

int MultiBee::getInitialBeeTag()
{
	return initialBeeTag;
}

MultiBee::~MultiBee()
{
}
