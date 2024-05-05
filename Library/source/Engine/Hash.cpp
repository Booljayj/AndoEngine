#include "Engine/Hash.h"

namespace Archive {
	void Serializer<Hash32>::Write(Output& archive, Hash32 const hash) {
		Serializer<uint32_t>::Write(archive, hash.ToValue());
	}

	void Serializer<Hash32>::Read(Input& archive, Hash32& hash) {
		uint32_t value = 0;
		Serializer<uint32_t>::Read(archive, value);
		hash = Hash32{ value };
	}

	void Serializer<Hash64>::Write(Output& archive, Hash64 const hash) {
		Serializer<uint64_t>::Write(archive, hash.ToValue());
	}

	void Serializer<Hash64>::Read(Input& archive, Hash64& hash) {
		uint64_t value = 0;
		Serializer<uint64_t>::Read(archive, value);
		hash = Hash64{ value };
	}

	void Serializer<Hash128>::Write(Output& archive, Hash128 const& hash) {
		Serializer<uint64_t>::Write(archive, hash.ToLowValue());
		Serializer<uint64_t>::Write(archive, hash.ToHighValue());
	}

	void Serializer<Hash128>::Read(Input& archive, Hash128& hash) {
		uint64_t low = 0;
		uint64_t high = 0;
		Serializer<uint64_t>::Read(archive, low);
		Serializer<uint64_t>::Read(archive, high);
		hash = Hash128{ low, high };
	}
}

namespace YAML {
	Node convert<Hash32>::encode(Hash32 hash) {
		return convert<uint32_t>::encode(hash.ToValue());
	}
	bool convert<Hash32>::decode(Node const& node, Hash32& hash) {
		hash = Hash32{ node.as<uint32_t>() };
		return true;
	}

	Node convert<Hash64>::encode(Hash64 hash) {
		return convert<uint64_t>::encode(hash.ToValue());
	}
	bool convert<Hash64>::decode(Node const& node, Hash64& hash) {
		hash = Hash64{ node.as<uint64_t>() };
		return true;
	}
	
	Node convert<Hash128>::encode(Hash128 hash) {
		Node node{ NodeType::Sequence };
		node.push_back(hash.ToLowValue());
		node.push_back(hash.ToHighValue());
		return node;
	}
	bool convert<Hash128>::decode(Node const& node, Hash128& hash) {
		if (!node.IsSequence() || node.size() != 2) return false;

		hash = Hash128{ node[0].as<uint64_t>(), node[1].as<uint64_t>() };
		return true;
	}
}

std::format_context::iterator std::formatter<Hash32>::format(const Hash32& hash, format_context& ctx) const {
	char scratch[20] = { 0 };
	auto const result = format_to_n(scratch, sizeof(scratch), "{:08x}"sv, hash.ToValue());
	return formatter<string_view>::format(string_view{ scratch, result.out }, ctx);
}


std::format_context::iterator std::formatter<Hash64>::format(const Hash64& hash, format_context& ctx) const {
	char scratch[20] = { 0 };
	auto const result = format_to_n(scratch, sizeof(scratch), "{:016x}"sv, hash.ToValue());
	return formatter<string_view>::format(string_view{ scratch, result.out }, ctx);
}

std::format_context::iterator std::formatter<Hash128>::format(const Hash128& hash, format_context& ctx) const {
	char scratch[40] = { 0 };
	auto const result = format_to_n(scratch, sizeof(scratch), "{:016x}-{:016x}"sv, hash.ToHighValue(), hash.ToLowValue());
	return formatter<string_view>::format(string_view{ scratch, result.out }, ctx);
}
