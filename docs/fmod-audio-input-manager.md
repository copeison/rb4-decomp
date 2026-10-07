# FMOD audio input manager

The vtable at `0x18F0CF0` belongs to a global manager that combines FMOD Studio
bus routing with low-level record-driver discovery. The executable contains no
class-name string, so `FmodAudioInputManager` and `FmodAudioInputDevice` are
descriptive names supported by their behavior.

Each bus route is 24 bytes: an interned path, an `FMOD::Studio::Bus*`, and the
bank-authored volume. `0x2754C0` replaces the route list and `0x275630` resolves
every path with `Studio::System::getBus`. A successful bind reads and retains
the original volume. `0x275740` multiplies runtime gain by that authored value,
`0x275780` controls mute, and `0x2757C0` exposes the bus channel group. Failed
binding clears the partial result and sets a retry flag serviced by `0x2758A0`.

`0x275830` creates a fixed pool of `0x4150`-byte input-device objects. The
device constructor at `0x27B3E0` records its pool slot, defaults the driver ID
to `-1`, and starts with a 48 kHz sample rate. Binding at `0x27B780` queries
FMOD again, requires the expected driver name to match, then records the driver
ID, name, sample rate, and initial playback frequency.

Device reconciliation at `0x275950` first checks every active slot. The helper
at `0x27B880` searches all current record drivers by name and disconnects a
slot when that driver disappears or loses the `FMOD_DRIVER_STATE_CONNECTED`
bit. The manager then enumerates connected drivers, accepts only names ending
in the exact suffix `GENERAL`, suppresses duplicates by name, and binds each new
driver to the first available pool slot. The original notifies the owning
object manager whenever a device is added or removed; the cleaned method
returns a `changed` flag for the caller to perform that notification.
