import os

disk_image = "fat_stripped.img"
header_template = "content.template"
header_out = "content.h"

os.system('mkdosfs -F 16 -f 1 -n "ShutUp&Die" -r 16 -s 1 -S 512 -C fat.img 4201')
os.system('mcopy -i fat.img readme.pdf ::/')
os.system('sed \'$ s/\\x00*$//\' fat.img > fat_stripped.img')
os.system('rm fat.img')

array = "{"
index = 0

try:
    with open(disk_image, "rb") as f:
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

array = array[0: -2] + " }\\"

# Read in the header file
with open(header_template, 'r') as file:
    filedata = file.read()

# Replace the target string
filedata = filedata.replace('$FS_CONTENT', array)

with open(header_out, 'w') as file:
    file.write(filedata)

os.system('rm fat_stripped.img')
