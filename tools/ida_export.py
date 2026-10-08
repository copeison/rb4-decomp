#!/usr/bin/env python3
"""Build an IDA database and export a reproducible initial analysis snapshot."""

from __future__ import annotations

import argparse
import csv
import json
import re
from pathlib import Path
from typing import Any, Iterable

from ida_domain import Database
from ida_domain.database import IdaCommandOptions


SOURCE_PATH_RE = re.compile(
    r"(?:[A-Za-z]:[\\/]|/)[^\x00\r\n]{3,240}?"
    r"\.(?:cpp|cxx|cc|c|hpp|hh|h|inl|asm|s|elf)",
    re.IGNORECASE,
)

FOCUSED_DECOMPILATIONS = {
    "thread-affinity-find-group": 0x258C60,
    "thread-affinity-build-cpu-mask": 0x2590B0,
    "game-initialize": 0xA0,
    "game-run-frame": 0x190,
    "game-main": 0x3C0,
    "game-systems-initialize": 0x402C30,
    "game-systems-shutdown": 0x402D30,
    "render-platform-name": 0x363030,
    "render-api-name": 0x1AE4D0,
    "render-api-for-platform": 0x4414A0,
    "orbis-render-api": 0x8D5DE0,
    "audio-get-sample-rate": 0xD3BA0,
    "audio-set-mix-format": 0xD3BC0,
    "audio-dispatch-output-blocks": 0x1127880,
    "audio-output-dispatcher-set-sample-rate": 0x1127B90,
    "audio-register-mix-consumer": 0x1128380,
    "dingo-initialize": 0x33F090,
    "stage-presence-id-to-symbol": 0x997590,
    "ui-layout-id-to-symbol": 0xBB06A0,
    "ui-load-layout-by-id": 0xBB5D40,
    "command-line-mark-switches-handled": 0x252BC0,
    "performance-counter-ticks-to-milliseconds": 0x25C0E0,
    "performance-counter-milliseconds-to-ticks": 0x25C110,
    "special-pad-reader-open": 0x8D28A0,
    "special-pad-reader-append-calibration-samples": 0x8D2EA0,
    "special-pad-reader-take-calibration-samples": 0x8D3000,
    "special-pad-reader-set-calibration-mode": 0x8D3060,
    "audio-update-listener-attributes": 0x262300,
    "audio-clip-fmod-start": 0x266FD0,
    "audio-clip-fmod-dsp-set-position": 0x266CB0,
    "audio-clip-fmod-dsp-read": 0x266CD0,
    "audio-clip-fmod-event-callback": 0x267310,
    "audio-clip-fmod-pause": 0x267BB0,
    "audio-clip-fmod-resume": 0x267C00,
    "audio-clip-fmod-get-channel-position-ms": 0x267C60,
    "audio-clip-fmod-get-position-ms": 0x267CC0,
    "audio-clip-fmod-set-position-ms": 0x267D30,
    "audio-clip-fmod-update": 0x267D70,
    "audio-clip-fmod-process-release": 0x267E50,
    "audio-clip-fmod-update-3d-attributes": 0x267E70,
    "audio-clip-fmod-prepare-for-audio-reset": 0x267F10,
    "audio-clip-fmod-defer-channel-release": 0x267FB0,
    "audio-clip-fmod-release-event-instance": 0x268080,
    "audio-clip-fmod-clear-dsp-user-data": 0x2681E0,
    "audio-clip-fmod-release-dsp": 0x268260,
    "audio-clip-fmod-stop-and-wait": 0x2682A0,
    "audio-clip-fmod-set-event-parameter": 0x2683F0,
    "audio-clip-fmod-get-event-parameter": 0x268410,
    "fmod-audio-bus-generator-return-to-pool": 0x268380,
    "fmod-audio-bus-manager-get-setting": 0x268510,
    "fmod-audio-bus-manager-type-symbol": 0x268520,
    "fmod-audio-bus-manager-extension-symbol": 0x2685C0,
    "fmod-audio-bus-manager-retain": 0x268660,
    "fmod-audio-bus-manager-prepare-all-for-audio-reset": 0x2686D0,
    "fmod-audio-bus-manager-stop-all": 0x268740,
    "fmod-audio-bus-manager-active-handles": 0x2687B0,
    "fmod-audio-bus-manager-set-setting": 0x268900,
    "fmod-audio-bus-manager-initialize-pool": 0x268910,
    "fmod-audio-bus-manager-shutdown-pool": 0x268A60,
    "fmod-audio-bus-manager-acquire": 0x268B40,
    "fmod-audio-bus-manager-create-from-resource": 0x268CA0,
    "fmod-audio-bus-manager-create-from-options": 0x268D80,
    "fmod-audio-bus-generator-initialize-sound": 0x268EF0,
    "fmod-audio-bus-generator-register-bus-path": 0x2692E0,
    "fmod-audio-stream-generator-initialize-pool-slot": 0x2692B0,
    "fmod-audio-stream-generator-destruct": 0x2693B0,
    "fmod-audio-stream-generator-delete": 0x269440,
    "fmod-audio-stream-generator-update": 0x2694D0,
    "fmod-audio-stream-generator-try-start-sound": 0x269980,
    "fmod-audio-stream-generator-apply-volume": 0x269B40,
    "fmod-audio-stream-generator-update-3d": 0x269B70,
    "fmod-audio-stream-generator-request-pause": 0x269BF0,
    "fmod-audio-stream-generator-request-resume": 0x269C00,
    "fmod-audio-stream-generator-get-position-ms": 0x269C10,
    "fmod-audio-stream-generator-set-position-ms": 0x269C60,
    "fmod-audio-stream-generator-set-playback-rate": 0x269C70,
    "fmod-audio-stream-generator-clear-loop-points": 0x269CB0,
    "fmod-audio-stream-generator-set-loop-points": 0x269CD0,
    "fmod-audio-stream-generator-prepare-for-audio-reset": 0x269CF0,
    "fmod-audio-stream-generator-stop": 0x269D00,
    "fmod-audio-stream-generator-return-to-pool": 0x269FB0,
    "fmod-audio-stream-manager-create-fallback": 0x26A270,
    "fmod-audio-stream-manager-get-setting": 0x26A280,
    "fmod-audio-stream-manager-type-symbol": 0x26A290,
    "fmod-audio-stream-manager-extension-symbol": 0x26A330,
    "fmod-audio-stream-manager-retain": 0x26A3D0,
    "fmod-audio-stream-manager-prepare-all-for-audio-reset": 0x26A450,
    "fmod-audio-stream-manager-stop-all": 0x26A4C0,
    "fmod-audio-stream-manager-active-handles": 0x26A530,
    "fmod-audio-stream-manager-set-setting": 0x26A680,
    "fmod-audio-stream-manager-initialize-pool": 0x26A690,
    "fmod-audio-stream-manager-shutdown-pool": 0x26A850,
    "fmod-audio-stream-generator-type-symbol": 0x26AA20,
    "fmod-buffered-stream-manager-create-from-resource": 0x26AB60,
    "fmod-buffered-stream-manager-create-from-options": 0x26AC30,
    "fmod-buffered-stream-generator-initialize": 0x26ADA0,
    "fmod-buffered-stream-generator-initialize-pool-slot": 0x26AFF0,
    "fmod-buffered-stream-generator-create-sound": 0x26B060,
    "fmod-buffered-stream-generator-initialize-buffers": 0x26B150,
    "fmod-buffered-stream-generator-acquire-bus-generator": 0x26B490,
    "fmod-buffered-stream-generator-update": 0x26B9A0,
    "fmod-buffered-stream-generator-finish-sound-open": 0x26BAD0,
    "fmod-buffered-stream-generator-set-position-ms": 0x26BDC0,
    "fmod-buffered-stream-generator-return-to-pool": 0x26C1B0,
    "fmod-buffered-stream-generator-refill-buffers": 0x26C480,
    "fmod-buffered-stream-generator-render-callback": 0x26C720,
    "fmod-buffered-stream-generator-compute-sync-adjustment": 0x26D8D0,
    "fmod-buffered-stream-interpolate-optimal32-6p5o": 0x26D940,
    "fmod-buffered-stream-generator-enable-sync": 0x26DAB0,
    "fmod-buffered-stream-generator-set-sync-target-ms": 0x26DBA0,
    "fmod-buffered-stream-generator-disable-sync": 0x26DBD0,
    "fmod-buffered-stream-manager-type-symbol": 0x26DC10,
    "fmod-buffered-stream-manager-extension-symbol": 0x26DCB0,
    "fmod-buffered-stream-manager-retain": 0x26DD50,
    "fmod-buffered-stream-manager-initialize-pool": 0x26E000,
    "fmod-buffered-stream-manager-shutdown-pool": 0x26E270,
    "fmod-buffered-stream-generator-type-symbol": 0x26E3A0,
    "fmod-dialog-generator-initialize-event": 0x26E990,
    "fmod-dialog-programmer-sound-callback": 0x26EA10,
    "fmod-dialog-generator-return-to-pool": 0x26EBE0,
    "fmod-dialog-manager-create": 0x26EC60,
    "fmod-dialog-manager-type-symbol": 0x26EE90,
    "fmod-dialog-manager-extension-symbol": 0x26EF30,
    "fmod-dialog-manager-retain": 0x26EFD0,
    "fmod-dialog-manager-active-handles": 0x26F120,
    "fmod-dialog-manager-initialize-pool": 0x26F280,
    "fmod-dialog-manager-shutdown-pool": 0x26F420,
    "fmod-dialog-generator-pause": 0x26F6E0,
    "fmod-dialog-generator-resume": 0x26F6F0,
    "fmod-dialog-generator-set-event-parameter": 0x26F760,
    "fmod-dialog-generator-get-event-parameter": 0x26F770,
    "fmod-dialog-generator-update": 0x26FAB0,
    "fmod-dialog-generator-stop-and-wait": 0x26FB10,
    "fmod-dialog-generator-type-symbol": 0x26FAC0,
    "fmod-studio-sound-generator-initialize-event": 0x26FC30,
    "fmod-studio-sound-generator-event-callback": 0x26FF50,
    "fmod-studio-sound-generator-update": 0x2702C0,
    "fmod-studio-sound-generator-prepare-for-audio-reset": 0x270620,
    "fmod-studio-sound-generator-stop-and-wait": 0x2706A0,
    "fmod-studio-sound-generator-set-event-parameter": 0x2707D0,
    "fmod-studio-sound-generator-get-event-parameter": 0x270800,
    "fmod-studio-sound-generator-configure-fade": 0x270880,
    "fmod-studio-sound-generator-configure-volume-transition": 0x2709B0,
    "fmod-studio-sound-manager-create": 0x270B20,
    "fmod-studio-sound-manager-type-symbol": 0x270DA0,
    "fmod-studio-sound-manager-extension-symbol": 0x270E40,
    "fmod-studio-sound-manager-retain": 0x270EE0,
    "fmod-studio-sound-manager-active-handles": 0x271030,
    "fmod-studio-sound-manager-initialize-pool": 0x271190,
    "fmod-studio-sound-manager-shutdown-pool": 0x271350,
    "fmod-studio-sound-generator-type-symbol": 0x2715B0,
    "fmod-audio-stream-resource-construct": 0x271660,
    "fmod-audio-stream-resource-destruct": 0x271750,
    "fmod-audio-stream-resource-unregister": 0x271830,
    "fmod-audio-stream-resource-register": 0x271B20,
    "fmod-audio-stream-resource-find": 0x271C20,
    "fmod-audio-stream-resource-build-type-metadata": 0x271D00,
    "fmod-audio-stream-resource-find-or-create": 0x2720F0,
    "fmod-audio-stream-resource-load": 0x272340,
    "fmod-audio-stream-resource-type-symbol": 0x272D20,
    "fmod-audio-stream-resource-is-unavailable": 0x272DF0,
    "fmod-sound-to-pcm-callback-destruct": 0x272E60,
    "fmod-sound-to-pcm-callback-delete": 0x272EE0,
    "fmod-sound-to-pcm-callback-run": 0x272F60,
    "fmod-sound-to-pcm-callback-release-consumer": 0x273150,
    "fmod-bank-resource-construct": 0x273740,
    "fmod-bank-resource-destruct": 0x2738A0,
    "fmod-bank-resource-unload-all": 0x273980,
    "fmod-bank-resource-build-type-metadata": 0x273AF0,
    "fmod-bank-resource-event-paths": 0x273C10,
    "fmod-bank-resource-bus-paths": 0x273F00,
    "fmod-bank-resource-load": 0x274200,
    "fmod-bank-resource-resolve-platform-path": 0x274710,
    "fmod-bank-resource-load-into-studio-system": 0x274A80,
    "fmod-bank-resource-make-strings-bank-path": 0x274C40,
    "fmod-bank-resource-make-master-bank-path": 0x274CF0,
    "fmod-bank-resource-type-symbol": 0x275040,
    "fmod-bank-resource-is-unavailable": 0x275120,
    "fmod-audio-input-manager-destruct": 0x275490,
    "fmod-audio-input-manager-set-bus-paths": 0x2754C0,
    "fmod-audio-input-manager-bind-buses": 0x275630,
    "fmod-audio-input-manager-set-bus-volume": 0x275740,
    "fmod-audio-input-manager-set-bus-mute": 0x275780,
    "fmod-audio-input-manager-get-bus-channel-group": 0x2757C0,
    "fmod-audio-input-manager-initialize-device-slots": 0x275830,
    "fmod-audio-input-manager-retry-bus-bindings": 0x2758A0,
    "fmod-audio-input-manager-clear-bus-paths": 0x2758B0,
    "fmod-audio-input-manager-is-general-record-driver": 0x275910,
    "fmod-audio-input-manager-refresh-record-devices": 0x275950,
    "fmod-audio-input-manager-is-supported": 0x275C00,
    "fmod-recording-target-construct": 0x275E20,
    "fmod-recording-target-destruct": 0x275FB0,
    "fmod-recording-target-start-async-recording": 0x276080,
    "fmod-recording-target-thread-entry": 0x276110,
    "fmod-recording-target-wait-for-thread": 0x276120,
    "fmod-recording-target-suspend-mixer": 0x276130,
    "fmod-recording-target-resume-mixer": 0x276140,
    "fmod-recording-target-get-voice-pool": 0x276170,
    "fmod-recording-target-lock": 0x2761A0,
    "fmod-recording-target-unlock": 0x2761D0,
    "fmod-recording-target-update": 0x276200,
    "fmod-recording-target-register-mix-consumer": 0x276210,
    "fmod-recording-target-unregister-mix-consumer": 0x276230,
    "fmod-recording-target-dispatch-mix-buffers": 0x276250,
    "fmod-recording-target-get-output-dispatcher": 0x276290,
    "fmod-recording-target-get-audio-state": 0x2762B0,
    "fmod-recording-output-callback-invoke": 0x276340,
    "audio-build-fmod-3d-attributes": 0x27ACB0,
    "fmod-audio-input-device-construct": 0x27B3E0,
    "fmod-audio-input-device-bind-record-driver": 0x27B780,
    "fmod-audio-input-device-refresh-record-driver": 0x27B880,
    "fmod-buffered-output-get-num-drivers": 0x2763F0,
    "fmod-buffered-output-get-driver-info": 0x276400,
    "fmod-buffered-output-initialize": 0x276480,
    "fmod-buffered-output-close": 0x2764B0,
    "fmod-buffered-output-update": 0x2764C0,
    "fmod-buffered-output-get-handle": 0x276520,
    "fmod-load-modules-and-set-thread-affinity": 0x261F60,
    "render-system-construct": 0x3DD410,
    "render-system-destruct": 0x3DD790,
    "render-system-initialize": 0x3DDAE0,
    "render-system-initialize-builtin-buffers": 0x3DDC20,
    "render-system-shutdown": 0x3DDE60,
    "orbis-render-system-create": 0x8D5DF0,
    "orbis-render-system-construct": 0x8D77F0,
    "orbis-render-system-destruct": 0x8D79B0,
    "orbis-submit-done-thread-run": 0x8D7340,
    "orbis-submit-done-thread-entry": 0x8D77E0,
    "orbis-render-system-delete": 0x8D7B00,
    "orbis-create-default-vertex-buffer": 0x8D7DB0,
    "orbis-create-identity-instance-buffer": 0x8D7EB0,
    "orbis-render-system-shutdown": 0x8D8040,
    "orbis-render-system-wait-idle": 0x8D8100,
    "orbis-wait-for-gpu-idle": 0x8D8140,
    "orbis-release-retired-allocations": 0x8D8200,
    "orbis-render-system-submit-frame": 0x8D8300,
    "orbis-defer-allocation-release": 0x8D83F0,
    "orbis-release-all-retired-allocations": 0x8D84B0,
    "orbis-create-fence": 0x8D85C0,
    "orbis-create-texture-1d": 0x8D8980,
    "orbis-texture-1d-construct": 0x8E4F60,
    "orbis-create-texture-2d": 0x8D89B0,
    "orbis-texture-2d-construct": 0x8D62C0,
    "orbis-create-texture-3d": 0x8D89E0,
    "orbis-texture-3d-construct": 0x8E53C0,
    "orbis-create-texture-cube": 0x8D8A10,
    "orbis-texture-cube-construct": 0x8E6BA0,
    "orbis-create-texture-array-1d": 0x8D8A40,
    "orbis-texture-array-1d-construct": 0x8E5870,
    "orbis-create-texture-array-2d": 0x8D8A70,
    "orbis-texture-array-2d-construct": 0x8E5D40,
    "orbis-create-texture-array-cube": 0x8D8AA0,
    "orbis-texture-array-cube-construct": 0x8E6640,
    "orbis-fence-destruct": 0x8E15F0,
    "orbis-fence-base-destruct": 0x8E1640,
    "orbis-fence-delete": 0x8E1680,
    "orbis-fence-construct": 0x8E1570,
    "orbis-create-constant-buffer": 0x8D8AD0,
    "orbis-create-compute-buffer": 0x8D8BC0,
    "orbis-compute-buffer-construct": 0x8E3250,
    "orbis-create-particle-buffer": 0x8D8BF0,
    "orbis-particle-buffer-construct": 0x8E2AA0,
    "orbis-create-occlusion-query": 0x8D8C30,
    "orbis-occlusion-query-construct": 0x8E28C0,
    "render-supported-platform-ids": 0x3641B0,
    "render-platform-config-construct": 0x6B9940,
    "render-platform-config-initialize": 0x6B99B0,
    "render-system-poll": 0x3DE0E0,
    "render-system-begin-frame": 0x3DE130,
    "render-system-prepare-frame": 0x3DE170,
    "render-system-attach-frame-owner": 0x3DE3A0,
    "render-system-finish-frame": 0x3DE4A0,
    "render-system-end-frame": 0x3DE7C0,
    "render-system-skip-frame": 0x3DEAA0,
    "screenshot-request": 0x43B120,
    "screenshot-capture-pending": 0x43B130,
    "screenshot-capture-frame": 0x43B140,
    "screenshot-capture-to-file": 0x43B240,
    "render-frame-owner-output-extent": 0x448730,
    "render-frame-owner-draw-mode": 0x448760,
    "render-frame-owner-debug-view": 0x4487C0,
    "render-frame-owner-set-draw-mode": 0x448780,
    "render-frame-owner-set-debug-view": 0x4487E0,
    "render-draw-mode-name": 0x645E40,
    "render-draw-mode-from-name": 0x645E80,
    "render-debug-view-name": 0x6B4EE0,
    "render-debug-view-from-name": 0x6B5510,
    "render-command-take-screenshot": 0x6BABD0,
    "render-command-cycle-screenshot-resolution": 0x6BAC00,
    "render-command-set-shading-mode": 0x6BAC70,
    "render-command-set-shading-mode-for-owner": 0x6BACB0,
    "render-command-set-buffer-inspection-mode": 0x6BAF40,
    "render-command-set-buffer-inspection-mode-for-owner": 0x6BAF80,
    "render-register-debug-commands": 0x6BB0E0,
    "render-settings-initialize": 0x6BB470,
    "render-quality-level-name": 0x442520,
    "render-quality-level-from-name": 0x442540,
    "render-parse-resolution": 0x441940,
    "screenshot-resolution-name": 0x43AF40,
    "render-construct-default-resources": 0x6BDB30,
    "render-initialize-default-resources": 0x6BDCA0,
    "render-release-default-resources": 0x6BF860,
    "render-poll-default-resources": 0x6BFA00,
    "rnd-scene-resource-poll": 0xFD8B0,
    "render-create-default-textures": 0x6BDE60,
    "rnd-scene-resource-construct": 0x4384A0,
    "render-create-compute-buffer": 0x636C70,
    "render-compute-buffer-descriptor-init": 0x636DA0,
    "render-create-default-materials": 0x6BEC50,
    "render-create-default-camera": 0x6BEBC0,
    "render-load-default-lighting": 0x6BEF40,
    "render-create-fallback-default-lighting": 0x6BF4F0,
    "render-apply-default-lighting-mode": 0x6BFA60,
    "render-configure-default-shadowed-spot": 0x6BFD40,
    "render-load-scene-resource": 0x6C0160,
    "render-default-shadow-offset": 0x6BFEA0,
    "render-get-default-texture": 0x6C00F0,
    "rnd-material-set-shader-graph": 0x4F3790,
    "rnd-material-set-sharing-type": 0x4F3870,
    "rnd-material-set-blend-mode": 0x4F3A00,
    "audio-consume-all-timing-reports": 0x262B80,
    "fmod-audio-state-initialize": 0x276F30,
    "fmod-audio-initialize": 0x2773C0,
    "fmod-audio-attach-studio-system": 0x277840,
    "fmod-audio-detach-studio-system": 0x277A80,
    "fmod-audio-consume-timing-report": 0x277BA0,
    "fmod-audio-dispatch-mix-buffers": 0x2781C0,
    "fmod-register-custom-dsp-plugins": 0x278270,
    "fmod-audio-configure-speakers": 0x2783A0,
    "fmod-system-callback": 0x2783E0,
    "fmod-audio-initialize-custom-output": 0x2786D0,
    "audio-timing-accumulator-finish": 0x278880,
    "audio-rolling-timing-accumulator-finish": 0x278910,
    "audio-timing-average-buffer-percent": 0x278C80,
    "audio-timing-maximum-buffer-percent": 0x278CB0,
    "audio-rolling-timing-average-buffer-percent": 0x278D10,
    "audio-rolling-timing-maximum-buffer-percent": 0x278D50,
    "fmod-defer-channel-dsp-release": 0x278A00,
    "fmod-process-deferred-releases": 0x278DF0,
    "fmod-file-handle-open": 0x279FE0,
    "fmod-async-file-reader-initialize": 0x27A1C0,
    "fmod-async-file-reader-thread": 0x27A270,
    "fmod-async-file-reader-shutdown": 0x27A460,
    "fmod-file-seek": 0x27A4D0,
    "fmod-file-read": 0x27A530,
    "fmod-file-open": 0x27A5A0,
    "fmod-file-close": 0x27A6D0,
    "fmod-file-async-read": 0x27A780,
    "fmod-file-async-cancel": 0x27A850,
}

