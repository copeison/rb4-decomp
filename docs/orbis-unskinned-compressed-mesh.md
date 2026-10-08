# Orbis compressed unskinned mesh

`UnskinnedCompressed` reduces the unskinned vertex stride from 80 to 52 bytes:

```text
offset 0x00  float position[3]
offset 0x0C  int16 packed_normal[4]
offset 0x14  int16 packed_tangent[4]
offset 0x1C  int16 packed_bitangent[4]
offset 0x24  uint16 packed_color[4]
offset 0x2C  uint16 packed_texture_coordinate[2]
offset 0x30  uint16 packed_secondary_texture_coordinate[2]
```

The format descriptor at `0x4430C0` proves the `0x34` stride and attribute
offsets 0, 12, 20, 28, 36, 44, and 48. Its format codes distinguish the
signed packed basis vectors from the packed color and texture-coordinate
fields. The growth routine at `0x8DF540` zero-initializes the complete record.

The format virtual at `0x8DF050` returns ID 6. CPU accessors span `0x8DF060`
through `0x8DF1DF`, with backend finalization at `0x8DF1E0` and alternating
dirty-buffer upload at `0x8DF220`. Vertex and index rebuilds are at `0x8DFB70`
and `0x8DFD80`.

IDA now contains all concrete/base methods, secondary-interface destructors,
vector helpers, and backend rebuild boundaries through `0x8E007F`.
