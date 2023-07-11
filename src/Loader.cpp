#include "Loader.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "BinaryReader.h"
#include "Agent.h"
#include "Event.h"
#include "Skill.h"
#include "Encounter.h"

Encounter *Loader::LoadEncounter() {
	
	std::array<char,4> magic;
	std::array<char,8> version;

	uint8_t revision = 0;
	uint16_t bossId = 0;
	uint8_t unused = 0;

	(*Reader)
		.read_into(magic)
		.read_into(version)
		.read(&revision)
		.read(&bossId)
		.read(&unused);

	uint32_t numAgents = 0;
	std::vector<Agent *> agents;
	(*Reader).read(&numAgents);
	for (auto i = 0; i < numAgents; i++)
		agents.push_back(LoadAgent());

	uint32_t numSkills = 0;
	std::vector<Skill *> skills;
	(*Reader).read(&numSkills);
	for (auto i = 0; i < numSkills; i++)
		skills.push_back(LoadSkill());

	std::vector<Event *> events;
	while (!Reader->eof())
		events.push_back(LoadEvent());

	return new Encounter(std::move(agents), std::move(skills), std::move(events));
}

Agent *Loader::LoadAgent() {
	Agent *agent = new Agent();
	std::array<char,64> buf {};
	(*Reader)
		.read(&agent->addr)
		.read(&agent->prof)
		.read(&agent->elite)
		.read(&agent->toughness)
		.read(&agent->concentration)
		.read(&agent->healing)
		.read(&agent->hitbox_width)
		.read(&agent->condition)
		.read(&agent->hitbox_height)
		.read_into(buf)
		.read(&agent->unused);
	std::string account;
	int subgroup;
	auto iter = std::find(buf.begin(), buf.end(), '\0');
	if (iter == buf.end()) {
		agent->name = std::string(&buf[0]);
	}
	else {
		size_t len = iter - buf.begin();
		agent->name = std::string(&buf[0], len);
		agent->account = "[TODO]";
	}
	return agent;
}

Skill *Loader::LoadSkill() {
	Skill *skill = new Skill();
	std::array<char,64> name;
	(*Reader)
		.read(&skill->id)
		.read_into(name);
	skill->name = std::string(&name[0]);
	return skill;
}

Event *Loader::LoadEvent() {
	Event *event = new Event();
	(*Reader)
		.read(&event->time)
		.read(&event->src_agent)
		.read(&event->dst_agent)
		.read(&event->value)
		.read(&event->buff_dmg)
		.read(&event->overstack_value)
		.read(&event->skillid)
		.read(&event->src_instid)
		.read(&event->dst_instid)
		.read(&event->src_master_instid)
		.read(&event->dst_master_instid)
		.read(&event->iff)
		.read(&event->buff)
		.read(&event->result)
		.read(&event->is_activation)
		.read(&event->is_buffremove)
		.read(&event->is_ninety)
		.read(&event->is_fifty)
		.read(&event->is_moving)
		.read(&event->is_statechange)
		.read(&event->is_flanking)
		.read(&event->is_shields)
		.read(&event->is_offcycle)
		.read(&event->pad61)
		.read(&event->pad62)
		.read(&event->pad63)
		.read(&event->pad64);
	return event;
}