FOCUSED_DISASSEMBLIES = {
    "orbis-render-system-initialize": 0x8D7B20,
}


def write_csv(path: Path, fieldnames: list[str], rows: Iterable[dict[str, Any]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def format_permissions(permissions: int) -> str:
    return "".join(
        (
            "r" if permissions & 4 else "-",
            "w" if permissions & 2 else "-",
            "x" if permissions & 1 else "-",
        )
    )


def collect_analysis(
    db: Database,
    database_path: Path,
    export_dir: Path,
    docs_path: Path,
) -> dict[str, Any]:
    import ida_auto
    import ida_entry
    import ida_funcs
    import ida_hexrays
    import ida_name
    import ida_nalt
    import ida_segment
    import idautils
    import idc

    ida_auto.auto_wait()

    segment_rows: list[dict[str, Any]] = []
    for start in idautils.Segments():
        segment = ida_segment.getseg(start)
        if segment is None:
            continue
        segment_rows.append(
            {
                "name": ida_segment.get_segm_name(segment),
                "class": ida_segment.get_segm_class(segment),
                "start": f"0x{segment.start_ea:X}",
                "end": f"0x{segment.end_ea:X}",
                "size": segment.end_ea - segment.start_ea,
                "permissions": format_permissions(segment.perm),
            }
        )

    function_rows: list[dict[str, Any]] = []
    for start in idautils.Functions():
        function = ida_funcs.get_func(start)
        if function is None:
            continue
        name = ida_name.get_name(start) or f"sub_{start:X}"
        function_rows.append(
            {
                "address": f"0x{start:X}",
                "end": f"0x{function.end_ea:X}",
                "size": function.end_ea - start,
                "name": name,
                "library": bool(function.flags & ida_funcs.FUNC_LIB),
                "thunk": bool(function.flags & ida_funcs.FUNC_THUNK),
            }
        )

    entry_rows: list[dict[str, Any]] = []
    for index in range(ida_entry.get_entry_qty()):
        ordinal = ida_entry.get_entry_ordinal(index)
        address = ida_entry.get_entry(ordinal)
        entry_rows.append(
            {
                "ordinal": ordinal,
                "address": f"0x{address:X}",
                "name": ida_entry.get_entry_name(ordinal) or ida_name.get_name(address),
            }
        )

    strings = list(idautils.Strings())
    source_paths: set[str] = set()
    for item in strings:
        value = str(item)
        source_paths.update(match.group(0) for match in SOURCE_PATH_RE.finditer(value))

    write_csv(
        export_dir / "segments.csv",
        ["name", "class", "start", "end", "size", "permissions"],
        segment_rows,
    )
    write_csv(
        export_dir / "functions.csv",
        ["address", "end", "size", "name", "library", "thunk"],
        function_rows,
    )
    write_csv(
        export_dir / "entries.csv",
        ["ordinal", "address", "name"],
        entry_rows,
    )
    (export_dir / "source_paths.txt").write_text(
        "".join(f"{path}\n" for path in sorted(source_paths, key=str.lower)),
        encoding="utf-8",
    )

    entry_address = entry_rows[0]["address"] if entry_rows else "0x920"
    entry_ea = int(entry_address, 16)
    entry_function = ida_funcs.get_func(entry_ea)
    if entry_function is None:
        ida_funcs.add_func(entry_ea)
        ida_auto.auto_wait()
        entry_function = ida_funcs.get_func(entry_ea)

    disassembly: list[str] = []
    if entry_function is not None:
        for address in idautils.FuncItems(entry_function.start_ea):
            line = idc.generate_disasm_line(address, 0) or ""
            disassembly.append(f"{address:016X}: {line}")
    (export_dir / "entrypoint.asm").write_text(
        "\n".join(disassembly) + ("\n" if disassembly else ""),
        encoding="utf-8",
    )

    decompiler_available = bool(ida_hexrays.init_hexrays_plugin())
    pseudocode = ""
    if decompiler_available and entry_function is not None:
        try:
            pseudocode = str(ida_hexrays.decompile(entry_function.start_ea))
        except Exception as exc:  # IDA raises several version-specific types.
            pseudocode = f"/* Entry-point decompilation failed: {exc} */\n"
    (export_dir / "entrypoint.c").write_text(pseudocode, encoding="utf-8")

    if decompiler_available:
        for output_name, address in FOCUSED_DECOMPILATIONS.items():
            function = ida_funcs.get_func(address)
            if function is None:
                continue
            try:
                focused_pseudocode = str(ida_hexrays.decompile(function.start_ea))
            except Exception as exc:  # IDA exposes version-specific exception types.
                focused_pseudocode = f"/* Decompilation failed: {exc} */\n"
            (export_dir / f"{output_name}.c").write_text(
                focused_pseudocode, encoding="utf-8"
            )

    for output_name, address in FOCUSED_DISASSEMBLIES.items():
        function = ida_funcs.get_func(address)
        if function is None:
            continue
        focused_disassembly = [
            f"{item:016X}: {idc.generate_disasm_line(item, 0) or ''}"
            for item in idautils.FuncItems(function.start_ea)
        ]
        (export_dir / f"{output_name}.asm").write_text(
            "\n".join(focused_disassembly) + "\n", encoding="utf-8"
        )

    named_functions = sum(
        1
        for row in function_rows
        if not str(row["name"]).startswith(("sub_", "loc_", "nullsub_"))
    )
    largest_functions = sorted(
        function_rows, key=lambda row: int(row["size"]), reverse=True
    )[:20]
    summary = {
        "input_path": ida_nalt.get_input_file_path(),
        # Database.path reports the loader input while creating a new IDB.
        # Record the requested persistent output path instead.
        "database_path": str(database_path),
        "image_base": f"0x{ida_nalt.get_imagebase():X}",
        "minimum_address": f"0x{db.minimum_ea:X}",
        "maximum_address": f"0x{db.maximum_ea:X}",
        "segment_count": len(segment_rows),
        "function_count": len(function_rows),
        "named_function_count": named_functions,
        "entry_count": len(entry_rows),
        "entry_address": entry_address,
        "string_count": len(strings),
        "source_path_count": len(source_paths),
        "decompiler_available": decompiler_available,
        "largest_functions": largest_functions,
    }
    (export_dir / "summary.json").write_text(
        json.dumps(summary, indent=2) + "\n", encoding="utf-8"
    )

    largest_lines = "\n".join(
        f"| `{row['address']}` | `{row['name']}` | {row['size']:,} |"
        for row in largest_functions[:10]
    )
    docs_path.parent.mkdir(parents=True, exist_ok=True)
    docs_path.write_text(
        f"""# IDA baseline

This snapshot was generated by `tools/ida_export.py` from the IDA-compatible
copy of the extracted executable.

## Analysis totals

| Metric | Value |
| --- | ---: |
| Image base | `{summary['image_base']}` |
| Address range | `{summary['minimum_address']}` to `{summary['maximum_address']}` |
| Segments | {summary['segment_count']:,} |
| Functions | {summary['function_count']:,} |
| Named functions | {summary['named_function_count']:,} |
| IDA strings | {summary['string_count']:,} |
| Recovered source paths | {summary['source_path_count']:,} |
| Hex-Rays available | {str(summary['decompiler_available']).lower()} |

## Largest discovered functions

| Address | Current name | Size (bytes) |
| --- | --- | ---: |
{largest_lines}

The complete machine-readable exports are under `analysis/exports/`. These are
the baseline for later naming and source reconstruction passes.
""",
        encoding="utf-8",
    )
    return summary


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="IDA-compatible input ELF")
    parser.add_argument(
        "--database",
        type=Path,
        default=Path("analysis/ida/eboot.i64"),
        help="persistent IDA database path",
    )
    parser.add_argument(
        "--exports",
        type=Path,
        default=Path("analysis/exports"),
        help="directory for tracked analysis exports",
    )
    parser.add_argument(
        "--docs",
        type=Path,
        default=Path("docs/ida-baseline.md"),
        help="generated Markdown summary",
    )
    parser.add_argument(
        "--rebuild", action="store_true", help="rebuild the IDA database from the ELF"
    )
    args = parser.parse_args()

    input_path = args.input.resolve()
    database_path = args.database.resolve()
    export_dir = args.exports.resolve()
    docs_path = args.docs.resolve()
    database_path.parent.mkdir(parents=True, exist_ok=True)
    export_dir.mkdir(parents=True, exist_ok=True)

    if database_path.exists() and not args.rebuild:
        open_path = database_path
        options = IdaCommandOptions(auto_analysis=True)
    else:
        open_path = input_path
        options = IdaCommandOptions(
            auto_analysis=True,
            new_database=True,
            output_database=str(database_path),
            log_file=str(database_path.with_suffix(".log")),
        )

    with Database.open(str(open_path), options, save_on_close=True) as database:
        summary = collect_analysis(database, database_path, export_dir, docs_path)

    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
