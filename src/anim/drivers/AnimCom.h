#pragma once

#include <cstddef>

#include "anim/core/AnimInstID.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "entity/core/ComMetaData.h"
#include "utl/text/Symbol.h"

class AnimDriverCom;
class AnimOptions;
class Entity;
class ResourcePath;

// The component an instanced object plays its child entity's animations
// through (anim/AnimCom.o, 0x326BE0-0x3276E7; class id "Animator", alias
// "AnimCom"): each call goes to the AnimDriverCom on the root of the
// entity the object's InstanceCom instances. Without a child or a driver
// the calls do nothing. The vtable at 0x18FA020 has 47 slots; slots 41 to
// 46 are the class's own. The object is 24 bytes. The map's methods take
// the object (ObjPtr const&) first; this build's do not.
class AnimCom : public Component {
public:
    AnimCom();  // 0x326BE0
    // Copies the component for an imprint. Inlined into _Imprint
    // (0x3274E0).
    AnimCom(const AnimCom& other) : Component(other) {}
    // Slots 0-1: 0x326C10 (Component's destructor) and 0x326C20.
    ~AnimCom() override {}

    Symbol GetId() const override;          // slot 4: 0x327460
    Symbol GetClassName() const override;   // slot 5: 0x327470
    int CurrentRev() const override;        // slot 7: 0x327480
    bool IsA(Symbol type) const override;   // slot 8: 0x3274A0
    Component* AsComponent() override;      // slot 9: 0x3274D0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x3274E0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x327600
    ComMetaData& _GetMetaData() override;       // slot 23: 0x327610
    // Slot 29: true. The map's AnimCom::_LoadResources(ObjPtr const&) has
    // this body.
    bool _OnResourcesLoaded() override;  // 0x326EA0
    // Slots 31 and 33: empty. The map's _Enter and _Poll.
    void _Enter() override;  // 0x326EB0
    void _Poll() override;   // 0x326EC0
    // Slot 38: polls as in the game mode (slot 33).
    void _EditPoll() override;  // 0x327620

    // Slots 41-46: the driver's calls; the null id, false and nothing
    // without a driver.
    virtual AnimInstID StartAnim(const ResourcePath& path, const AnimOptions& options);  // 0x326ED0
    virtual bool StopAnim(AnimInstID& id);  // 0x327010
    // Set the playback's weight, scale and looping.
    virtual void SetAnimWeight(const AnimInstID& id, float weight);  // 0x3270E0
    virtual void SetAnimScale(const AnimInstID& id, float scale);    // 0x3271D0
    virtual void SetAnimLoop(const AnimInstID& id, bool loop);       // 0x3272C0
    virtual void Clear();  // 0x3273A0

    // The AnimDriverCom on the entity's root, or null.
    static AnimDriverCom* _FindDriver(Entity* entity);  // 0x326FB0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in anim/AnimInit.o at 0x2A7330.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B1CD0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x326C40

    // "Animator", constructed by the static initializer at 0x327630.
    static Symbol sId;  // 0x19FB410
    // Also "Animator". Name not in the reference map.
    static Symbol sClassName;           // 0x19FB418
    static PropRegistry sPropRegistry;  // 0x19FB420
    static ComMetaData sMetaData;       // 0x19FB4C0

private:
    // The driver of the entity the object's InstanceCom instances, or
    // null. Inlined into slots 41 to 46. Name not in the reference map.
    AnimDriverCom* _GetDriver() const;
};

static_assert(sizeof(AnimCom) == 24);
