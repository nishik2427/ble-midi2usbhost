#ifndef _M5_UNIT_SYNTH_H
#define _M5_UNIT_SYNTH_H

#define UNIT_SYNTH_BAUD 31250

#define MIDI_CMD_NOTE_OFF 0x80  // Note Off
#define MIDI_CMD_NOTE_ON  0x90  // Note On
#define MIDI_CMD_POLYPHONIC_AFTERTOUCH \
    0xA0  // Polyphonic Aftertouch (or Key Pressure)
#define MIDI_CMD_CONTROL_CHANGE \
    0xB0                              // Control Change (or Channel Mode
                                      // Message)
#define MIDI_CMD_PROGRAM_CHANGE 0xC0  // Program Change
#define MIDI_CMD_CHANNEL_AFTERTOUCH \
    0xD0  // Channel Aftertouch (or Channel Pressure)
#define MIDI_CMD_PITCH_BEND       0xE0  // Pitch Bend
#define MIDI_CMD_SYSTEM_EXCLUSIVE 0xF0  // System Exclusive (SysEx) Start
#define MIDI_CMD_TIME_CODE        0xF1  // MIDI Time Code Quarter Frame
#define MIDI_CMD_SONG_POSITION    0xF2  // Song Position Pointer
#define MIDI_CMD_SONG_SELECT      0xF3  // Song Select
#define MIDI_CMD_TUNE_REQUEST     0xF6  // Tune Request
#define MIDI_CMD_END_OF_SYSEX     0xF7  // End of SysEx
#define MIDI_CMD_TIMING_CLOCK \
    0xF8  // Timing Clock (used in System Real-Time Messages)
#define MIDI_CMD_START    0xFA  // Start (used in System Real-Time Messages)
#define MIDI_CMD_CONTINUE 0xFB  // Continue (used in System Real-Time Messages)
#define MIDI_CMD_STOP     0xFC  // Stop (used in System Real-Time Messages)
#define MIDI_CMD_ACTIVE_SENSING \
    0xFE  // Active Sensing (used in System Real-Time Messages)
#define MIDI_CMD_SYSTEM_RESET 0xFF  // System Reset

typedef enum {
    GrandPiano_1 = 1,
    BrightPiano_2,
    ElGrdPiano_3,
    HonkyTonkPiano,
    ElPiano1,
    ElPiano2,
    Harpsichord,
    Clavi,
    Celesta,
    Glockenspiel,
    MusicBox,
    Vibraphone,
    Marimba,
    Xylophone,
    TubularBells,
    Santur,
    DrawbarOrgan,
    PercussiveOrgan,
    RockOrgan,
    ChurchOrgan,
    ReedOrgan,
    AccordionFrench,
    Harmonica,
    TangoAccordion,
    AcGuitarNylon,
    AcGuitarSteel,
    AcGuitarJazz,
    AcGuitarClean,
    AcGuitarMuted,
    OverdrivenGuitar,
    DistortionGuitar,
    GuitarHarmonics,
    AcousticBass,
    FingerBass,
    PickedBass,
    FretlessBass,
    SlapBass1,
    SlapBass2,
    SynthBass1,
    SynthBass2,
    Violin,
    Viola,
    Cello,
    Contrabass,
    TremoloStrings,
    PizzicatoStrings,
    OrchestralHarp,
    Timpani,
    StringEnsemble1,
    StringEnsemble2,
    SynthStrings1,
    SynthStrings2,
    ChoirAahs,
    VoiceOohs,
    SynthVoice,
    OrchestraHit,
    Trumpet,
    Trombone,
    Tuba,
    MutedTrumpet,
    FrenchHorn,
    BrassSection,
    SynthBrass1,
    SynthBrass2,
    SopranoSax,
    AltoSax,
    TenorSax,
    BaritoneSax,
    Oboe,
    EnglishHorn,
    Bassoon,
    Clarinet,
    Piccolo,
    Flute,
    Recorder,
    PanFlute,
    BlownBottle,
    Shakuhachi,
    Whistle,
    Ocarina,
    Lead1Square,
    Lead2Sawtooth,
    Lead3Calliope,
    Lead4Chiff,
    Lead5Charang,
    Lead6Voice,
    Lead7Fifths,
    Lead8BassLead,
    Pad1Fantasia,
    Pad2Warm,
    Pad3PolySynth,
    Pad4Choir,
    Pad5Bowed,
    Pad6Metallic,
    Pad7Halo,
    Pad8Sweep,
    FX1Rain,
    FX2Soundtrack,
    FX3Crystal,
    FX4Atmosphere,
    FX5Brightness,
    FX6Goblins,
    FX7Echoes,
    FX8SciFi,
    Sitar,
    Banjo,
    Shamisen,
    Koto,
    Kalimba,
    BagPipe,
    Fiddle,
    Shanai,
    TinkleBell,
    Agogo,
    SteelDrums,
    Woodblock,
    TaikoDrum,
    MelodicTom,
    SynthDrum,
    ReverseCymbal,
    GtFretNoise,
    BreathNoise,
    Seashore,
    BirdTweet,
    TelephRing,
    Helicopter,
    Applause,
    Gunshot,
} unit_synth_instrument_t;

#endif



