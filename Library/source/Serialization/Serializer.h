#pragma once
#include <ostream>
#include <istream>
#include <sstream>

/**
 * Binary Serialization is based around the idea of "data blocks". A data block consists of
 * a chunk of serialized data preceeded by an integer that denotes how large it is. This
 * chunk of serialized data may also contain other data blocks in a nested structure. Often,
 * each of the nested data blocks is preceeded by a fixed-size identifier number that will
 * indicate how the nested data block relates to the data block inside of which it is nested.
 * It may also contain a number indicating how many nested data blocks exist, then an array
 * of each of these blocks.
 *
 * The Serialize method is responsible for writing a data block from the indicated object
 * to the Stream, while the Deserialize method is responsible for reading a data block and
 * extracting the information from it into the object.
 *
 *	struct DataBlock {
 *		uint32_t DataSize;
 *		union {
 *			char* Data;
 *			tuple<uint16_t, DataBlock>* IdentifierBasedNestedDataBlocks;
 *			tuple<uint32_t, DataBlock*> ArrayBasedNestedDataBlocks;
 *		}
 *	}
 */

namespace Serialization {
	//@todo Make all serialization functions contextual so they can print more detailed warning or error infomation
	struct ISerializer {
		virtual ~ISerializer() {}

		//Serialize to and from binary data streams
		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) = 0;
		virtual bool DeserializeBinary( void* Data, std::istream& Stream ) = 0;

		//Serialize to and from human-readable text streams
		virtual void SerializeText( void const* Data, std::ostringstream& Stream ) = 0;
		virtual bool DeserializeText( void* Data, std::istringstream& Stream ) = 0;

		//Serialize to and from a fast binary data stream
		//These methods should only be used if the reading and writing are done by the same binary version.
		virtual void SerializeFastBinary( void const* Data, std::ostream& Stream ) {}
		virtual void DeserializeFastBinary( void* Data, std::istream& Stream ) {}
	};
}
