#pragma once

// contents flags are separate bits
// a given brush can contribute multiple content bits
// multiple brushes can be in a single leaf

// these definitions also need to be in q_shared.h!

// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_EMPTY			0		// No contents

#define	CONTENTS_SOLID			0x1		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			0x2		// translucent, but not watery (glass)
#define	CONTENTS_AUX			0x4
#define	CONTENTS_GRATE			0x8		// alpha-tested "grate" textures.  Bullets/sight pass through, but solids don't
#define	CONTENTS_SLIME			0x10
#define	CONTENTS_WATER			0x20
#define	CONTENTS_WINDOW_NOCOLLIDE 0x40	// block AI line of sight
#define CONTENTS_OPAQUE			0x80	// things that cannot be seen through (may be non-solid though)
#define	LAST_VISIBLE_CONTENTS	CONTENTS_OPAQUE

#define ALL_VISIBLE_CONTENTS (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS-1))

#define CONTENTS_TESTFOGVOLUME	0x100
#define CONTENTS_PHYSICSCLIP	0x200	

#define CONTENTS_SOUNDTRIGGER	0x400

#define CONTENTS_NOGRAPPLE		0x800	// no grapples
#define CONTENTS_OCCLUDESOUND	0x1000	// occlude sounds

// ignore CONTENTS_OPAQUE on surfaces that have SURF_NODRAW
#define CONTENTS_IGNORE_NODRAW_OPAQUE	0x2000

// hits entities which are MOVETYPE_PUSH (doors, plats, etc.)
#define CONTENTS_MOVEABLE		0x4000

// remaining contents are non-visible, and don't eat brushes
#define	CONTENTS_TEST_SOLID_BODY_SHOT		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000

#define	CONTENTS_OPERATOR_FLOOR	0x40000
#define	CONTENTS_BLOCKLOS		0x80000
#define	CONTENTS_NOCLIMB		0x100000
#define	CONTENTS_TITANCLIP		0x200000
#define	CONTENTS_BULLETCLIP		0x400000
#define	CONTENTS_OPERATORCLIP	0x800000

#define	CONTENTS_NOAIRDROP		0x1000000	// no air drops

#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEBRIS			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_BLOCK_PING		0x20000000
#define CONTENTS_HITBOX			0x40000000	// use accurate hitboxes on trace

// NOTE: These are stored in a short in the engine now.  Don't use more than 16 bits
#define	SURF_LIGHT		0x0001		// value will hold the light strength
#define	SURF_SKY2D		0x0002		// don't draw, indicates we should skylight + draw 2d sky but not draw the 3D skybox
#define	SURF_SKY		0x0004		// don't draw, but add to skybox
#define	SURF_WARP		0x0008		// turbulent water warp
#define	SURF_TRANS		0x0010
#define SURF_NOPORTAL	0x0020	// the surface can not have a portal placed on it
#define	SURF_TRIGGER	0x0040	// FIXME: This is an xbox hack to work around elimination of trigger surfaces, which breaks occluders
#define	SURF_NODRAW		0x0080	// don't bother referencing the texture

#define	SURF_HINT		0x0100	// make a primary bsp splitter

#define	SURF_SKIP		0x0200	// completely ignore, allowing non-closed brushes
#define SURF_NOLIGHT	0x0400	// Don't calculate light
#define SURF_BUMPLIGHT	0x0800	// calculate three lightmaps for the surface for bumpmapping
#define SURF_NOSHADOWS	0x1000	// Don't receive shadows
#define SURF_NODECALS	0x2000	// Don't receive decals
#define SURF_NOPAINT	SURF_NODECALS	// the surface can not have paint placed on it
#define SURF_NOCHOP		0x4000	// Don't subdivide patches on this surface 
#define SURF_HITBOX		0x8000	// surface is part of a hitbox



