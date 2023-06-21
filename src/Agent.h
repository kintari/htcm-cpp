#pragma once

#include <array>
#include <cstdint>
#include <string>

class Agent {
public:

	bool IsGadget() const {
		return (elite == 0xffffffff) && ((prof & 0xffff0000) == 0xffff0000);
	}
	
	bool IsNPC() const {
		return (elite == 0xffffffff) && ((prof & 0xffff0000) != 0xffff0000);
	}
	
	bool IsPlayer() const {
		return elite != 0xffffffff;
	}

	int GetSpecies() const {
		return IsNPC() ? (prof & 0x0000ffff) : -1;
	}

	uint64_t addr;
	uint32_t prof;
	uint32_t elite;
	uint16_t toughness;
	uint16_t concentration;
	uint16_t healing;
	uint16_t hitbox_width;
	uint16_t condition;
	uint16_t hitbox_height;
	std::string name, account;
	int subgroup;
	uint32_t unused;
};