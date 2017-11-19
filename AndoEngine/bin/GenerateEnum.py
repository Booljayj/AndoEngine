#!/usr/bin/env python

import sys
import os
import yaml

#try to get the element from the list. If it is missing or nil, return the default value
def try_get(list, index, default):
	try:
		if list[index] == None:
			return default
		else:
			return list[index]
	except IndexError:
		return default

#dictionary that returns empty strings for missing keys, meaning missing format arguments will be removed
class FormatDict(dict):
	def __missing__(self, key):
		return ''

include_rel_template = '#include "{}"\n'
include_sys_template = '#include {}\n'
namespace_begin_template = 'namespace {} {{\n'
namespace_end_template = '}\n'

header_template = '''\
// GENERATED FILE: DO NOT EDIT MANUALLY
#pragma once
{includes}\

{namespace_begin}\
class {class_name} {{
public:
	using TYPE = {enum_type};
{conversion_typedefs}\
	enum ENUM : TYPE {{
{enum_values}\
	}};

	static inline constexpr TYPE Count() {{ return _Count; }}
	static inline constexpr bool IsValid( const TYPE Value ) {{ return Value < _Count; }}
	static inline constexpr ENUM Cast( const TYPE Value ) {{ return static_cast<ENUM>( IsValid( Value ) ? Value : _Count ); }}

	static constexpr const char* _NameBlock = {chunked_name_block};

	//Converting to/from a human-readable name
	static const TYPE _NameOffsets[];
	static constexpr const char* ToName( const ENUM Value ) {{
		return _NameBlock + _NameOffsets[static_cast<TYPE>( Value )];
	}}
	static ENUM FromName( const char* Name );
{conversion_declarations}\
private:
	static constexpr TYPE _Count = {enum_values_count};
}};
{namespace_end}\
'''

conversion_typedef_template = '\tusing CONV{index}_TYPE = {type};\n'
conversion_declaration_template = '''
	//Conversion {index}: to/from {name}
	static const CONV{index}_TYPE _Conv{index}Data[];
	static constexpr CONV{index}_TYPE To{name}( const ENUM Value ) {{
		return _Conv{index}Data[static_cast<TYPE>( Value )];
	}}
	static ENUM From{name}( const CONV{index}_TYPE CValue );
'''

source_template = '''\
// GENERATED FILE: DO NOT EDIT MANUALLY
#include "{header_file_name}"

{namespace_begin}\
const {class_name}::TYPE {class_name}::_NameOffsets[] = {{ {chunked_name_offsets} }};

{class_name}::ENUM {class_name}::FromName( const char* Name ) {{
	for( TYPE Index = 0; Index < _Count; ++Index ) {{
		if( strcmp( _NameBlock + _NameOffsets[Index], Name ) == 0 ) {{
			return static_cast<ENUM>( Index );
		}}
	}}
	return static_cast<ENUM>( _Count );
}}
{conversion_definitions}\
{namespace_end}\
'''

conversion_definition_template = '''
const {class_name}::CONV{index}_TYPE {class_name}::_Conv{index}Data[] = {{ {chunked_data} }};

{class_name}::ENUM {class_name}::From{name}( const {class_name}::CONV{index}_TYPE CValue ) {{
	for( TYPE Index = 0; Index < _Count; ++Index ) {{
		if( _Conv{index}Data[Index] == CValue ) {{
			return static_cast<ENUM>( Index );
		}}
	}}
	return static_cast<ENUM>( {default} );
}}
'''

input_file_path = sys.argv[1]
output_directory = sys.argv[2].rstrip( '/' )

file_name = input_file_path.rsplit( '/', 1 )[1]
header_file_path = '{}/{}.h'.format( output_directory, file_name )
source_file_path = '{}/{}.cpp'.format( output_directory, file_name )

os.makedirs( output_directory, exist_ok = True )

input_file = open( input_file_path, 'r' )
raw_data = yaml.load( input_file, Loader = yaml.BaseLoader ) #load all values as strings
input_file.close()

format_args = FormatDict(
	[
		( 'class_name', raw_data['name'] ),
		( 'enum_type', raw_data['type'] ),
		( 'header_file_name', file_name + '.h' ),
	]
)

raw_values = raw_data['values']
format_args['enum_values'] = ''.join( ['\t\t{},\n'.format( value['name'] ) for value in raw_values] )
format_args['enum_values_count'] = str( len( raw_values ) )

value_name_offsets = [0]
value_names = []
for value in raw_values:
	value_names.append( value['name'] + '\\0' )
	value_name_offsets.append( value_name_offsets[-1] + len( value['name'] ) + 1 )
value_names.append( 'INVALID\\0' )

## Names Block
chunk_size = 8
for i in range( 0, len( value_names ), chunk_size ):
	format_args['chunked_name_block'] += '\n\t\t"{}"'.format( ''.join( value_names[i:i+chunk_size] ) )

## Name Offsets
chunk_size = 12
for i in range( 0, len( value_name_offsets ), chunk_size ):
	format_args['chunked_name_offsets'] += '\n\t{},'.format( ', '.join( str(x).rjust(3) for x in value_name_offsets[i:i+chunk_size] ) )

if 'include' in raw_data:
	for include in raw_data['include']:
		if include.startswith( '<' ):
			format_args['includes'] += include_sys_template.format( include )
		else:
			format_args['includes'] += include_rel_template.format( include )
	#format_args['includes'] += '\n'

if 'namespace' in raw_data:
	format_args['namespace_begin'] = namespace_begin_template.format( raw_data['namespace'] )
	format_args['namespace_end'] = namespace_end_template

if 'conv' in raw_data:
	for index, conversion in enumerate( raw_data['conv'] ):
		conversion_format_args = FormatDict()
		conversion_format_args['class_name'] = format_args['class_name']
		conversion_format_args['index'] = index
		conversion_format_args['name'] = conversion['name']
		conversion_format_args['type'] = conversion['type']
		conversion_format_args['default'] = conversion['default']

		default = conversion['default']
		conversion_data_values = [try_get(value['conv'], index, default) for value in raw_values]

		chunk_size = 8
		for i in range( 0, len( conversion_data_values ), chunk_size ):
			conversion_format_args['chunked_data'] += '\n\t{},'.format( ', '.join( conversion_data_values[i:i+chunk_size] ) )

		format_args['conversion_typedefs'] += conversion_typedef_template.format_map( conversion_format_args )
		format_args['conversion_declarations'] += conversion_declaration_template.format_map( conversion_format_args )
		format_args['conversion_definitions'] += conversion_definition_template.format_map( conversion_format_args )

	format_args['conversion_typedefs'] += '\n'
	format_args['conversion_declarations'] += '\n'
	format_args['conversion_definitions'] += '\n'

header_file = open( header_file_path, 'w' )
header_file.write( header_template.format_map( format_args ) )
header_file.close()

source_file = open( source_file_path, 'w' )
source_file.write( source_template.format_map( format_args ) )
source_file.close()
