#pragma once

#include <cstddef>

class Stream {
public:
	virtual size_t read(void *buf, size_t count) = 0;
	virtual bool done() const = 0;
};