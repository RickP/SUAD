pdf_file = "content.txt"
header_template = "content.template"
header_out = "content.h"


array = " \\\n{"
index = 0

try:
    with open(pdf_file, "rb") as f:
        byte = f.read(1)
        while byte:
            index += 1
            # Do stuff with byte.
            array = array + '0x' + str(byte.hex()) + ', '
            if index % 20 == 0:
                array = array + '\\\n'
            if index % 512 == 0:
                array = array + '}, {'
            byte = f.read(1)

except IOError:
    print('Error While Opening the file!')

array = array[0: -2] + "}"

# Read in the header file
with open(header_template, 'r') as file:
    filedata = file.read()

# Replace the target string
filedata = filedata.replace('PDF_FILE', array)
filedata = filedata.replace('PDF_SIZE', str(index))

with open(header_out, 'w') as file:
    file.write(filedata)
