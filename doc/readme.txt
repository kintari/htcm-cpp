writeencounter.cpp has code used to write evtc files.
check file header - evtc bytes for filetype, arcdps build yyyymmdd for compatibility, target species id for boss.
an npcid of 1 indicates log is wvw.
an npcid of 2 indicates log is map
nameless/combatless agents may not be written to table while appearing in events.
skill and agent names use utf8 encoding.
evtc_agent.name is a combo string on players - character name <null> account name <null> subgroup str literal <null>.
add u16 field to the agent table, agents[x].instance_id, initialized to 0.
add u64 fields agents[x].first_aware initialized to 0, and agents[x].last_aware initialized to u64max.
add u64 field agents[x].master_addr, initialized to 0.
if evtc_agent.is_elite == 0xffffffff && upper half of evtc_agent.prof == 0xffff, agent is a gadget with pseudo id as lower half of evtc_agent.prof (volatile id).
if evtc_agent.is_elite == 0xffffffff && upper half of evtc_agent.prof != 0xffff, agent is a npc with species id as lower half of evtc_agent.prof (reliable id).
if evtc_agent.is_elite != 0xffffffff, agent is a player with profession as evtc_agent.prof and elite spec as evtc_agent.is_elite.
gadgets do not have true ids and are generated through a combination of gadget parameters - they will collide with npcs and should be treated separately.
iterate through all events, assigning instance ids and first/last aware ticks.
set agents[x].instance_id = src_instid where agents[x].addr == src_agent && !is_statechange.
set agents[x].first_aware = time on first event, then all consecutive event times to agents[x].last_aware.
iterate through all events again, this time assigning master agent.
set agents[z].master_agent on encountering src_master_instid != 0.
agents[z].master_addr = agents[x].addr where agents[x].instance_id == src_master_instid && agent[x].first_aware < time < last_aware.
iterate through all events one last time, this time parsing for the data you want.
src_agent and dst_agent should be used to associate event data with local data.
parse event type order should check is_statechange > is_activation > is_buffremove > the rest.
is_statechange will do the heavy lifting of non-internal and parser-requested events - make sure to ignore unknown statechange types.

---

common to most events:
time - timegettime64() at time of registering the event.
src_agent - agent the caused the event.
dst_agent - agent the event happened to.
skillid - skill id of relevant skill (can be zero).
src_instid - id of agent as appears in game at time of event.
dst_instid - id of agent as appears in game at time of event.
src_master_instid - if src_agent has a master (eg. is minion), will be equal to instid of master, zero otherwise.
dst_master_instid - if dst_agent has a master (eg. is minion), will be equal to instid of master, zero otherwise.
iff - current affinity of src_agent and dst_agent, is of enum iff.
is_ninety - src_agent is above 90% health.
is_fifty - dst_agent is below 50% health.
is_moving - src_agent is moving at time of event.
is_flanking - src_agent is flanking dst_agent at time of event.

statechange events:
is_statechange will be non-zero of enum cbtstatechange.
refer to enum comments below for handling individual statechange types.

is_activation events:
is_statechange will be zero.
is_activation will be non-zero of enum cbtactivation.
cancel_fire or cancel_cancel, value will be the ms duration of the time spent in animation.
cancel_fire or cancel_cancel, buff_dmg will be the ms duration of the scaled (as if not affected) time spent.
normal or quickness, value will be the ms duration at which all significant effects have happened.
normal or quickness, buff_dmg will be the ms duration at which control is expected to be returned to character.
dst_agent will be x/y of target of skill effect.
overstack_value will be z of target of skill effect.

is_buffremove events:
is_statechange and is_activation will be zero.
is_buffremove will be non-zero of enum cbtbuffremove.
buff will be non-zero.
src_agent is agent that had buff removed, dst_agent is the agent that removed it.
value will be the remaining time removed calculated as duration.
buff_dmg will be the remaining time removed calculated as intensity (warning: can overflow on cbtb_all, use as sum check only).
result will be the number of stacks removed (cbtb_all only).
pad61-64 will be buff instance id of buff removed (cbtb_single only).

for all events below, statechange, activation, and buffremove will be zero.

