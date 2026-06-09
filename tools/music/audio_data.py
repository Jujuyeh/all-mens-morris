"""Shared TableTop Studio audio data helpers."""

from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any


REST_NOTE = 0
EDITED_MENU_PATH = Path("tools/music/menu-music.json")
SOUND_EFFECTS_PATH = Path("tools/music/sound-effects.json")


def midi_frequency(note: int) -> int:
    return round(440.0 * (2.0 ** ((note - 69) / 12.0)))


def format_values(values: list[int], indent: str = "  ") -> str:
    lines = []
    for start in range(0, len(values), 16):
        chunk = values[start:start + 16]
        lines.append(indent + ", ".join(str(value) for value in chunk) + ",")
    return "\n".join(lines)


def generated_header(count: int, min_note: int, max_note: int, tick_ms: int) -> str:
    return f"""#pragma once

#include <Arduino.h>

constexpr uint8_t MENU_MUSIC_EVENT_COUNT = {count};
constexpr uint8_t MENU_MUSIC_MIN_NOTE = {min_note};
constexpr uint8_t MENU_MUSIC_MAX_NOTE = {max_note};
constexpr uint8_t MENU_MUSIC_TICK_MS = {tick_ms};

extern const uint8_t MenuMusicNotes[] PROGMEM;
extern const uint8_t MenuMusicDurations[] PROGMEM;
extern const uint16_t MenuMusicFrequencies[] PROGMEM;
"""


def generated_source(events: list[dict[str, int]], min_note: int, max_note: int) -> str:
    notes = [event["note"] for event in events]
    durations = [event["duration"] for event in events]
    frequencies = [midi_frequency(note) for note in range(min_note, max_note + 1)]
    return f"""#include "MenuMusic.h"

// Generated from TableTop Studio menu music data.
// Original source: Mutopia's public-domain MIDI for Scott Joplin's The Entertainer.

const uint8_t MenuMusicNotes[] PROGMEM = {{
{format_values(notes)}
}};

const uint8_t MenuMusicDurations[] PROGMEM = {{
{format_values(durations)}
}};

const uint16_t MenuMusicFrequencies[] PROGMEM = {{
{format_values(frequencies)}
}};
"""


def validate_events(events: Any) -> list[dict[str, int]]:
    if not isinstance(events, list) or not events:
        raise ValueError("Audio needs at least one event")
    clean = []
    for index, event in enumerate(events):
        if not isinstance(event, dict):
            raise ValueError(f"Audio event {index} must be an object")
        note = int(event.get("note", REST_NOTE))
        duration = int(event.get("duration", 1))
        if note < 0 or note > 127:
            raise ValueError(f"Audio event {index} note is outside MIDI range")
        if duration < 1 or duration > 255:
            raise ValueError(f"Audio event {index} duration must fit uint8_t")
        clean.append({"note": note, "duration": duration})
    return clean


def parse_int_array(source: str, name: str) -> list[int]:
    match = re.search(
        rf"const\s+uint(?:8|16)_t\s+{name}\[\]\s+PROGMEM\s*=\s*\{{(.*?)\}};",
        source,
        flags=re.DOTALL,
    )
    if not match:
        raise ValueError(f"Could not find {name}")
    return [int(token) for token in re.findall(r"\d+", match.group(1))]


def load_menu_music(root: Path) -> dict[str, Any]:
    edited_path = root / EDITED_MENU_PATH
    if edited_path.exists():
        payload = json.loads(edited_path.read_text(encoding="utf-8"))
        payload["events"] = validate_events(payload.get("events"))
        payload["path"] = str(EDITED_MENU_PATH)
        return payload

    header = (root / "src/MenuMusic.h").read_text(encoding="utf-8")
    source = (root / "src/MenuMusic.cpp").read_text(encoding="utf-8")
    tick_ms = int(re.search(r"MENU_MUSIC_TICK_MS = (\d+)", header).group(1))
    notes = parse_int_array(source, "MenuMusicNotes")
    durations = parse_int_array(source, "MenuMusicDurations")
    events = [{"note": note, "duration": duration} for note, duration in zip(notes, durations)]
    return {
        "id": "menu-music",
        "name": "Menu Music",
        "kind": "music",
        "tickMs": tick_ms,
        "events": validate_events(events),
        "path": "src/MenuMusic.*",
    }


def save_menu_music(root: Path, payload: dict[str, Any]) -> dict[str, Any]:
    events = validate_events(payload.get("events"))
    tick_ms = int(payload.get("tickMs", 69))
    if tick_ms < 20 or tick_ms > 250:
        raise ValueError("tickMs must be between 20 and 250")
    used_notes = [event["note"] for event in events if event["note"] != REST_NOTE]
    if not used_notes:
        raise ValueError("Menu music needs at least one note")
    min_note = min(used_notes)
    max_note = max(used_notes)

    edited = {
        "id": "menu-music",
        "name": "Menu Music",
        "kind": "music",
        "tickMs": tick_ms,
        "events": events,
    }
    edited_path = root / EDITED_MENU_PATH
    edited_path.parent.mkdir(parents=True, exist_ok=True)
    edited_path.write_text(json.dumps(edited, indent=2) + "\n", encoding="utf-8")
    (root / "src/MenuMusic.h").write_text(
        generated_header(len(events), min_note, max_note, tick_ms),
        encoding="utf-8",
    )
    (root / "src/MenuMusic.cpp").write_text(
        generated_source(events, min_note, max_note),
        encoding="utf-8",
    )
    return edited | {"path": str(EDITED_MENU_PATH)}


def load_effects(root: Path) -> list[dict[str, Any]]:
    path = root / SOUND_EFFECTS_PATH
    if not path.exists():
        return [
            {
                "id": "new-effect",
                "name": "New Effect",
                "kind": "effect",
                "tickMs": 50,
                "events": [{"note": 72, "duration": 2}],
            }
        ]
    payload = json.loads(path.read_text(encoding="utf-8"))
    effects = payload.get("effects", [])
    for effect in effects:
        effect["events"] = validate_events(effect.get("events"))
    return effects


def save_effect(root: Path, payload: dict[str, Any]) -> dict[str, Any]:
    effect_id = re.sub(r"[^a-z0-9_-]+", "-", str(payload.get("id") or payload.get("name") or "effect").lower()).strip("-")
    effect = {
        "id": effect_id or "effect",
        "name": str(payload.get("name") or "Effect"),
        "kind": "effect",
        "tickMs": int(payload.get("tickMs", 50)),
        "events": validate_events(payload.get("events")),
    }
    effects = [item for item in load_effects(root) if item.get("id") != effect["id"]]
    effects.append(effect)
    effects.sort(key=lambda item: item.get("id", ""))
    path = root / SOUND_EFFECTS_PATH
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps({"effects": effects}, indent=2) + "\n", encoding="utf-8")
    return effect | {"path": str(SOUND_EFFECTS_PATH)}
