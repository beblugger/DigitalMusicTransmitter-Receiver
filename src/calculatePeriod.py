basefreq = 30000

def midi_note_to_frequency(midi_note):
    """Convert MIDI note number to frequency in Hz."""
    return 2 ** ((midi_note - 69) / 12) * 440

def midi_note_to_period(midi_note):
    """Convert MIDI note number to period in seconds."""
    frequency = midi_note_to_frequency(midi_note)
    return 1 / frequency

def generate_midi_frequencies_and_periods():
    """Generate and write frequencies and periods for MIDI notes 0 to 127 to pitchperiod.h."""
    with open("pitchPeriod.h", "w") as file:
        file.write("#include<stdint.h>\n#define BASEFREQ "+f"{int(basefreq)}"+"\nconst uint16_t pitchPeriod[]={")
        for midi_note in range(128):
            frequency = midi_note_to_frequency(midi_note)
            period = midi_note_to_period(midi_note)
            file.write(f"{int(basefreq/frequency)}")
            if midi_note < 127:
                file.write(",")
                if midi_note % 16 == 15:
                    file.write("\n")
        file.write("};")

if __name__ == "__main__":
    generate_midi_frequencies_and_periods()