buff apply events:
buff will be non-zero.
buff_dmg will be zero.
value will be non-zero duration applied.
pad61-64 will be buff instance id of the buff.
is_shields will be stack active status.
if is_offcycle is zero, overstack_value will be duration of the stack that is expected to be removed.
if is_offcycle is non-zero, overstack_value will be the new duration of the stack, value will be duration change (no new buff added).
NOTE: for data eg. category, use CBTS_BUFFINFO.
logs will always contain skill ID data for 718 719 723 725 726 738 742 861 873 19426 30328 717 736 42883 29025 29466 43499 44871 51683
NOTE: 4/29/2021+ delays buffapply til end of combat unless both agents are in squad

buff damage events:
buff will be non-zero.
value will be zero.
buff_dmg will be the simulated damage dealt.
is_offcycle will be zero if damage accumulated by tick, non-zero if reactively (eg. confusion skill use)
result will be zero if damage expected to hit, 1 for invuln by buff, 2/3/4 for invuln by player skill
pad61 will be 1 if dst is currently downed.

direct damage events:
buff will be zero.
value will be the combined shield+health damage dealt.
overstack_value will be the shield damage dealt.
is_offcycle will be 1 if dst is currently downed.
result will be of enum cbtresult.

---

/* is friend/foe
enum iff {
	IFF_FRIEND,
	IFF_FOE,
	IFF_UNKNOWN
};

/* combat result (direct) */
enum cbtresult {
	CBTR_NORMAL, // strike was neither crit or glance
	CBTR_CRIT, // strike was crit
	CBTR_GLANCE, // strike was glance
	CBTR_BLOCK, // strike was blocked eg. mesmer shield 4
	CBTR_EVADE, // strike was evaded, eg. dodge or mesmer sword 2
	CBTR_INTERRUPT, // strike interrupted something
	CBTR_ABSORB, // strike was "invulned" or absorbed eg. guardian elite
	CBTR_BLIND, // strike missed
	CBTR_KILLINGBLOW, // not a damage strike, target was killed by skill by
	CBTR_DOWNED, // not a damage strike, target was downed by skill by
	CBTR_BREAKBAR, // not a damage strike, target had value of breakbar damage dealt
  CBTR_ACTIVATION, // not a damage strike, on-activation event (src hit dst if damaging buff)
	CBTR_UNKNOWN
};

/* combat activation */
enum cbtactivation {
	ACTV_NONE, // not used - not this kind of event
	ACTV_START, // started skill/animation activation
	ACTV_QUICKNESS_UNUSED, // unused as of nov 5 2019
	ACTV_CANCEL_FIRE, // stopped skill activation with reaching tooltip time
	ACTV_CANCEL_CANCEL, // stopped skill activation without reaching tooltip time
	ACTV_RESET, // animation completed fully
	ACTV_UNKNOWN
};

