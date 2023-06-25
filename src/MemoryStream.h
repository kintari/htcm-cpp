#pragma once

#include "Stream.h"

#include <vector>

class MemoryStream : public Stream {
public:

	MemoryStream(std::vector<uint8_t> &&bytes)
		: _bytes(bytes), _pos(0)
	{
	}

	virtual size_t read(void *buf, size_t count) {
		assert(buf);
		size_t avail = _bytes.size() - _pos;
		size_t n = std::min(count, avail);
		if (n > 0) memcpy(buf, &_bytes[_pos], n);
		_pos += n;
		return n;
	}

	virtual bool rewind(size_t offset) {
		if (_pos >= offset) {
			_pos -= offset;
			return true;
		}
		else {
			return false;
		}
	}

	virtual bool done() const {
		return _pos >= _bytes.size();
	}

private:
	std::vector<uint8_t> _bytes;
	size_t _pos;
};