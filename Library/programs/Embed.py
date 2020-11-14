import sys
import os

header_template = '''\
// GENERATED FILE: DO NOT EDIT MANUALLY
#pragma once
#include <array>

namespace Embedded {{
	extern std::array<uint8_t, {size}> const {name};
}}
'''

source_template = '''\
// GENERATED FILE: DO NOT EDIT MANUALLY
#include "{header_file_name}"

namespace Embedded {{
	std::array<uint8_t, {size}> const {name} = {{
{bytes_string}
	}};
}}
'''

input_file_path = sys.argv[1].lstrip('/')
output_directory = sys.argv[2].lstrip('/').rstrip('/')

file_name = input_file_path.rsplit('/', 1)[1]
header_file_path = '{}/{}.h'.format(output_directory, file_name)
source_file_path = '{}/{}.cpp'.format(output_directory, file_name)

variable_name = file_name.replace('.', '_')

with open(input_file_path, 'rb') as input_file:
	input_data = input_file.read()
	input_data_size:int = len(input_data)

	bytes_string = ''
	for group_index in range(0, int((input_data_size/16)+1)):
		line_string = ''
		for item_index in range(group_index*16, (group_index+1)*16):
			if item_index >= input_data_size:
				break
			line_string += '0x{0:0{1}X},'.format(input_data[item_index],2)
		if len(line_string) == 0:
			break
		bytes_string += (line_string + '\n')

	format_arguments = {
		'size': input_data_size,
		'name': variable_name,
		'header_file_name': file_name + '.h',
		'bytes_string': bytes_string
	}

	header_file = open(header_file_path, 'w')
	header_file.write(header_template.format_map(format_arguments))
	header_file.close()

	source_file = open(source_file_path, 'w')
	source_file.write(source_template.format_map(format_arguments))
	source_file.close()