/* combat state change */
enum cbtstatechange {
	CBTS_NONE, // not used - not this kind of event
	CBTS_ENTERCOMBAT, // src_agent entered combat, dst_agent is subgroup
	CBTS_EXITCOMBAT, // src_agent left combat
	CBTS_CHANGEUP, // src_agent is now alive
	CBTS_CHANGEDEAD, // src_agent is now dead
	CBTS_CHANGEDOWN, // src_agent is now downed
	CBTS_SPAWN, // src_agent is now in game tracking range (not in realtime api)
	CBTS_DESPAWN, // src_agent is no longer being tracked (not in realtime api)
	CBTS_HEALTHUPDATE, // src_agent is at health percent. dst_agent = percent * 10000 (eg. 99.5% will be 9950) (not in realtime api)
	CBTS_LOGSTART, // log start. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp
	CBTS_LOGEND, // log end. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp
	CBTS_WEAPSWAP, // src_agent swapped weapon set. dst_agent = current set id (0/1 water, 4/5 land)
	CBTS_MAXHEALTHUPDATE, // src_agent has had it's maximum health changed. dst_agent = new max health (not in realtime api)
	CBTS_POINTOFVIEW, // src_agent is agent of "recording" player  (not in realtime api)
	CBTS_LANGUAGE, // src_agent is text language  (not in realtime api)
	CBTS_GWBUILD, // src_agent is game build  (not in realtime api)
	CBTS_SHARDID, // src_agent is sever shard id  (not in realtime api)
	CBTS_REWARD, // src_agent is self, dst_agent is reward id, value is reward type. these are the wiggly boxes that you get
	CBTS_BUFFINITIAL, // combat event that will appear once per buff per agent on logging start (statechange==18, buff==18, normal cbtevent otherwise)
	CBTS_POSITION, // src_agent changed, cast float* p = (float*)&dst_agent, access as x/y/z (float[3]) (not in realtime api)
	CBTS_VELOCITY, // src_agent changed, cast float* v = (float*)&dst_agent, access as x/y/z (float[3]) (not in realtime api)
	CBTS_FACING, // src_agent changed, cast float* f = (float*)&dst_agent, access as x/y (float[2]) (not in realtime api)
	CBTS_TEAMCHANGE, // src_agent change, dst_agent new team id
	CBTS_ATTACKTARGET, // src_agent is an attacktarget, dst_agent is the parent agent (gadget type), value is the current targetable state (not in realtime api)
	CBTS_TARGETABLE, // dst_agent is new target-able state (0 = no, 1 = yes. default yes) (not in realtime api)
	CBTS_MAPID, // src_agent is map id  (not in realtime api)
	CBTS_REPLINFO, // internal use, won't see anywhere
	CBTS_STACKACTIVE, // src_agent is agent with buff, dst_agent is the stackid marked active
	CBTS_STACKRESET, // src_agent is agent with buff, value is the duration to reset to (also marks inactive), pad61-pad64 buff instance id
	CBTS_GUILD, // src_agent is agent, dst_agent through buff_dmg is 16 byte guid (client form, needs minor rearrange for api form)
	CBTS_BUFFINFO, // is_flanking = probably invuln, is_shields = probably invert, is_offcycle = category, pad61 = stacking type, pad62 = probably resistance, src_master_instid = max stacks, overstack_value = duration cap (not in realtime)
	CBTS_BUFFFORMULA, // (float*)&time[8]: type attr1 attr2 param1 param2 param3 trait_src trait_self, (float*)&src_instid[2] = buff_src buff_self, is_flanking = !npc, is_shields = !player, is_offcycle = break, overstack = value of type determined by pad61 (none/number/skill) (not in realtime, one per formula)
	CBTS_SKILLINFO, // (float*)&time[4]: recharge range0 range1 tooltiptime (not in realtime)
	CBTS_SKILLTIMING, // src_agent = action, dst_agent = at millisecond (not in realtime, one per timing)
	CBTS_BREAKBARSTATE, // src_agent is agent, value is u16 game enum (active, recover, immune, none) (not in realtime api)
	CBTS_BREAKBARPERCENT, // src_agent is agent, value is float with percent (not in realtime api)
	CBTS_ERROR, // (char*)&time[32]: error string (not in realtime api)
	CBTS_TAG, // src_agent is agent, value is the id (volatile, game build dependent) of the tag, buff will be non-zero if commander
	CBTS_BARRIERUPDATE,  // src_agent is at barrier percent. dst_agent = percent * 10000 (eg. 99.5% will be 9950) (not in realtime api)
	CBTS_STATRESET,  // with arc ui stats reset (not in log), src_agent = npc id of active log
	CBTS_EXTENSION, // cbtevent with statechange byte set to this
	CBTS_APIDELAYED, // cbtevent with statechange byte set to this
	CBTS_INSTANCESTART, // src_agent is ms time at which the instance likely was started
	CBTS_TICKRATE, // every 500ms, src_agent = 25 - tickrate (when tickrate < 21)
	CBTS_LAST90BEFOREDOWN, // src_agent is enemy agent that went down, dst_agent is time in ms since last 90% (for downs contribution)
	CBTS_EFFECT, // src_agent is owner. dst_agent if at agent, else &value = float[3] xyz, &iff = float[2] xy orient, &pad61 = float[1] z orient, skillid = effectid. if is_flanking: duration = trackingid. &is_shields = uint16 duration. if effectid = 0, end &is_shields = trackingid (not in realtime api)
	CBTS_IDTOGUID, // &src_agent = 16byte persistent content guid, overstack_value is of contentlocal enum, skillid is content id  (not in realtime api)
	CBTS_LOGNPCUPDATE, // log npc update. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp. src_agent = species id. dst_agent = agent, flanking = is gadget
	CBTS_IDLEEVENT, // internal use, won't see anywhere
	CBTS_EXTENSIONCOMBAT, // cbtevent with statechange byte set to this, treats skillid as skill for evtc skill table
	CBTS_UNKNOWN
};

/* combat buff remove type */
enum cbtbuffremove {
	CBTB_NONE, // not used - not this kind of event
	CBTB_ALL, // last/all stacks removed (sent by server)
	CBTB_SINGLE, // single stack removed (sent by server). will happen for each stack on cleanse
	CBTB_MANUAL, // single stack removed (auto by arc on ooc or all stack, ignore for strip/cleanse calc, use for in/out volume)
 	CBTB_UNKNOWN
};

