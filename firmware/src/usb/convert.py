header_template = "content.template"
header_out = "content.h"
readme_file = "readme.pdf"

array = "{"
index = 0

try:
    with open(readme_file, "rb") as f:
        byte = f.read(1)
        while byte:
            index += 1
            # Do stuff with byte.
            array = array + '0X' + str(byte.hex()) + ', '
            if index % 512 == 0:
                array = array + '}, \\\n{'
            if index % 20 == 0:
                array = array + '\\\n'
            byte = f.read(1)

except IOError:
    print('Error While Opening the file!')

array = array[0: -2] + " }"

# Read in the header file
with open(header_template, 'r') as file:
    filedata = file.read()

# Replace the target string
filedata = filedata.replace('$FS_CONTENT', array)
filedata = filedata.replace('$FS_SIZE', str(index))
filedata = filedata.replace('$FS_NAMES', '{"README  ", "PDF"}')
filedata = filedata.replace('$FS_LABEL', '"SHUTUP&DIE "')

with open(header_out, 'w') as file:
    file.write(filedata)
