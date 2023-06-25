#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

#include "MemoryStream.h"
#include "zip.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// cause of death
// mechanics check
// who hit orb
// cc check
// revealed
// food / util
// achievement eligibility


template <typename... Ts>
void print(Ts... ts)
{
	std::stringstream ss;
	(ss << ... << ts);
	std::cout << ss.str();
}

template <typename... Ts>
void println(Ts... ts)
{
	print(ts..., '\n');
}

class Main
{
public:

	Main(const std::vector<std::string> &&args) : Args(std::move(args)) {}

	int Run();

protected:

	struct StrikeDamage {
		uint64_t time;
		Agent &attacker, &target;
		Skill &skill;
		int amount;
	};

	struct BuffRemove {
		uint64_t time;
		Agent &instigator, &target;
		Skill &buff;
	};

	void OnStrikeDamage(const StrikeDamage &);
	void OnBuffRemove(const BuffRemove &);

private:
	const std::vector<std::string> Args;
};

void Main::OnBuffRemove(const BuffRemove& event) {
	//assert(skill); // 'skill' is the buff that was removed
	auto message = json {
		{ "time", event.time },
		{ "agent", {
			{ "id", event.target.addr },
			{ "name", event.target.name }
		}},
		{ "buff", {
			{ "id", event.buff.id },
			{ "name", event.buff.name },
			{ "remove", 1 }
		}}
	};
	std::cout << message << std::endl;
}




int Main::Run() {
/*
	if (Args.size() > 0) {
			if (false) {

				// zip file
				printf("is a zip file\n");

				// locate the 'end of central directory' record
				size_t offset = st.st_size - 4;
				while (offset > 0) {
					uint32_t value = *(const uint32_t *) &data[offset];
					if (value == 0x06054b50) {
						printf("found eocd magic at %zu bytes (eof-%zu)\n", offset, st.st_size - offset);
						zip_eocd_t *eocd = (zip_eocd_t *) &data[offset];
						printf("num_cdir_recs=%d\n", eocd->num_cdir_recs);
						printf("cdir_size=%d\n", eocd->cdir_size);
						printf("cdir_start=%d\n", eocd->cdir_start);


			}
			else if (magic == ) {
				printf("is an evtc file\n");
			}
			else {
				printf("unknown file type\n");
			}
			
			//BinaryReader reader(arg);
			//reader.read(&magic);
			uint32_t magic = 0;
			if (magic == 0x04034b50) {
				// zip file
				printf("is a zip file\n");
				std::array<uint8_t,512> buf;
				
				//fread()
			}
			else if (magic == 0x43545645) {
				printf("is an evtc file\n");
				//Loader loader(reader);
				//Encounter *encounter = loader.LoadEncounter();
				Encounter *encounter = nullptr;
				for (auto skill : encounter->Skills)
				{
					// std::cout << skill->id << std::endl;
					// std::cout << std::string(&skill->name[0]) << std::endl;
				}
				auto time_base = encounter->Events[0]->time;
				for (size_t i = 0; i < encounter->Events.size(); i++)
				{
					Event *event = encounter->Events[i];
					uint64_t time = event->time - time_base;
					Agent *src_agent = encounter->GetAgentByAddr(event->src_agent);
					Agent *dst_agent = encounter->GetAgentByAddr(event->dst_agent);
					Skill *skill = encounter->GetSkill(event->skillid);
					if (event->is_statechange)
					{
							switch (event->is_statechange)
						{
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
					}
					else if (event->is_activation)
					{
					}
					else if (event->is_buffremove)
					{
						
					}
					else if (event->buff != 0)
					{
					}
					else
					{
						auto agent = json {
							{ "id", event->src_agent },
							{ "name", src_agent ? src_agent->name : std::to_string(event->src_agent) }
						};
						auto target = json {
							{ "id", dst_agent->addr },
							{ "name", dst_agent->name }
						};
						auto message = json {
							{ "time", time },
							{ "agent", agent },
							{ "strike", {
								{ "target", target },
								{ "amount", event->value },
								{ "skill", skill->name }
							}}
						};
						std::cout << message << std::endl;
						message = json {
							{ "time", time },
							{ "agent", target },
							{ "struck", {
								{ "attacker", agent },
								{ "amount", event->value },
								{ "skill", skill->name }
							}}
						};
						std::cout << message << std::endl;
						//OnStrikeDamage(StrikeDamage{time, *src_agent, *dst_agent, *skill, event->value});
					}
				}
			}		
		}
	}
	*/
	return 0;
}

