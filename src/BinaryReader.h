#pragma once

#include <array>
#include <concepts>
#include <string>

#include "Stream.h"

class BinaryReader {
public:

	BinaryReader(Stream *stream)
		: _stream(stream)
	{
	}

	~BinaryReader() {};

	template <std::integral T>
	BinaryReader& read(T *value) {
		return read(value, sizeof(T));
	}

	BinaryReader& read(void *buf, size_t count) {
		_stream->read(buf, count);
		return *this;
	}

	template <typename T>
	BinaryReader& read_into(T& a);

	template <typename T, size_t N>
	BinaryReader& read_into(std::array<T,N>& a) {
		return read(&a[0], N*sizeof(T));
	}

	bool eof() const {
		return _stream->done();
	}

private:
	Stream *_stream;
};
