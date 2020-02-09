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
 * to the stream, while the Deserialize method is responsible for reading a data block and
 * extracting the information from it into the object.
 *
 * An example of what a data block looks like if it were directly expressed as a struct:
 *	struct DataBlock {
 *		uint32_t DataSize;
 *		union {
 *			char data[];
 *			tuple<uint16_t, DataBlock> IdentifierBasedNestedDataBlocks[];
 *			tuple<uint32_t, DataBlock*> ArrayBasedNestedDataBlocks;
 *		}
 *	};
 */

namespace Reflection {
	struct TypeInfo;
}

namespace Serialization {
	//@todo Make all serialization functions contextual so they can print more detailed warning or error infomation
	/** Interface that provides serialization implementations for a type */
	struct ISerializer {
		virtual ~ISerializer() = default;

		/** Serialize to and from binary data streams */
		virtual bool SerializeBinary(Reflection::TypeInfo const& type, void const* data, std::ostream& stream) const = 0;
		virtual bool DeserializeBinary(Reflection::TypeInfo const& type, void* data, std::istream& stream) const = 0;

		/** Serialize to and from human-readable text streams */
		//virtual void SerializeText( void const* data, std::ostringstream& stream ) const = 0;
		//virtual bool DeserializeText( void* data, std::istringstream& stream ) const = 0;

		/** Serialize to and from a fast binary data stream */
		//These methods should only be used if the reading and writing are done by the same binary version. A lot of information will be omitted, and many assumptions will be made.
		//virtual void SerializeFastBinary( void const* data, std::ostream& stream ) const = 0;
		//virtual void DeserializeFastBinary( void* data, std::istream& stream ) const = 0;
	};

	/** Returns whether it's possible to serialize a type (whether it has a serializer interface or not) */
	bool CanSerializeType(Reflection::TypeInfo const& type);
	/** Returns whether a type should be serialized (based on its flags and whether it has a serializer interface) */
	bool ShouldSerializeType(Reflection::TypeInfo const& type);

	/** Serialize data to and from the stream as if it was the provided type */
	bool SerializeTypeBinary(Reflection::TypeInfo const& type, void const* data, std::ostream& stream);
	bool DeserializeTypeBinary(Reflection::TypeInfo const& type, void* data, std::istream& stream);
}
