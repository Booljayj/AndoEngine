#include "Engine/String.h"

namespace Reflection {
	template<typename T>
	struct TStringTypeInfo : public ImplementedTypeInfo<T, StringTypeInfo> {
		using StringTypeInfo::characters;

		TStringTypeInfo(std::u16string_view name, std::u16string_view description)
			: ImplementedTypeInfo<T, StringTypeInfo>(Reflect<T>::ID, name, description)
		{
			characters = &Reflect<T::value_type>::Get();
		}

		virtual std::u16string Get(void const* instance) const override final { throw std::runtime_error{ "not implemented yet" }; }
		virtual void Set(void const* instance, std::u16string const& string) const override final { throw std::runtime_error{ "not implemented yet" }; }
		virtual void Reset() const override final { throw std::runtime_error{ "not implemented yet" }; }
	};
}

::Reflection::StringTypeInfo const& Reflect<std::string>::Get() {
	static ::Reflection::TStringTypeInfo<std::string> const info_string{ u"std::string", u"ansi string"sv };
	return info_string;
};
::Reflection::StringTypeInfo const& Reflect<std::u8string>::Get() {
	static ::Reflection::TStringTypeInfo<std::u8string> const info_u8string{ u"std::u8string", u"UTF-8 string"sv };
	return info_u8string;
};
::Reflection::StringTypeInfo const& Reflect<std::u16string>::Get() {
	static ::Reflection::TStringTypeInfo<std::u16string> const info_u16string{ u"std::u16string", u"UTF-16 string"sv };
	return info_u16string;
};
::Reflection::StringTypeInfo const& Reflect<std::u32string>::Get() {
	static ::Reflection::TStringTypeInfo<std::u32string> const info_u32string{ u"std::u32string", u"UTF-32 string"sv };
	return info_u32string;
};