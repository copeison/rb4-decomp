# Orbis compressed skinned mesh

`SkinnedCompressed` extends the 52-byte compressed unskinned record with
12 bytes of packed skinning data, producing a 64-byte vertex:

```text
offset 0x00  float position[3]
offset 0x0C  int16 packed_normal[4]
offset 0x14  int16 packed_tangent[4]
offset 0x1C  int16 packed_bitangent[4]
offset 0x24  uint16 packed_color[4]
offset 0x2C  uint16 packed_texture_coordinate[2]
offset 0x30  uint16 packed_secondary_texture_coordinate[2]
offset 0x34  uint16 packed_bone_weights[4]
offset 0x3C  uint32 packed_bone_indices
```

The format descriptor at `0x4430C0` proves the `0x40` stride and attribute
offsets. Its final two entries describe four packed weight components at
offset `0x34` and four packed bone-index components at offset `0x3C`. The
growth routine at `0x8E09A0` zero-initializes each complete record.

The format virtual at `0x8E04F0` returns ID 7. CPU accessors span `0x8E0500`
through `0x8E065F`, with backend finalization at `0x8E0660` and alternating
dirty-buffer upload at `0x8E06A0`. Vertex and index rebuilds are at `0x8E1070`
and `0x8E1260`.

IDA now contains every concrete and base method, secondary-interface
destructor, vector helper, and backend rebuild boundary through `0x8E154F`.
