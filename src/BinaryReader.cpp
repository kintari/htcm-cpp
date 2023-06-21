
#include "BinaryReader.h"

BinaryReader::BinaryReader(const std::string& filename)
	: _file(nullptr)
{
	_file = fopen(filename.c_str(), "rb");
}

BinaryReader::~BinaryReader()
{
	fclose(_file);
}
