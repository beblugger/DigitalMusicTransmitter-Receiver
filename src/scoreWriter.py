data = [
    (76 ,1,0),
(81 ,1,1200),
(84 ,1,1200),
(83 ,1,600),
(81 ,1,300),
(88 ,1,300),
(84 ,1,600),
(81 ,1,600),
(81 ,1,2400),
(76 ,1,2400),
(88 ,1,1200),
(86 ,1,1200),
(88 ,1,600),
(86 ,1,300),
(84 ,1,300),
(83 ,1,600),
(81 ,1,600),
(83 ,1,900),
(81 ,1,300),
(79 ,1,300),
(76 ,1,300),
(74 ,1,300),
(72 ,1,300),
(72 ,1,300),
(74 ,1,600),
(76 ,1,300),
(74 ,1,1200),
(76 ,1,900),
(71 ,1,300),
(70 ,1,300),
(67 ,1,300),
(69 ,1,600),
(69 ,1,2400),
(0 ,1,1800),
(76 ,1,300),
(79 ,1,300),
(76 ,1,900),
(76 ,1,300),
(0 ,1,600),
(76 ,1,300),
(84 ,1,300),
(81 ,1,600),
(81 ,1,300),
(81 ,1,600),
(84 ,1,300),
(81 ,1,300),
(79 ,1,600),
(72 ,1,600),
(74 ,1,600),
(72 ,1,300),
(74 ,1,150),
(76 ,1,150),
(0 ,1,1800),
(76 ,1,300),
(84 ,1,300),
(81 ,1,900),
(81 ,1,300),
(81 ,1,900),
(84 ,1,300),
(86 ,1,600),
(88 ,1,600),
(88 ,1,1200),
(88 ,1,600),
(86 ,1,300),
(83 ,1,300),
(79 ,1,900),
(81 ,1,300),
(69 ,1,2400),
(76 ,1,900),
(74 ,1,300),
(76 ,1,300),
(72 ,1,300),
(71 ,1,300),
(69 ,1,300),
(76 ,1,2400),
(81 ,1,300),
(79 ,1,600),
(76 ,1,300),
(74 ,1,300),
(72 ,1,300),
(74 ,1,300),
(76 ,1,300),
(76 ,1,1800),
(79 ,1,300),
(81 ,1,300),
(83 ,1,900),
(81 ,1,300),
(79 ,1,300),
(76 ,1,300),
(74 ,1,300),
(72 ,1,300),
(72 ,1,300),
(74 ,1,600),
(76 ,1,300),
(74 ,1,1200),
(76 ,1,900),
(71 ,1,300),
(69 ,1,300),
(67 ,1,300),
(69 ,1,600),
(0 ,1,1800),
(76 ,1,300),
(74 ,1,300),
(76 ,1,900),
(83 ,1,300),
(81 ,1,300),
(79 ,1,300),
(81 ,1,600),
(81 ,1,2400),
(81 ,0,2400)
]

struct_name = "noteCmd"
array_name = "noteArray"
header_file = "note_library.h"
source_file = "note_library.c"

def generate_c_code(data, struct_name, array_name, header_file, source_file):
    # Generate header file content
    header_content = f"""\
#ifndef NOTE_LIBRARY_H
#define NOTE_LIBRARY_H

#include <stdint.h>

typedef struct
{{
    uint8_t pitch;     /**< The pitch of the note. */
    uint8_t intensity; /**< The intensity of the note. */
    uint16_t delay;    /**< The delay before playing the note. Unit is microsecond */
}} {struct_name};

extern const {struct_name} {array_name}[];

#endif // NOTE_LIBRARY_H
"""

    # Generate source file content
    source_content = f"""\
#include "{header_file}"

const {struct_name} {array_name}[] = {{
"""

    for pitch, intensity, delay in data:
        source_content += f"    {{{pitch}, {intensity}, {delay}}},\n"

    source_content += "};\n"

    # Write header file
    with open(header_file, 'w') as header:
        header.write(header_content)

    # Write source file
    with open(source_file, 'w') as source:
        source.write(source_content)

# Generate the C code
generate_c_code(data, struct_name, array_name, header_file, source_file)
