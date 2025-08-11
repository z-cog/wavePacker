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

- **LEFT**
- **DOWN**
- **UP**
- **RIGHT**

These labels are written as cue points into a corresponding `.wav` audio file, making it compatible with cue-driven systems like Unreal Engine's MetaSound system.

The mapping is currently based on the MIDI note number (0–127) % 4.

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
A new file is created in `build/`: 
`A_filenameWithCues.wav`
This file contains embedded cue points with labels.

---

## DIRECTORY STRUCTURE

Expected folder layout:
```
wavePacker/
├── .vscode/                   # Not strictly necessary, but helpful for VSCode
│   ├── launch.json
│   ├── settings.json
│   └── tasks.json
├── build/
│   ├── mySong.mid              # MIDI file
│   ├── mySong.wav              # WAV file
│   ├── A_mySongWithCues.wav    # Output (after running wavePacker)
│   └── wavePacker.exe          # Compiled executable
├── include/
│   ├── midifile/               # Header files from @craigsapp/midifile
│   │   ├── Binasc.h
│   │   ├── MidiEvent.h
│   │   ├── MidiEventList.h
│   │   ├── MidiFile.h
│   │   ├── MidiMessage.h
│   │   └── Options.h
│   └── wave/
│       └── chunk.h
├── lib/
│   └── libmidifile.a           # Static library built from @craigsapp/midifile
└── src/
    └── main.cpp

```

---

## EXAMPLES

Run with default settings:

```bash
wavePacker mySong
```

Run with debug mode:

```bash
wavePacker -d mySong
```

---

## BUILDING

*Note: Full build instructions will be added soon. For now, this was compiled using `g++` in Visual Studio Code with custom settings.*

### Dependencies

- [midifile](https://github.com/craigsapp/midifile) (static library version).

### Steps
* 1. Clone or download [midifile](https://github.com/craigsapp/midifile)
* 2. Compile it as a static library (`libmidifile.a`)
* 3. Place it in a directory called `lib/`

---

## KNOWN ISSUES

- Only processes Note-On events.
- Ignores velocity and Note-Offs.
- `.mid` and `.wav` files must have the same basename.
- Expects both input files to be in the `build/` directory.

---

## SEE ALSO

- [midifile](https://github.com/craigsapp/midifile)
- [Unreal Engine Audio Cues](https://docs.unrealengine.com)

---

## AUTHOR

Created by **Zachary Colgrove**  
For the **BYU 2026 Capstone Game – *DragonKisser***  
GitHub: **[@z-cog](https://github.com/z-cog)**  

---

## LICENSE

I need to ask my professor, but you're probably fine to use this as needed.
