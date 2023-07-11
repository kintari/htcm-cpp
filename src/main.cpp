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
#include <ranges>
#include <regex>
#include <set>
#include <vector>

#include "Agent.h"
#include "BinaryReader.h"
#include "Encounter.h"
#include "Event.h"
#include "Loader.h"
#include "Skill.h"
#include "Phase.h"

#include "MemoryStream.h"
#include "zip.h"

#include <nlohmann/json.hpp>

#include "zlib.h"

using json = nlohmann::json;



// phase transitions
// cause of death
// mechanics check
// who hit orb
// cc check (breakdown, timings)
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

// invisibility: 728
// hide in shadows: 10269
// stealth: 13017
// dwarf rite: 26596
// diminished: 46668
// targeted: 60517
// influence of the void: 64524
// infirmity: 67965
// debilitated: 67972
// voidwalker: 68066
// void empowerment: 68083

void summarize(Encounter *encounter) {

	auto time_base = encounter->Events[0]->time;
	auto phase = Phase::NONE;

	uint64_t max_time = 0;
	for (auto event: encounter->Events) {
		max_time = std::max(max_time, event->time);
	}
	
	for (auto e: encounter->Events) {
		if (!e->is_statechange && e->src_agent != 0) {
			Agent *src_agent = encounter->GetAgentByAddr(e->src_agent);
			if (src_agent != nullptr) {
				src_agent->instance_id = e->src_instid;
			}
		}
	}

	for (auto agent: encounter->Agents) {
		auto f = [=](const Event *e) {
			return !e->is_statechange && e->src_agent == agent->addr;
		};
		auto iter0 = std::find_if(
			encounter->Events.begin(),
			encounter->Events.end(),
			f
		);
		agent->first_aware = iter0 != encounter->Events.end() ? (*iter0)->time : 0;
		auto iter1 = std::find_if(
			encounter->Events.rbegin(),
			encounter->Events.rend(),
			f
		);
		agent->last_aware =
			iter1 != encounter->Events.rend() ?
			(*iter1)->time :
			std::numeric_limits<uint64_t>::max();
	}

	for (auto e: encounter->Events) {
		Agent *src_agent = encounter->GetAgentByAddr(e->src_agent);
		if (src_agent && !e->is_statechange) {
			src_agent->first_aware = std::min(src_agent->first_aware, e->time);
			src_agent->last_aware = std::max(src_agent->last_aware, e->time);
		}
	}

	std::vector<const Agent *> orbs;
	for (auto e: encounter->Events) {
		if (e->is_statechange == CBTS_BREAKBARSTATE) {
			if (e->value == 0) {
				const Agent *src_agent = encounter->GetAgentByAddr(e->src_agent);
				if (src_agent->name == "Void Amalgamate") {
					orbs.push_back(src_agent);
				}
			}
		}
	}

	for (auto e: encounter->Events) {
		const Agent *src_agent = encounter->GetAgentByAddr(e->src_agent);
		if (e->is_statechange == CBTS_SPAWN) {
			if (src_agent) {
				if (phase == Phase::JORMAG && src_agent->name == "Void Warforged") {
					phase = Phase::PRIMORDUS;
				}
				else if (phase == Phase::PRIMORDUS && src_agent->name == "Void Brandbomber") {
					phase = Phase::KRALKATORRIK;
				}
				else if (phase == Phase::KRALKATORRIK && src_agent->name == "Void Time Caster") {
					phase = Phase::PURIFICATION2;
				}
			}
		}
		else if (e->is_statechange == CBTS_BREAKBARSTATE) {
			if (e->value == 0) {
				if (e->src_agent == orbs[0]->addr) {
					phase = Phase::JORMAG;
				}
				else if (e->src_agent == orbs[1]->addr) {
					phase = Phase::MORDREMOTH;
				}
			}
		}
		else if (e->is_statechange == CBTS_HEALTHUPDATE) {
			if (src_agent && src_agent->name == "Void Amalgamate") {
				
			}
		}
	}

	for (auto orb: orbs) {
		println("orb: ", orb->addr);
	}

	
}

typedef struct {
	void *bytes;
	size_t size;
} buf_t;

#define EVTC_FILE_MAGIC 0x43545645

#define ERR_FILE_FORMAT -1

int parse_evtc(const buf_t *buf) {
	
	const uint8_t *bytes = (const uint8_t *) buf->bytes;

	/* check file signature */
	if (*(uint32_t *)(bytes) != EVTC_FILE_MAGIC) {
		return ERR_FILE_FORMAT;
	}
	
	std::vector<uint8_t> vec(bytes, bytes + buf->size);
	MemoryStream stream(std::move(vec));
	BinaryReader reader(&stream);
	Loader loader(&reader);
	Encounter *e = loader.LoadEncounter();
	if (e) {
		summarize(e);
	}
	return 0;
}

