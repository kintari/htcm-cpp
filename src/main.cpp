#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <stddef.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <vector>

#include "Agent.h"
#include "BinaryReader.h"
#include "Encounter.h"
#include "Event.h"
#include "Loader.h"
#include "Skill.h"
#include "json.h"

// mechanics check
// who hit orb
// cc check

typedef std::pair<std::string, std::string> string_pair;

template <typename... Ts> void print(Ts... ts) {
	(std::cout << ... << ts);
}

namespace json {

	std::string escape(const std::string &s) {
		std::string result;
		for (size_t i = 0; i < s.size(); i++) {
			char ch = s[i];
			if (ch == '"') {
				result += std::string("\\") + ch;
			}
			else {
				result += ch;
			}
		}
		return result;
	}
}

class Main {
public:

  Main(const std::vector<std::string> &&args) : Args(std::move(args)) {}

  int Run();

protected:

	struct StrikeDamage {
		uint64_t Time;
		Agent &Attacker, &Target;
		Skill &Skill;
		int Amount;
	};

	struct BuffRemove {
		uint64_t Time;
		Agent &Instigator, &Target;
		Skill &Buff;
	};

  struct EventData {
    Encounter *Encounter;
    uint64_t Time;
    Event *Event;
    Agent *Agent;
    Skill *Skill;
  };

  void OnStrikeDamage(const StrikeDamage&);
	void OnBuffRemove(const BuffRemove&);

  void OnReveal(const EventData &);
  void OnDown(const EventData &);
  void OnUp(const EventData &);
  void OnDead(const EventData &);
  void OnHitOrb(const EventData &);
  void OnMissGreen(const EventData &);
  void OnBreakbarState(const EventData &);
  void OnBreakbarDamage(const EventData &);

private:

  const std::vector<std::string> Args;
};

std::string quote(const std::string &s) { return "\"" + s + "\""; }

template <typename K, typename V>
V get_or(const std::map<K, V> &m, const K &key, const V &value) {
  auto iter = m.find(key);
  return iter == m.end() ? value : iter->second;
}

int Main::Run() {
  if (Args.size() > 0) {
    for (auto &arg : Args) {
      BinaryReader reader(arg);
      Loader loader(reader);
      Encounter *encounter = loader.LoadEncounter();
      std::map<int, std::string> professions;
      professions[3] = "Engineer";
      professions[7] = "Mesmer";
      std::map<int, std::string> specializations;
      specializations[66] = "Virtuoso";

      /*
            for (size_t i = 0; i < encounter->Agents.size(); i++) {
              std::vector<string_pair> row;
              auto &agent = encounter->Agents[i];
              if (agent->IsPlayer()) {
                int prof = static_cast<int>(agent->prof);
                int elite = static_cast<int>(agent->elite);
              } else if (agent->IsNPC()) {
                row.emplace_back("type", "npc");
                row.emplace_back("species",
         std::to_string(agent->GetSpecies())); } else { row.emplace_back("type",
         "gadget");
              }
              to_json(std::cout, row);
                                      std::cout << std::endl;
            }
      */

      for (auto skill : encounter->Skills) {
        // std::cout << skill->id << std::endl;
        // std::cout << std::string(&skill->name[0]) << std::endl;
      }

      auto time_base = encounter->Events[0]->time;
      for (size_t i = 0; i < encounter->Events.size(); i++) {
        Event *event = encounter->Events[i];
        uint64_t time = event->time - time_base;
        Agent *src_agent = encounter->GetAgentByAddr(event->src_agent);
        Agent *dst_agent = encounter->GetAgentByAddr(event->dst_agent);
				Skill *skill = encounter->GetSkill(event->skillid);
        if (event->is_statechange) {
					switch (event->is_statechange) {
						case CBTS_CHANGEDEAD:
							OnDead(EventData{encounter, time, event, src_agent});
							break;
						case CBTS_CHANGEDOWN:
							OnDown(EventData{encounter, time, event, src_agent});
							break;
						case CBTS_CHANGEUP:
							OnUp(EventData{encounter, time, event, src_agent});
							break;
					}
        } else if (event->is_activation) {

        } else if (event->is_buffremove) {
					OnBuffRemove(BuffRemove { time, *dst_agent, *src_agent, *skill });
        } else if (event->buff != 0) {
					
        } else {
					if (src_agent) {
         		OnStrikeDamage(StrikeDamage { time, *src_agent, *dst_agent, *skill, event->value });
					}
        }
      }
    }
  }
  return 0;
}

void Main::OnStrikeDamage(const StrikeDamage &strike) {
//std::string skill_name = skill == nullptr ? "unknown_skill" : &skill->name[0];
//std::string attacker_name = attacker ? attacker->name : "???";
//std::string target_name = target ? target->name : "???";
	if (strike.Attacker.IsPlayer()) {
		print(
			"{\n",
			"\t", "\"player\": {\n",
			"\t\t", "\"name\": \"", strike.Attacker.name, "\"\n",
			"\t},\n",
			"\t", "\"event\": {\n",
			"\t\t", "\"time\": ", strike.Time, ",\n",
			"\t\t", "\"type\": \"strike\",\n",
			"\t\t", "\"target\": \"", strike.Target.name, "\",\n",
			"\t\t", "\"skill\": \"", json::escape(strike.Skill.name), "\",\n",
			"\t\t",	"\"damage\": ", strike.Amount, "\n",
			"\t", "}\n",
			"}\n"
		);
	}
	else {
		/*
		print(
			"{",
				"\"player\": { \"name\": \"", attacker_name, "\",",
				"\"event\": {",
					"\"time\": ", time, ", ",
					"\"type\": \"strike\",",
					"\"target\": \"", target_name, "\",",
					"\"skill\": \"", skill_name, "\",",
					"\"damage\": ", value, "\"",
				"}",
			"}\n"
		);
		*/
	}
}

void Main::OnBuffRemove(const BuffRemove& data) {
	//if (data.Instigator.IsPlayer)
  print(
		"{",
			"\"event\": { \"time\": ", data.Time, "}",
			"\"type\": \"buff_remove\"," 
			"\"buff\": {"
				"\"id\": ", data.Buff.id, ","
				"\"name\": \"", data.Buff.name, "\"",
			"}",
		"}",
	"}\n"
);
}

void Main::OnDown(const EventData &data) {
  print("{ \"player\": { \"name\": \"", data.Agent->name, "\" }, \"event\": { \"time\": ", data.Time, ", \"type\": \"down\" } }\n");
}

void Main::OnUp(const EventData &data) {
  print("{ \"player\": { \"name\": \"", data.Agent->name, "\" }, \"event\": { \"time\": ", data.Time, ", \"type\": \"up\" } }\n");
}

void Main::OnDead(const EventData &data) {
  print("{ \"player\": { \"name\": \"", data.Agent->name, "\" }, \"event\": { \"time\": ", data.Time, ", \"type\": \"dead\" } }\n");
}

int main(int argc, const char *argv[]) {

  std::vector<std::string> args;
  for (int i = 1; i < argc; i++)
    args.push_back(argv[i]);

  try {
  	Main app(std::move(args));
  	app.Run();
  	return 0;
  }
  catch (std::exception& e) {
  	std::cerr << "error: " << e.what() << std::endl;
  	return -1;
  }

}