/* combat buff cycle type */
enum cbtbuffcycle {
	CBTC_CYCLE, // damage happened on tick timer
	CBTC_NOTCYCLE, // damage happened outside tick timer (resistable)
	CBTC_NOTCYCLENORESIST, // BEFORE MAY 2021: the others were lumped here, now retired
	CBTC_NOTCYCLEDMGTOTARGETONHIT, // damage happened to target on hitting target
	CBTC_NOTCYCLEDMGTOSOURCEONHIT, // damage happened to source on htiting target
	CBTC_NOTCYCLEDMGTOTARGETONSTACKREMOVE, // damage happened to target on source losing a stack
	CBTC_UNKNOWN
};

/* buff formula attributes */
enum e_attribute {
	ATTR_NONE,
	ATTR_POWER,
	ATTR_PRECISION,
	ATTR_TOUGHNESS,
	ATTR_VITALITY,
	ATTR_FEROCITY,
	ATTR_HEALING,
	ATTR_CONDITION,
	ATTR_CONCENTRATION,
	ATTR_EXPERTISE,
	ATTR_CUST_ARMOR,
	ATTR_CUST_AGONY,
	ATTR_CUST_STATINC,
	ATTR_CUST_PHYSINC,
	ATTR_CUST_CONDINC,
	ATTR_CUST_PHYSREC,
	ATTR_CUST_CONDREC,
	ATTR_CUST_ATTACKSPEED,
	ATTR_CUST_SIPHONINC,
	ATTR_CUST_SIPHONREC,
	ATTR_UNKNOWN = 65535
};

/* custom skill ids */
enum cbtcustomskill {
	CSK_RESURRECT = 1066, // not custom but important and unnamed
	CSK_BANDAGE = 1175, // personal healing only
	CSK_DODGE = 23275 // will occur in is_activation==normal event
};

/* language */
enum gwlanguage {
	GWL_ENG = 0,
	GWL_FRE = 2,
	GWL_GEM = 3,
	GWL_SPA = 4,
	GWL_CN = 5,
};

/* content local enum */
enum n_contentlocal {
	CONTENTLOCAL_EFFECT,
	CONTENTLOCAL_MARKER
};

/* combat event logging (revision 1, when header[12] == 1)
   all fields except time are event-specific, refer to descriptions of events above */
typedef struct cbtevent {
	uint64_t time; /* timegettime() at time of event */
	uint64_t src_agent;
	uint64_t dst_agent;
	int32_t value;
	int32_t buff_dmg;
	uint32_t overstack_value;
	uint32_t skillid;
	uint16_t src_instid;
	uint16_t dst_instid;
	uint16_t src_master_instid;
	uint16_t dst_master_instid;
	uint8_t iff;
	uint8_t buff;
	uint8_t result;
	uint8_t is_activation;
	uint8_t is_buffremove;
	uint8_t is_ninety;
	uint8_t is_fifty;
	uint8_t is_moving;
	uint8_t is_statechange;
	uint8_t is_flanking;
	uint8_t is_shields;
	uint8_t is_offcycle;
	uint8_t pad61;
	uint8_t pad62;
	uint8_t pad63;
	uint8_t pad64;
} cbtevent;

/* combat event (old, when header[12] == 0) */
typedef struct cbtevent {
	uint64_t time;
	uint64_t src_agent;
	uint64_t dst_agent;
	int32_t value;
	int32_t buff_dmg;
	uint16_t overstack_value;
	uint16_t skillid;
	uint16_t src_instid;
	uint16_t dst_instid;
	uint16_t src_master_instid;
	uint8_t iss_offset;
	uint8_t iss_offset_target;
	uint8_t iss_bd_offset;
	uint8_t iss_bd_offset_target;
	uint8_t iss_alt_offset;
	uint8_t iss_alt_offset_target;
	uint8_t skar;
	uint8_t skar_alt;
	uint8_t skar_use_alt;
	uint8_t iff;
	uint8_t buff;
	uint8_t result;
	uint8_t is_activation;
	uint8_t is_buffremove;
	uint8_t is_ninety;
	uint8_t is_fifty;
	uint8_t is_moving;
	uint8_t is_statechange;
	uint8_t is_flanking;
	uint8_t is_shields;
	uint8_t is_offcycle;
	uint8_t pad64;
} cbtevent;
