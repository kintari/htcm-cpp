#pragma once

class BinaryReader;

class Encounter;
class Agent;
class Skill;
class Event;

class Loader {
public:
	Loader(BinaryReader& r) : Reader(r) {}
	Encounter *LoadEncounter();
	Agent *LoadAgent();
	Skill *LoadSkill();
	Event *LoadEvent();
private:
	BinaryReader& Reader;
};