#pragma once

#include <iostream>
#include <map>
#include <vector>

template <typename T>
void to_json(std::ostream& out, T);

template <>
void to_json(std::ostream& out, std::string value) {
	//for (size_t i = 0; i < depth; i++)
	//	out << "  ";
	out << '\"' << value << '\"';
}

template <>
void to_json(std::ostream& out, int value) {
	//for (size_t i = 0; i < depth; i++)
	//	out << "  ";
	out << value;
}

template <typename K, typename V>
void to_json(std::ostream& out, const std::map<K,V>& value) {
	//for (size_t i = 0; i < depth; i++)
	//	out << "  ";
	auto f = [&out](auto& entry) {
		out << "\"" << entry.first << "\": ";
		to_json(out, entry.second);
	};
	out << "{ ";
	auto iter = value.begin();
	if (iter != value.end()) {
		f(*iter);
		while (++iter != value.end()) {
			out << ", ";
			f(*iter);
		}
	}
	out << " }";
}

template <typename K, typename V>
void to_json(std::ostream& out, const std::vector<std::pair<K,V>>& value) {
	//for (size_t i = 0; i < depth; i++)
	//	out << "  ";
	auto f = [&out](auto& entry) {
		out << "\"" << entry.first << "\": ";
		to_json(out, entry.second);
	};
	out << "{ ";
	auto iter = value.begin();
	if (iter != value.end()) {
		f(*iter);
		while (++iter != value.end()) {
			out << ", ";
			f(*iter);
		}
	}
	out << " }";
}

template <typename T>
void to_json(std::ostream& out, const std::vector<T>& value, int depth) {
	//for (size_t i = 0; i < depth; i++)
	//	out << "  ";
	out << "[";
	for (auto& entry: value) {
		to_json(out, entry);
		out << ", ";
	}
	out << "]";
}