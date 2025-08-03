#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	template<typename T>
	static NumericTypeInfo::ENumericType GetNumericType() {
		if constexpr (std::is_same_v<T, bool>) return NumericTypeInfo::ENumericType::Binary;
		else if constexpr (std::floating_point<T>) return NumericTypeInfo::ENumericType::FloatingPoint;
		else if constexpr (std::is_signed_v<T>) return NumericTypeInfo::ENumericType::SignedInteger;
		else if constexpr (std::is_unsigned_v<T>) return NumericTypeInfo::ENumericType::UnsignedInteger;
		else throw std::logic_error{ "type is not numeric" };
	}

	template<typename T>
	struct TValuelessTypeInfo : public ValuelessTypeInfo {
		TValuelessTypeInfo(std::u16string_view name, std::u16string_view description)
			: ValuelessTypeInfo(ETypeClassification::Valueless, Reflect<T>::ID, MemoryParams{ 0, 0 }, FTypeFlags::None(), name, description)
		{}

		virtual void Destruct(void* instance) const override final {}
		virtual void Construct(void* instance) const override final {}
		virtual void Construct(void* instance, void const* other) const override final {}
		virtual void Copy(void* instance, void const* other) const override final {}
		virtual bool Equal(void const* instanceA, void const* instanceB) const override final { return true; }

		virtual void Serialize(Archive::Output& archive, void const* instance) const override final {}
		virtual void Deserialize(Archive::Input& archive, void* instance) const override final {}

		virtual YAML::Node Serialize(void const* instance) const override final { return YAML::Node{}; }
		virtual void Deserialize(YAML::Node const& node, void* instance) const override final {}
	};

	template<typename T>
	struct TNumericTypeInfo : public ImplementedTypeInfo<T, NumericTypeInfo> {
		TNumericTypeInfo(std::u16string_view name, std::u16string_view description)
			: ImplementedTypeInfo<T, NumericTypeInfo>(Reflect<T>::ID, name, description)
			, numeric_type(GetNumericType<T>())
		{}

		virtual uint64_t GetUnsignedInteger(void const* instance) const override final { return static_cast<uint64_t>(Cast(instance)); }
		virtual int64_t GetSignedInteger(void const* instance) const override final { return static_cast<int64_t>(Cast(instance)); }
		virtual double GetFloatingPoint(void const* instance) const override final { return static_cast<double>(Cast(instance)); }

		virtual void Set(void* instance, uint64_t value) const override final { Cast(instance) = static_cast<T>(value); }
		virtual void Set(void* instance, int64_t value) const override final { Cast(instance) = static_cast<T>(value); }
		virtual void Set(void* instance, double value) const override final { Cast(instance) = static_cast<T>(value); }
	};
}

#define L_DEFINE_REFLECT(Classification, QualifiedType, SimpleType, TypeDescription)\
	::Reflection::T ## Classification ## TypeInfo<QualifiedType> const info_ ## SimpleType{ u#QualifiedType, u ## TypeDescription ## sv }; \
	::Reflection::Classification ## TypeInfo const& Reflect<QualifiedType>::Get() { return info_ ## SimpleType; }

L_DEFINE_REFLECT(Valueless, void, void, "Void type, used for missing type values");
L_DEFINE_REFLECT(Valueless, std::monostate, monostate, "Variant monostate type, used to indicate the variant can have a 'stateless' or 'default constructed' option");

L_DEFINE_REFLECT(Numeric, std::byte, byte, "a byte of memory");
L_DEFINE_REFLECT(Numeric, bool, bool, "boolean");

L_DEFINE_REFLECT(Numeric, int8_t, int8_t, "signed 8-bit integer");
L_DEFINE_REFLECT(Numeric, uint8_t, uint8_t, "unsigned 8-bit integer");
L_DEFINE_REFLECT(Numeric, int16_t, int16_t, "signed 16-bit integer");
L_DEFINE_REFLECT(Numeric, uint16_t, uint16_t, "unsigned 16-bit integer");
L_DEFINE_REFLECT(Numeric, int32_t, int32_t, "signed 32-bit integer");
L_DEFINE_REFLECT(Numeric, uint32_t, uint32_t, "unsigned 32-bit integer");
L_DEFINE_REFLECT(Numeric, int64_t, int64_t, "signed 64-bit integer");
L_DEFINE_REFLECT(Numeric, uint64_t, uint64_t, "unsigned 64-bit integer");

L_DEFINE_REFLECT(Numeric, char, char, "single-byte");
L_DEFINE_REFLECT(Numeric, char8_t, char8_t, "8-bit character");
L_DEFINE_REFLECT(Numeric, char16_t, char16_t, "16-bit character");
L_DEFINE_REFLECT(Numeric, char32_t, char32_t, "32-bit character");

L_DEFINE_REFLECT(Numeric, float, float, "single-precision number");
L_DEFINE_REFLECT(Numeric, double, double, "double-precision number");

#undef L_DEFINE_REFLECT
