#pragma once

#include <cstddef>

#include "audio/core/resources/Resource.h"

class Entity;

// A resource that holds an entity (entity/EntityResource.o). Only the
// entity, the first virtual this class adds, and the entry points the
// renderer calls are modelled.
class EntityResource : public Resource {
public:
    // Slot 13. Creates the resource's entity and returns it. The return type
    // is not in the reference map; the callers use the returned entity.
    virtual Entity* CreateEntity();  // 0xFD6A0

    // Loads the entity's resources.
    void LoadResources();  // 0xFD990
    // Enters the entity when there is one, through the virtual at slot 16.
    // The map has EnterEntity(EntityPtr) as a virtual; this build adds this
    // non-virtual wrapper and passes the entity pointer.
    void EnterEntity(Entity* entity);  // 0xFD850
    // Polls the entity when there is one, through the virtual at slot 18.
    // The map has PollEntity(EntityPtr) as a virtual; this build adds this
    // non-virtual wrapper and passes the entity pointer.
    void PollEntity(Entity* entity);  // 0xFD8B0

    Entity* mEntity;  // Name not in the reference map.
};

static_assert(offsetof(EntityResource, mEntity) == 48);

class PollMgr;

// Per-thread poll state reached through the thread-local descriptor at
// 0x19B03C8, which utl/PollMgr.o creates; the map's build has a
// TLSValue<ThreadPollContext> there. RecordingAudioRenderTarget clears the
// flag while it loads its entity. Names not in the reference map.
struct EntityThreadState {
    // The poll manager the thread is polling for, set by the manager's
    // select (0x24F900, 0x24F9B0) and cleared by its deselect (0x24FA90).
    // Entities check it with the flag before deferring their enter and poll
    // work (0xF53A0).
    PollMgr* mPollMgr;
    // While it and mPollMgr are set, an entity that becomes ready on this
    // thread enters and polls at once (0xF53A0); TransEntityResource clears
    // it around EnterEntity (0x1BB580).
    bool mEnterImmediately;
};

extern thread_local EntityThreadState gEntityThreadState;
