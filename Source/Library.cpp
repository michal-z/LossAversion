#include "Pch.h"
#include "Library.h"


std::vector<uint8_t> LoadFile(const char* fileName)
{
	FILE* file = fopen(fileName, "rb");
	assert(file);
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	assert(size != -1);
	std::vector<uint8_t> content(size);
	fseek(file, 0, SEEK_SET);
	fread(&content[0], 1, content.size(), file);
	fclose(file);
	return content;
}

double GetTime()
{
	LARGE_INTEGER counter, frequency;
	QueryPerformanceCounter(&counter);
	QueryPerformanceFrequency(&frequency);
	return counter.QuadPart / (double)frequency.QuadPart;
}
