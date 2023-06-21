#include "Encounter.h"

#include <assert.h>
#include <algorithm>

Encounter::Encounter(
	std::vector<Agent *>&& agents,
	std::vector<Skill *>&& skills,
	std::vector<Event *>&& events)
	: Agents(agents), Skills(skills), Events(events)
{
	AgentsByAddr.resize(1000000);
	for (auto agent: Agents) {
		assert(agent->addr < AgentsByAddr.size());
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