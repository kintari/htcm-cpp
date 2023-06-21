#pragma once

#include <string>
#include <concepts>

#include <stdio.h>
#include <array>

class BinaryReader {
public:

	BinaryReader(const std::string& filename);
	~BinaryReader();
	
	template <std::integral T>
	BinaryReader& read(T *value) {
		return read(value, sizeof(T));
	}

	BinaryReader& read(void *buf, size_t count) {
		(void) fread(buf, 1, count, _file);
		return *this;
	}

	template <typename T>
	BinaryReader& read_into(T& a);

	template <typename T, size_t N>
	BinaryReader& read_into(std::array<T,N>& a) {
		return read(&a[0], N*sizeof(T));
	}

bool eof() const {
		return feof(_file);
	}

private:

	FILE *_file;
};
