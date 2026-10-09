# Special pad reader

The input subsystem contains a derived pad reader for two special-controller
hardware variants. Its vtable begins at `0x195EB10`, and the game keeps two
instances at `0x1AD4280` and `0x1AD6398`.

## Private pad imports

Three `libScePad` imports are absent from the PS4 SDK 5.008 public stub:

| NID | Recovered name | PLT stub |
| --- | --- | ---: |
| `hGbf2QTBmqc` | `scePadGetExtControllerInformation` | `0x1245100` |
| `WFIiSfXGUq8` | `scePadOpenExt` | `0x1245110` |
| `xqgVCEflEDY` | `scePadSetFeatureReport` | `0x1245120` |

The names come from a [generated public NID map](https://github.com/DNNDHH/PS5-3.20_Libs/blob/main/libScePad.c)
and agree with every call site:
the open function receives a user ID, special port type, index, and VID/PID
parameters; the information function receives a handle and output structure;
and the feature-report function receives a handle, report ID, data pointer, and
size. The reviewed mappings live in `tools/ps4_known_nids.csv` so import export
remains reproducible.

## Hardware probing

`PembrokeGuitarController::Activate` at `0x8D28A0` opens the normal special-pad path, then
probes these extended device identifiers:

| Game hardware ID | Vendor ID | Product ID |
| ---: | ---: | ---: |
| `0x1F` | `0x0738` | `0x8261` |
| `0x20` | `0x0E6F` | `0x0173` |

Each six-byte open parameter stores the product ID twice. A probe counts as
successful when byte 11 of the returned extended controller information is
nonzero; that byte is the controller's connected-count field.

## Calibration sample path

The virtual method at `0x8D3060` selects calibration mode 0, 1, or 2 with HID
feature report `0x30`. The five report bytes are `30 01 08 00 xx`, where the
last byte is respectively `01`, `FF`, or `43`. A failed write is retried once
after one millisecond.

While calibration is active, `0x8D2EA0` consumes 120-byte `ScePadData` records
and maintains a rolling buffer of 64 floats. Mode 1 converts device-unique byte
9 directly to a float. The other active mode reads the little-endian 16-bit
value at device-unique bytes 7 and 8 and stores its square root. `0x8D3000`
copies the 64 values and their count to the caller, then clears the stored
count. Callers around `0xDAC0C0` use modes 1 and 2 during the game's calibration
sequence.

The mode names remain numeric because the binary proves the data paths but does
not identify which physical calibration sensor each mode selects.