static bool verify_fail(const char *cond) {
	fprintf(stderr, "error: failed verify(): %s\n", cond);
	return false;
}

#define verify(cond) ((cond) || verify_fail(#cond))

zip_eocd_t *zip_get_eocd(const buf_t *buf) {
	if (buf->size >= 4) {
		const uint8_t *bytes = (const uint8_t *) buf->bytes;
		for (size_t offset = buf->size - 4; offset >= 0; offset--) {
			uint32_t value = *(const uint32_t *) &bytes[offset];
			if (value == ZIP_EOCD_MAGIC) {
				return (zip_eocd_t *) &bytes[offset];
			}
		}
	}
	return NULL;
}

bool get_file_size(size_t *size_out, FILE * f) {
	struct stat st = { 0 };
	bool result = fstat(fileno(f), &st) == 0;
	if (verify(result))
		if (size_out)
			*size_out = st.st_size;
	return result;
}

bool load_file(buf_t *buf, const char *filename) {
	bool result = false;
	FILE *f = fopen(filename, "rb");
	if (f) {
		size_t size = 0;
		if (get_file_size(&size, f)) {
			uint8_t *bytes = (uint8_t *) malloc(size);
			if (bytes) {
				if (fread(bytes, 1, size, f) == size) {
					buf->bytes = bytes;
					buf->size = size;
					fclose(f);
					return true;
				}
				free(bytes);
				bytes = 0;
			}
		}
		fclose(f);
	}
	return false;
}

bool decompress_file(buf_t *decompressed, const buf_t *compressed) {
	int err;
	z_stream d_stream; /* decompression stream */
	memset(&d_stream, 0, sizeof(d_stream));

	d_stream.next_in  = (uint8_t *) compressed->bytes;
	//d_stream.avail_in = 0;
	d_stream.next_out = (uint8_t *) decompressed->bytes;

	err = inflateInit2(&d_stream, -15);
	if (err != Z_OK) {
		fprintf(stderr, "[zlib error] inflateInit2 -> %s (%d)\n", d_stream.msg, err);
		return false;
	}
	else {
		d_stream.avail_in = compressed->size;
		d_stream.avail_out = decompressed->size;
		err = inflate(&d_stream, Z_FINISH);
		bool ok = err == Z_STREAM_END;
		if (!ok)
			fprintf(stderr, "[zlib error] inflate -> %s (%d)\n", d_stream.msg, err);
		inflateEnd(&d_stream);
		return ok;
	}
}

#define BYTES(x) ((uint8_t *)(x))

void analyze_file(const char *filename) {
	buf_t buf = { NULL, 0 };
	if (load_file(&buf, filename)) {
		//printf("buf.size=%zu\n", buf.size);
		if (buf.size > 0) {
			uint32_t magic = *(uint32_t *) buf.bytes;
			//printf("magic=0x%08x\n", magic);
			if (magic == ZIP_FILE_MAGIC) {
				zip_eocd_t *eocd = zip_get_eocd(&buf);
				if (eocd) {
					zip_cd_file_t *cdir = (zip_cd_file_t *)(((uint8_t *)(buf.bytes))+eocd->cdir_start);
					for (size_t i = 0; i < eocd->num_cdir_recs; i++) {
						zip_cd_file_t *hdr = &cdir[i];
						if (hdr) {
							zip_lfh_t *lfh = (zip_lfh_t *)(BYTES(buf.bytes) + hdr->local_file_header);
							char *lfh_filename = (char *) &lfh[1];
							char *extra = lfh_filename + lfh->filename_len;
							char *data = extra + lfh->extra_field_len;
							//char *comment = extra + hdr->extra_field_len;
							buf_t compressed = {
								data,
								lfh->compressed_size
							};
							buf_t decompressed = {
								malloc(lfh->uncompressed_size),
								lfh->uncompressed_size
							};
							decompress_file(&decompressed, &compressed);
							int res = parse_evtc(&decompressed);
							switch (res) {
								case ERR_FILE_FORMAT:
									fprintf(stderr, "%s[%.*s] is not a valid evtc file\n", filename, lfh->filename_len, lfh_filename);
									break;
							}
							free(decompressed.bytes);
						}
					}
				}
			}
		}
	}
}

int main(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++)
		analyze_file(argv[i]);
}