void Main::OnStrikeDamage(const StrikeDamage &strike)
{
	// std::string skill_name = skill == nullptr ? "unknown_skill" : &skill->name[0];
	// std::string attacker_name = attacker ? attacker->name : "???";
	// std::string target_name = target ? target->name : "???";
	if (strike.attacker.IsPlayer())
	{
		auto message = json{
			{ "player", {
				{ "name", strike.attacker.name }
			}},
			{ "event", {
				{ "type", "strike" },
				{ "time", strike.time },
				{ "target", strike.target.name },
				{"skill", {
						{ "name", strike.skill.name },
						{ "amount", strike.amount }
				}}
			}}
		};
		std::cout << message << std::endl;
	}
	else
	{
	}
}

#define EVTC_FILE_MAGIC 0x43545645

bool scan_for_magic(size_t *resultp, uint32_t magic, const void *buf, size_t length) {
	const uint8_t *bytes = (const uint8_t *) buf;
	for (size_t offset = 0; offset + 4 < length; offset++) {
		uint32_t value = *(uint32_t *) &bytes[offset];
		if (value == magic) {
			*resultp = offset;
			return true;
		}
	}
	return false;
}

void analyze_file(const char *filename) {
	FILE *f = fopen(filename, "rb");
	if (f) {
		struct stat st = { 0 };
		int res = fstat(fileno(f), &st);
		assert(res == 0);
		printf("st.st_size=%zu\n", st.st_size);
		if (st.st_size > 0) {
			uint8_t *buf = (uint8_t *) malloc(st.st_size);
			assert(buf);
			if (buf) {
				size_t num_read = fread(buf, 1, st.st_size, f);
				assert(num_read == st.st_size);
				if (num_read == st.st_size) {
					uint32_t magic = *(uint32_t *) buf;
					printf("magic=0x%08x\n", magic);
					if (magic == ZIP_FILE_MAGIC) {

						size_t eocd_offset;
						if (scan_for_magic(&eocd_offset, ZIP_EOCD_MAGIC, buf, num_read)) {

							/* keep scanning to find the last occurence */
							while (eocd_offset + 4 < num_read)
								if (!scan_for_magic(&eocd_offset, ZIP_EOCD_MAGIC, &buf[eocd_offset+4], num_read-eocd_offset-4))
									break;
							
							zip_eocd_t *eocd = (zip_eocd_t *) &buf[eocd_offset];
							zip_cd_file_t *cdir = (zip_cd_file_t *) &buf[eocd->cdir_start];
							
							for (size_t i = 0; i < eocd->num_cdir_recs; i++) {
								zip_cd_file_t *hdr = &cdir[i];

								char *filename = (char *) &hdr[1];
								char *extra = filename + hdr->filename_len;
								char *comment = extra + hdr->extra_field_len;
	
								printf("files[%zu]:\n", i);
								printf("  compressed_size: %d\n", hdr->compressed_size);
								printf("  uncompressed_size: %d\n", hdr->uncompressed_size);
								printf("  compression_method: %d\n", hdr->compression_method);
								printf("  local_file_header: %d\n", hdr->local_file_header);
								printf("  filename: '%.*s'\n", hdr->filename_len, (const char *) &hdr[1]);
								printf("  extra: '%.*s'\n", hdr->extra_field_len, extra);
								printf("  comment: '%.*s'\n", hdr->file_comment_len, comment);
							}
						}
					}
				}
			}
		}
		fclose(f);
	}
}

int main(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++)
		analyze_file(argv[i]);
}