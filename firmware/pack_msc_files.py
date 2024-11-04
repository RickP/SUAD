import zipfile
import glob
import os

label = "ShutUp&Die"

included_dir = "usb_files"
target_dir = "src/usb"
header_template = target_dir + "/content.template"
header_out = target_dir + "/content.h"

PACK_SOURCES = True
source_archive_file = included_dir + "/source.zip"

base_path = os.path.dirname(__file__)
os.chdir(base_path)

if PACK_SOURCES:
    print("Packing source files")
    if os.path.exists(source_archive_file):
        os.remove(source_archive_file)

    with zipfile.ZipFile(included_dir + "/source.zip", "w") as sourcezip:

        sourcezip.write('CMakeLists.txt')
        sourcezip.write('pico_sdk_import.cmake')
        sourcezip.write('README.md')
        sourcezip.write('pack_msc_files.py')
        sourcezip.mkdir("usb_files")
        folder_path = "src"
        len_dir_path = len(folder_path)
        for root, _, files in os.walk(folder_path):
            for file in files:
                if file != "content.h" and file != ".DS_Store":
                    file_path = os.path.join(root, file)
                    sourcezip.write(file_path)

files_to_include = glob.glob(included_dir + '/*')

str_components = []
file_size_string = ""
file_name_string = ""


def format_filename(filename):
    parts = os.path.basename(filename).split(".")
    return '{"' + str(parts[0][0:8]).ljust(8).upper() + '", "' + str(parts[1][0:3]).ljust(3).upper()+ '"}, '


if "usb_files/manual.pdf" not in files_to_include and "usb_files/Manual.pdf" not in files_to_include:
    exit("* Please put the manual.pdf in usb_files! *")

print("Reading in USB files")
for file_to_include in files_to_include:
    if os.path.isfile(file_to_include):
        index = 0
        try:
            print("Including file %s" % file_to_include)
            with open(file_to_include, "rb") as f:
                file_name_string += format_filename(file_to_include)
                str_components.append("{")
                byte = f.read(1)
                while byte:
                    index += 1
                    # Do stuff with byte.
                    str_components.append('0X' + str(byte.hex()) + ', ')
                    if index % 512 == 0:
                        str_components.append('}, \\\n{')
                    if index % 20 == 0:
                        str_components.append('\\\n')
                    byte = f.read(1)

        except IOError:
            print('Error While Opening the file %s!' % file_to_include)

        str_components.append(" }, \\\n")
        data_array_string = "".join(str_components)
        file_size_string += str(index) + ', '

# Read in the header file
print("Creating header file")
with open(header_template, 'r') as file:
    filedata = file.read()

# Replace the target string
filedata = filedata.replace('$FS_CONTENT', data_array_string)
filedata = filedata.replace('$FS_SIZE', file_size_string)
filedata = filedata.replace('$FS_NAMES', file_name_string)
filedata = filedata.replace('$FS_LABEL', '"' + label[0:11].ljust(11) + '"')

with open(header_out, 'w') as file:
    file.write(filedata)
