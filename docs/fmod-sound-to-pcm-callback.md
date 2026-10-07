# FMODSoundToPCMCallback

The string at `0x125D06B` identifies `FMODSoundToPCMCallback`. Its vtable begins
at `0x18F0C18`; destruction is at `0x272E60` and the decode worker is at
`0x272F60`.

The worker waits for a nonblocking FMOD sound to finish opening, while measuring
the time spent polling and honoring a cancellation byte. It then reads the
sound's format, default frequency, and PCM-frame length. The destination
consumer chooses a block-frame count from the sample rate, PCM16 format,
channel count, and total length.

The callback allocates one interleaved PCM16 block, seeks to frame zero, and
repeatedly calls `FMOD::Sound::readData`. Every nonempty block is forwarded to
the consumer. A rejected block marks the owning `FmodAudioStreamResource` as a
decode failure and switches to cancellation. The sound is rewound when the
loop ends, then the consumer receives either completion or cancellation.

Teardown releases the FMOD sound, frees the decode buffer, and clears the
resource's active callback under its recursive mutex in the original object.
The cleaned source expresses the callback ownership directly.