// -----------------------------------------------------
// spatial content masks - used for spatial queries (traceline,etc.)
// -----------------------------------------------------
#define	TRACE_MASK_ALL					(0xFFFFFFFF)
// everything that is normally solid
#define	TRACE_MASK_SOLID			(CONTENTS_MONSTER|CONTENTS_MOVEABLE|CONTENTS_PHYSICSCLIP|CONTENTS_GRATE|CONTENTS_WINDOW|CONTENTS_SOLID)
// everything that blocks player movement
#define	TRACE_MASK_PLAYERSOLID		(CONTENTS_MONSTER|CONTENTS_PLAYERCLIP|CONTENTS_MOVEABLE|CONTENTS_GRATE|CONTENTS_WINDOW|CONTENTS_SOLID)
// blocks titan movement
#define	TRACE_MASK_TITANSOLID		(CONTENTS_MONSTER|CONTENTS_TITANCLIP|CONTENTS_MOVEABLE|CONTENTS_GRATE|CONTENTS_WINDOW|CONTENTS_SOLID)
// blocks npc movement
#define	TRACE_MASK_NPCSOLID			(CONTENTS_MONSTER|CONTENTS_MONSTERCLIP|CONTENTS_MOVEABLE|CONTENTS_GRATE|CONTENTS_WINDOW|CONTENTS_SOLID)
// blocks fluid movement
#define	TRACE_MASK_NPCFLUID			(CONTENTS_MONSTER|CONTENTS_MONSTERCLIP|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_SOLID)
// everything normally solid, except monsters (world+brush only)
#define TRACE_MASK_SOLID_BRUSHONLY	(CONTENTS_MOVEABLE|CONTENTS_PHYSICSCLIP|CONTENTS_GRATE|CONTENTS_WINDOW|CONTENTS_SOLID)
// everything normally solid for player movement, except monsters (world+brush only)
#define TRACE_MASK_PLAYERSOLID_BRUSHONLY (CONTENTS_PLAYERCLIP|CONTENTS_MOVEABLE|CONTENTS_GRATE|CONTENTS_WINDOW|CONTENTS_SOLID)
// everything normally solid for npc movement, except monsters (world+brush only)
#define TRACE_MASK_NPCSOLID_BRUSHONLY	(CONTENTS_MONSTERCLIP|CONTENTS_MOVEABLE|CONTENTS_GRATE|CONTENTS_WINDOW|CONTENTS_SOLID)
// water physics in these contents
#define	TRACE_MASK_WATER			(CONTENTS_WATER|CONTENTS_SLIME)
// everything that blocks lighting
#define	TRACE_MASK_OPAQUE			(CONTENTS_MOVEABLE|CONTENTS_OPAQUE|CONTENTS_SOLID)
// everything that blocks lighting, but with monsters added.
#define TRACE_MASK_OPAQUE_AND_NPCS		(TRACE_MASK_OPAQUE|CONTENTS_MONSTER)
// everything that blocks line of sight for AI
#define TRACE_MASK_BLOCKLOS				(CONTENTS_BLOCKLOS|CONTENTS_MOVEABLE|CONTENTS_OPAQUE|CONTENTS_SOLID)
// everything that blocks line of sight for AI plus NPCs
#define TRACE_MASK_BLOCKLOS_AND_NPCS	(TRACE_MASK_BLOCKLOS|CONTENTS_MONSTER)
// everything that blocks line of sight for players
#define	TRACE_MASK_VISIBLE			(TRACE_MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE)
// everything that blocks line of sight for players, but with monsters added.
#define TRACE_MASK_VISIBLE_AND_NPCS	(TRACE_MASK_OPAQUE_AND_NPCS|CONTENTS_MONSTER)
// bullets see these as solid, except monsters (world+brush only)
#define TRACE_MASK_SHOT_BRUSHONLY	(CONTENTS_DEBRIS|CONTENTS_BULLETCLIP|CONTENTS_MOVEABLE|CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_WINDOW|CONTENTS_SOLID)
// bullets see these as solid
#define	TRACE_MASK_SHOT				(CONTENTS_HITBOX|CONTENTS_MONSTER|TRACE_MASK_SHOT_BRUSHONLY)
// bullets see these as solid, except monsters (world+brush only)
#define TRACE_MASK_SHOT_HULL		(CONTENTS_DEBRIS|CONTENTS_MONSTER|CONTENTS_BULLETCLIP|CONTENTS_MOVEABLE|CONTENTS_GRATE|CONTENTS_WINDOW|CONTENTS_SOLID)
// grenades see these as solid
#define TRACE_MASK_GRENADE			(CONTENTS_HITBOX|TRACE_MASK_SHOT_HULL)
// just the world, used for route rebuilding
#define TRACE_MASK_NPCWORLDSTATIC	(CONTENTS_MONSTERCLIP|CONTENTS_GRATE|CONTENTS_WINDOW|CONTENTS_SOLID)

