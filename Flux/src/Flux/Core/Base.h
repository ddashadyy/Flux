#pragma once

#include <memory>
#include <type_traits>

#ifdef FL_PLATFORM_WINDOWS
	#if FL_DYNAMIC_LINK
		#ifdef FL_BUILD_DLL
			#define FLUX_API __declspec(dllexport)
		#else
			#define FLUX_API __declspec(dllimport)
		#endif 
	#else 
		#define FLUX_API
	#endif
#else 
	#error Flux only supports Windows!
#endif 

#ifdef FL_ENABLE_ASSERTS
	#define FL_ASSERT(x, ...) { if(!(x)) { FL_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define FL_CORE_ASSERT(x, ...) { if(!(x)) { FL_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define FL_ASSERT(x, ...)
	#define FL_CORE_ASSERT(x, ...)
#endif


#define BIT(x) (1 << x)
#define FL_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define FL_NON_COPYABLE(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \

#define ENABLE_BITWISE_OPERATORS(EnumType) \
    inline EnumType operator|(EnumType a, EnumType b) { \
        return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(a) | static_cast<std::underlying_type_t<EnumType>>(b)); \
    } \
    inline EnumType& operator|=(EnumType& a, EnumType b) { \
        a = a | b; \
        return a; \
    } \
    inline EnumType operator&(EnumType a, EnumType b) { \
        return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(a) & static_cast<std::underlying_type_t<EnumType>>(b)); \
    } \
    inline bool operator==(EnumType a, std::underlying_type_t<EnumType> b) { \
        return static_cast<std::underlying_type_t<EnumType>>(a) == b; \
    }


namespace Flux {

	template <typename T>
	using Scope = std::unique_ptr<T>;

	template <typename T, typename ...Args >
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
	
	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename ...Args >
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}