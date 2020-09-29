#include "BobballArray.h"
#include "Compiler.h"
#include <iostream>

BobballArray::BobballArray(std::vector<std::string> arrayRanges)
{
	this->arrayRanges = arrayRanges;
}

std::vector<std::string> BobballArray::getBbRanges(std::vector<std::string> words)
{
	std::vector<std::string> rangeWords;
	for (unsigned int i = 2; i < words.size(); i++)
	{
		rangeWords.push_back(words[i]);
	}
	return rangeWords;
}

int BobballArray::getNumberOfDimensions()
{
	return arrayRanges.size();
}

std::vector<std::pair<int, int>> BobballArray::getRanges()
{
	std::vector<std::pair<int,int>> ranges;
	for (std::string range : arrayRanges)
	{
		std::vector<std::string> splt = Compiler::split(range, ".");
		ranges.push_back(std::pair<int, int>(std::stoi(splt[0]), std::stoi(splt[1])));
	}
	return ranges;
}

int BobballArray::getTotalReservedSpace()
{
	auto ranges = getRanges();
	int totalReservedSpace = (ranges[0].second - ranges[0].first + 1);
	for (unsigned int i = 1; i < ranges.size(); i++)
	{
		totalReservedSpace *= (ranges[i].second - ranges[i].first + 1);
	}
	return totalReservedSpace * 4;
}

void BobballArray::hasCorrectNumberOfIndices(std::vector<std::pair<int, int>>& ranges, std::vector<int>& indices)
{
	if (ranges.size() != indices.size())
	{
		std::cout << "Failed to compile: Incorrect number of indices: " << indices[0];
		for (unsigned int i = 1; i < indices.size(); i++)
		{
			std::cout << ", " << indices[i];
		}
		std::cout << std::endl;
		exit(1);
	}
}

void BobballArray::indicesAreInRange(std::vector<std::pair<int, int>>& ranges, std::vector<int>& indices)
{
	for (unsigned int i = 0; i < indices.size(); i++)
	{
		if (ranges[i].first > indices[i] || ranges[i].second < indices[i])
		{
			std::cout << "Failed to compile: index out of range" << std::endl;
			exit(1);
		}
	}
}

int BobballArray::getRelocationFactor()
{
	auto ranges = getRanges();
	int relocationFactor = 0;
	for (unsigned int i = 0; i < ranges.size(); i++)
	{
		int offset = 1;
		if (ranges.size() > i + 1)
		{
			offset = ranges[i + 1].second - ranges[i + 1].first + 1;
			for (unsigned int j = i + 2; j < ranges.size(); j++)
			{
				offset *= ranges[j].second - ranges[j].first + 1;
			}
		}
		relocationFactor += ranges[i].first * offset;
	}
	return relocationFactor;
}

int BobballArray::getBytePosition(std::vector<int> indices)
{
	auto ranges = getRanges();
	hasCorrectNumberOfIndices(ranges, indices);
	indicesAreInRange(ranges, indices);
	int bytePosition = 0;
	for (unsigned int i = 0; i < indices.size(); i++)
	{
		int offset = 1;
		if (indices.size() > i + 1)
		{
			offset = ranges[i + 1].second - ranges[i + 1].first + 1;
			for (unsigned int j = i + 2; j < indices.size(); j++)
			{
				offset *= ranges[j].second - ranges[j].first + 1;
			}
		}
		bytePosition += indices[i] * offset;
	}

	return (bytePosition - getRelocationFactor()) * 4;
}
