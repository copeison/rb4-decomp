#pragma once

// The core runtime's startup and per-frame service. The functions sit
// between math/Trig.o and utl/BinStream.o in this build; the reference map
// has none of them, so the object name and the function names are inferred.

// Starts the core runtime: the random generator, the timers, ThreadCall,
// the trig tables and the core script functions. At 0x219AA0. Name not in
// the reference map.
void core_initialize();
// Polls ThreadCall and the two other core services. SystemPoll calls it.
// Name not in the reference map.
void core_poll();  // 0x219BB0
// Updates TheTimeMgr. SystemPoll calls it after core_poll. Name not in the
// reference map.
void core_update_time();  // 0x219BD0
// Both of the above. Only FmodAudioStreamResource's asynchronous decode
// waits call it. Name not in the reference map.
void core_poll_and_update_time();  // 0x219B80
// Shuts down the core runtime. SystemTerminate calls it last. Name not in
// the reference map.
void core_terminate();  // 0x219BE0
