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

// Per-thread entity state reached through the thread-local descriptor at
// 0x19B03C8. RecordingAudioRenderTarget clears the flag while it loads its
// entity. Names not in the reference map.
struct EntityThreadState {
    void* mUnknown0;
    bool mUnknown8;
};

extern thread_local EntityThreadState gEntityThreadState;
