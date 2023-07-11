#pragma once

#include <memory>

#include "BinaryReader.h"

class Encounter;
class Agent;
class Skill;
class Event;

class Loader {
public:
	Loader(BinaryReader *r)
		: Reader(r)
	{
	}
	Encounter *LoadEncounter();
	Agent *LoadAgent();
	Skill *LoadSkill();
	Event *LoadEvent();
private:
	BinaryReader *Reader;
};