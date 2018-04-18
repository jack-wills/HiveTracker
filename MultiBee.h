#pragma once
class MultiBee
{
public:
	MultiBee(vector<Bee>& bees, int orginalBeeTag, int currentBeeTag);
	~MultiBee();
	int getInitialBeeTag(void);
	void addTag(int tag);
private:
	int initialBeeTag;
	vector<int> tags;
	vector<Bee>& beeArray;
};

