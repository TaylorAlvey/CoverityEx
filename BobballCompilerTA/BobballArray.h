#pragma once
#include <string>
#include <vector>

class BobballArray
{
private:
	std::vector<std::string> arrayRanges;
public:
	BobballArray(std::vector<std::string> arrayRanges);
	static std::vector<std::string> getBbRanges(std::vector<std::string> words);
	int getNumberOfDimensions();
	std::vector<std::pair<int, int>> getRanges();
	int getTotalReservedSpace();
	void hasCorrectNumberOfIndices(std::vector<std::pair<int, int>>& ranges, std::vector<int>& indices);
	void indicesAreInRange(std::vector<std::pair<int, int>>& ranges, std::vector<int>& indices);
	int getRelocationFactor();
	int getBytePosition(std::vector<int> indices);
};