#pragma once

#include "Agent.h"
#include "Skill.h"
#include "Event.h"

#include <vector>

class Encounter {
public:

	Encounter(
		std::vector<Agent *>&& agents,
		std::vector<Skill *>&& skills,
		std::vector<Event *>&& events);

	Agent *GetAgentByAddr(uint64_t addr) const;

	Skill *GetSkill(uint32_t id) const;

public:

	std::vector<Agent *> Agents, AgentsByAddr;
	std::vector<Skill *> Skills;
	std::vector<Event *> Events;
};