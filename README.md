# InstaDrums

Free, open-source VST3 drum sampler plugin built with JUCE.

![VST3](https://img.shields.io/badge/format-VST3-blue) ![C++](https://img.shields.io/badge/language-C%2B%2B17-orange) ![JUCE](https://img.shields.io/badge/framework-JUCE-green) ![License](https://img.shields.io/badge/license-GPL--3.0-lightgrey)

## Download

**[Latest Release: v1.1](https://github.com/hariel1985/InstaDrums/releases/tag/v1.1)**

### Windows
| File | Description |
|------|-------------|
| [InstaDrums-v1.1-VST3-Win64.zip](https://github.com/hariel1985/InstaDrums/releases/download/v1.1/InstaDrums-v1.1-VST3-Win64.zip) | VST3 plugin — copy to `C:\Program Files\Common Files\VST3\` |
| [InstaDrums-v1.1-Standalone-Win64.zip](https://github.com/hariel1985/InstaDrums/releases/download/v1.1/InstaDrums-v1.1-Standalone-Win64.zip) | Standalone application |

### macOS (Universal Binary: Apple Silicon + Intel)
| File | Description |
|------|-------------|
| [InstaDrums-VST3-macOS.zip](https://github.com/hariel1985/InstaDrums/releases/download/v1.1/InstaDrums-VST3-macOS.zip) | VST3 plugin — copy to `~/Library/Audio/Plug-Ins/VST3/` |
| [InstaDrums-AU-macOS.zip](https://github.com/hariel1985/InstaDrums/releases/download/v1.1/InstaDrums-AU-macOS.zip) | Audio Unit — copy to `~/Library/Audio/Plug-Ins/Components/` |
| [InstaDrums-Standalone-macOS.zip](https://github.com/hariel1985/InstaDrums/releases/download/v1.1/InstaDrums-Standalone-macOS.zip) | Standalone application |

### Linux (x64, built on Ubuntu 22.04)
| File | Description |
|------|-------------|
| [InstaDrums-VST3-Linux-x64.zip](https://github.com/hariel1985/InstaDrums/releases/download/v1.1/InstaDrums-VST3-Linux-x64.zip) | VST3 plugin — copy to `~/.vst3/` |
| [InstaDrums-Standalone-Linux-x64.zip](https://github.com/hariel1985/InstaDrums/releases/download/v1.1/InstaDrums-Standalone-Linux-x64.zip) | Standalone application |

> **macOS note:** Builds are Universal Binary (Apple Silicon + Intel). Not code-signed — after copying the plugin, remove the quarantine flag in Terminal:
> ```bash
> xattr -cr ~/Library/Audio/Plug-Ins/VST3/InstaDrums.vst3
> xattr -cr ~/Library/Audio/Plug-Ins/Components/InstaDrums.component
> ```

## Features

### Sampler
- 12 pads (4x3 grid), expandable by 4
- Velocity layers with round-robin (auto-detected from filenames: Ghost/PP/P/MP/F/FF)
- Drag & drop sample loading (single files or folders)
- WAV, AIFF, FLAC, OGG, MP3 support
- Kit presets (save/load .drumkit files)
- Smart folder loading with name matching (kick, snare, hihat, etc.)
- General MIDI drum mapping with configurable MIDI notes
- Choke groups, one-shot / polyphonic mode per pad

### Per-Pad Controls
- Volume, Pan, Pitch (+/- 24 semitones)
- ADSR envelope (Attack, Decay, Sustain, Release)
- Low-pass filter (Cutoff + Resonance)

### Per-Pad FX
- Compressor (Threshold, Ratio) with GR meter
- 3-band EQ (Lo / Mid / Hi, +/- 12dB)
- Distortion (Drive, Mix) - tanh waveshaper
- Reverb (Size, Decay)
- Each FX toggleable with animated switches

### Multi-Output Routing
- 7 stereo output buses: Main, Kick, Snare, HiHat, Toms, Cymbals, Perc
- Each pad pre-assigned to its bus (configurable)
- In REAPER: enable additional outputs via routing for separate track processing
- Pads with inactive buses automatically fall back to Main

### Master Bus
- Master Volume, Tune, Pan
- Output Limiter (0dB brickwall, toggleable)
- Peak VU meter with hold indicator

### GUI
- Dark modern UI inspired by hardware drum machines
- 3D metal knobs with glow effects (orange for sample controls, blue for FX)
- Waveform thumbnails on pads + large waveform in sample editor
- ADSR curve overlay on waveform
- Carbon fiber background texture
- Rajdhani custom font
- Fully resizable window with proportional scaling
- Animated toggle switches

## Building

### Requirements
- Windows 10/11
- Visual Studio 2022 Build Tools (C++ workload)
- CMake 3.22+
- JUCE framework (cloned to `../JUCE` relative to project)

### Build Steps

```bash
git clone https://github.com/juce-framework/JUCE.git ../JUCE
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Output:
- VST3: `build/InstaDrums_artefacts/Release/VST3/InstaDrums.vst3`
- Standalone: `build/InstaDrums_artefacts/Release/Standalone/InstaDrums.exe`

### Install

Copy the VST3 to the standard location:
```
xcopy /E /I /Y "build\InstaDrums_artefacts\Release\VST3\InstaDrums.vst3" "C:\Program Files\Common Files\VST3\InstaDrums.vst3"
```

## Sample Packs

The plugin ships without samples. Recommended free sample packs:
- [Salamander Drumkit](https://github.com/studiorack/salamander-drumkit) (CC0, acoustic, multi-velocity)
- [Free Wave Samples](https://freewavesamples.com/sample-type/drums) (royalty-free)
- [Samples From Mars](https://samplesfrommars.com/collections/free) (free drum machine kits)

## Tech Stack

- **Language:** C++17
- **Framework:** JUCE 8
- **Build:** CMake + MSVC 2022
- **Audio DSP:** juce::dsp (IIR filters, Compressor, Reverb)
- **Font:** Rajdhani (SIL Open Font License)
