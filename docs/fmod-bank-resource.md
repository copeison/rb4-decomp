# FModBankResource

The string at `0x125D143` identifies `FModBankResource`; its vtable begins at
`0x18F0C78`. Type metadata at `0x273AF0` registers the `bank` extension under
the `FMod Banks` category.

Path resolution at `0x274710` obtains the engine asset path, replaces a
`desktop` or `Desktop` path component with `PS4`, and recognizes localized
`_eng.bank` and `/eng.bank` suffixes. The localized case replaces the language
suffix through the sound manager before loading.

The loader at `0x274200` visits every active FMOD Studio system. Per-system
loading at `0x274A80` calls `loadBankFile`, loads sample data, services Studio
updates while sample data is loading, locks every bank bus channel group, and
flushes commands. The resource stores one bank per Studio system.

Unload at `0x273980` unloads each bank and continues yielding and updating its
owning Studio system while the bank reports `UNLOADING`. Event and bus queries
at `0x273C10` and `0x273F00` enumerate up to 2,048 objects from the first
loaded bank and return their full FMOD paths.

The load coordinator also tracks `master bank.bank` and
`master bank.strings.bank` as a paired global resource set. Helpers at
`0x274C40` and `0x274CF0` derive either filename from the other, allowing the
missing half to be loaded automatically and stale master references to be
replaced together.
