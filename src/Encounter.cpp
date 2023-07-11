#include "Encounter.h"

#include <assert.h>
#include <algorithm>
#include <numeric>

Encounter::Encounter(
	std::vector<Agent *>&& agents,
	std::vector<Skill *>&& skills,
	std::vector<Event *>&& events)
	: Agents(agents), Skills(skills), Events(events)
{
	uint64_t max_addr = 0;
	for (auto agent : agents)
		max_addr = std::max(max_addr, agent->addr);
	AgentsByAddr.resize(max_addr+1);
	for (auto agent: Agents) {
		AgentsByAddr[agent->addr] = agent;
	}
}

Agent *Encounter::GetAgentByAddr(uint64_t addr) const {
	return addr < AgentsByAddr.size() ?
		AgentsByAddr[addr] : nullptr;
}

Skill *Encounter::GetSkill(uint32_t id) const {
	auto iter = std::find_if(
		Skills.begin(),
		Skills.end(),
		[=](Skill *s){ return s->id == id; }
	);
	return iter == Skills.end() ? nullptr : *iter;
}