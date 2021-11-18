#ifndef TEMPLATE_CONFIG_H__
#define TEMPLATE_CONFIG_H__

#if 0
// Example usage:
//   (note for this example: switching off of TEMPLATE_CONFIG_USE_AUTO_NTTP should be
//     read as an attempt to show both uses, rather than an implication that both
//     uses must be provided for. For end-user code, only using one of auto-nttp or 
//     no auto-nttp is perfectly acceptable).

struct FirstTypeTag {};
struct SecondTypeTag {};
struct ThirdTypeTag {};

template<class... Args>
struct UsesConfig {
    using FirstType = MyConfig::GetTypeOrDefault_t<FirstTypeTag, double, Args...>;
    typedef MyConfig::GetTypeOrDefault_t<SecondTypeTag, long, Args...> SecondType;
#if TEMPLATE_CONFIG_USE_AUTO_NTTP
    static const int ThirdValue = MyConfig::GetValueOrDefault<ThirdTypeTag, 3, Args...>::value;
#else
    static const int ThirdValue = MyConfig::GetValueOrDefault<ThirdTypeTag, int, 3, Args...>::value;
#endif
};

int main()
{
	using namespace MyConfig;
	static_assert(std::is_same<
		UsesConfig<
			Config<FirstTypeTag,int>
		>::FirstType,
		int
	>::value);
	static_assert(std::is_same<
		UsesConfig<
			Config<FirstTypeTag,int>
		>::SecondType,
		long
	>::value);
	static_assert(std::is_same<
		UsesConfig<
			Config<FirstTypeTag,int>,
			Config<SecondTypeTag,char>
		>::SecondType,
		char
	>::value);
	static_assert(std::is_same<
		UsesConfig<
			Config<SecondTypeTag,char>,
			Config<FirstTypeTag,int>
		>::SecondType,
		char
	>::value);
	static_assert(
		UsesConfig<
			Config<FirstTypeTag,int>
		>::ThirdValue == 3
	);
	static_assert(
		UsesConfig<
			Config<FirstTypeTag,int>,
#if __cplusplus >= 201703L
			ConfigValue<ThirdTypeTag,4>
#else
			ConfigValue<ThirdTypeTag,int,4>
#endif
		>::ThirdValue == 4
	);
	return 0;
}
 
#endif /* #if 0 */

#include <type_traits>


#ifndef TEMPLATE_CONFIG_USE_AUTO_NTTP
    #define TEMPLATE_CONFIG_USE_AUTO_NTTP (__cplusplus >= 201703L)
#endif

#ifndef TEMPLATE_CONFIG_USE_TEMPLATE_VARIABLE
    #define TEMPLATE_CONFIG_USE_TEMPLATE_VARIABLE (__cplusplus >= 201402L)
#endif

namespace MyConfig {

template<class Tag, class Default, class... Args>
struct GetTypeOrDefault;
	
template<class Tag, class Default, class... Args>
using GetTypeOrDefault_t = typename GetTypeOrDefault<Tag, Default, Args...>::type;

template<class Tag, class Default, class First, class... Rest>
struct GetTypeOrDefault<Tag,Default,First,Rest...> {
    typedef typename std::conditional<
	        std::is_same<Tag,typename First::tag>::value,
			First,
		    GetTypeOrDefault<Tag,Default,Rest...>
	    >::type::type type;
};

template<class Tag, class Default>
struct GetTypeOrDefault<Tag,Default> {
    typedef Default type;
};

#if TEMPLATE_CONFIG_USE_AUTO_NTTP
template<class Tag, auto Value, class... Args>
struct GetValueOrDefault;
#else /* TEMPLATE_CONFIG_USE_AUTO_NTTP */
template<class Tag, class Type, Type Value, class... Args>
struct GetValueOrDefault;
#endif /* TEMPLATE_CONFIG_USE_AUTO_NTTP */
	
#if TEMPLATE_CONFIG_USE_TEMPLATE_VARIABLE
#if TEMPLATE_CONFIG_USE_AUTO_NTTP
template<class Tag, auto Value, class... Args>
static constexpr auto GetValueOrDefault_v = GetValueOrDefault<Tag, Value, Args...>::value;
#else /* TEMPLATE_CONFIG_USE_AUTO_NTTP */
template<class Tag, class Type, Type Value, class... Args>
static constexpr auto GetValueOrDefault_v = GetValueOrDefault<Tag, Type, Value, Args...>::value;
#endif /* TEMPLATE_CONFIG_USE_AUTO_NTTP */
#endif /* TEMPLATE_CONFIG_USE_TEMPLATE_VARIABLE */

#if TEMPLATE_CONFIG_USE_AUTO_NTTP
template<class Tag, auto Value, class First, class... Rest>
struct GetValueOrDefault<Tag,Value,First,Rest...> {
    static const auto value = std::conditional<
			std::is_same<Tag,typename First::tag>::value,
		    First,
	        GetValueOrDefault<Tag,Value,Rest...>
	    >::type::value;
};
#else /* TEMPLATE_CONFIG_USE_AUTO_NTTP */
template<class Tag, class Type, Type Value, class First, class... Rest>
struct GetValueOrDefault<Tag,Type,Value,First,Rest...> {
    static const auto value = std::conditional<
	        std::is_same<Tag,typename First::tag>::value,
	        First,
	        GetValueOrDefault<Tag,Type,Value,Rest...>
	    >::type::value;
};
#endif /* TEMPLATE_CONFIG_USE_AUTO_NTTP */

#if TEMPLATE_CONFIG_USE_AUTO_NTTP
template<class Tag, auto Value>
struct GetValueOrDefault<Tag,Value> {
    static const auto value = Value;
};
#else /* TEMPLATE_CONFIG_USE_AUTO_NTTP */
template<class Tag, class Type, Type Value>
struct GetValueOrDefault<Tag,Type,Value> {
    static const auto value = Value;
};
#endif /* TEMPLATE_CONFIG_USE_AUTO_NTTP */

// helper class for providing a configuration tag->type pair
template<class Tag, class Type>
struct Config {
    typedef Tag tag;
    typedef Type type;
};

// helper class for providing a configuration tag->value pair
#if TEMPLATE_CONFIG_USE_AUTO_NTTP
template<class Tag, auto Value>
struct ConfigValue {
    typedef Tag tag;
    static const auto value = Value;
};
#else /* TEMPLATE_CONFIG_USE_AUTO_NTTP */
template<class Tag, class Type, Type Value>
struct ConfigValue {
    typedef Tag tag;
    static const Type value = Value;
};
#endif /* TEMPLATE_CONFIG_USE_AUTO_NTTP */

} /* namespace MyConfig */

#endif /* TEMPLATE_CONFIG_H__ */