struct matrix3x4_t {
	float m[3][4];
};

struct csurface_t {
	const char* name;
	short surface_prop;
	uint16_t flags;
};

struct cplanetrace_t {
	vec3_t normal;
	float dist;
};

class c_base_trace {
public:
	vec3_t startpos;
	float unk1;
	vec3_t endpos;
	float unk2;
	cplanetrace_t plane;
	float fraction;
	int contents;
	bool allsolid;
	bool startsolid;
};

class c_game_trace : public c_base_trace {
public:
	char gap3A[0x4];
	csurface_t surface;
	float fractionleftsolid;
	int hitgroup;
	short physicsBone;
	char gap5A[0x6];
	c_base_entity* hit_entity;
	int hitbox;
	char gap6C[0x114];
};
static_assert(sizeof(c_game_trace) == 0x180);

typedef c_game_trace trace_t;

struct ray_t {
	vec3_aligned_t m_Start;
	vec3_aligned_t m_Delta;
	vec3_aligned_t m_StartOffset;
	int m_nUnk30;
	int m_nUnk34;
	int m_nUnk38;
	int m_nUnk3C;
	int m_nUnk40;
	int m_nUnk44;
	int m_nFlags;
	const matrix3x4_t* m_pWorldAxisTransform;
	int m_nUnk58;
	bool m_IsRay;
	bool m_IsSwept;
	int m_nFlags2;
	int m_nUnk68;

	void init(
		vec3_t& start,
		vec3_t& end,
		int nFlags1 = 0x3f800000,
		int nFlags2 = 0)
	{
		m_Delta = end - start;
		m_IsSwept = (m_Delta.length_squared() != 0);

		m_nUnk30 = 0;
		m_nUnk34 = 0;
		m_nUnk38 = 0;
		m_nUnk3C = 0;
		m_nUnk40 = 0;
		m_nUnk44 = 0;

		m_nFlags = nFlags1;

		m_pWorldAxisTransform = nullptr;
		m_IsRay = true;

		m_nUnk58 = 0;
		m_nFlags2 = nFlags2;
		m_nUnk68 = 0;

		m_StartOffset.zero();
		m_Start = start;
	}
};
static_assert(sizeof(ray_t) == 0x70);

enum trace_type_t {
	trace_everything = 0,
	trace_world_only,				// NOTE: This does *not* test static props!!!
	trace_entities_only,			// NOTE: This version will *not* test static props
	trace_everything_filter_props,	// NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};

class i_trace_filter {
public:
	virtual bool should_hit_entity(c_base_entity* entity, int contents_mask) = 0;
	virtual trace_type_t get_trace_type() const = 0;
	virtual void unknown() = 0;
};

class c_trace_filter : public i_trace_filter {
public:
	bool should_hit_entity(c_base_entity* entity, int contents_mask) {
		return !(entity == skip);
	}

	virtual trace_type_t get_trace_type() const {
		return trace_everything;
	}

	void unknown() {}

	void* skip;
};

class i_engine_trace {
public:
	virtual void stub_0() const = 0;
	virtual void stub_1() const = 0;

	virtual void clip_ray_to_collideable(
		const ray_t& ray,
		unsigned int mask,
		c_base_entity* entity,
		trace_t* trace) = 0;

	virtual void trace_ray_filtered(
		const ray_t& ray,
		unsigned int mask,
		i_trace_filter* trace_filter,
		trace_t* trace) = 0;

	virtual void trace_ray(
		const ray_t& ray,
		unsigned int mask,
		trace_t* trace) = 0;
};