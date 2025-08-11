# `wavePacker` – MIDI to WAV Cue Point Tool

## NAME
**wavePacker** – Parse MIDI note-on events and insert labeled cue points into a WAVE file for use in Unreal Engine.

---

## SYNOPSIS

```bash
wavePacker [--debug|-d] filename
```

---

## DESCRIPTION

**wavePacker** processes a `.mid` MIDI file and extracts `Note On` events, mapping each note to one of four directional cue labels:

- **UP**
- **DOWN**
- **LEFT**
- **RIGHT**

These labels are written as cue points into a corresponding `.wav` audio file, making it compatible with cue-driven systems like Unreal Engine's MetaSound system.

The mapping is based on the MIDI note number (0–127) modulo 4.

For reference, middle C is marked as note 60 in the MIDI format.

---

## OPTIONS

| Option       | Description                                 |
|--------------|---------------------------------------------|
| `--debug`, `-d` | Enables debug mode: prints verbose output and writes cue times/labels to a `.txt` file. |
| `--help`, `-h`  | Displays help information. |

---

## FILES

You must have the following files in the same directory:

- `filename.mid` – the MIDI file to parse
- `filename.wav` – the source audio file

**Output:**  
A new file named `A_filenameWithCues.wav` is created with embedded cue points and label metadata.

---

## EXAMPLES

Run with default settings:

```bash
wavePacker mySong
```

Run with debug mode and custom note ranges:

```bash
wavePacker -d mySong 20 60 100
```

---

## DEPENDENCIES

- [midifile](https://github.com/craigsapp/midifile) – for parsing MIDI files

---

## AUTHOR

Created by **Zachary Colgrove**  
For the **BYU 2026 Capstone Game – *DragonKisser***

---

## KNOWN ISSUES

- Only processes Note-On events; Note-Off and velocity nuances ignored.
- Expects `.mid` and `.wav` files to have the same basename.

---

## SEE ALSO

- [midifile](https://github.com/craigsapp/midifile)
- [Unreal Engine Audio Cues](https://docs.unrealengine.com)

---

## LICENSE

Probably creative commons idk.

