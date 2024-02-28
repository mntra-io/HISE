// Remove all warnings and make the loris library compile

// C++17 removes auto_ptr and some other helper classes so we'll make it compile with these definitions...
#if __cplusplus >= 201703L
#if _WIN32
namespace std
{
	template <typename U1, typename U2, typename U3> struct binary_function
	{
		using argument_type = U2;
	};
	template <typename U1, typename U2> struct unary_function
	{
		using argument_type = U2;
	};
}

#define bind1st bind
#define bind2nd bind

#endif

#if defined (LINUX) || defined (__linux__)
#define DEFINE_AUTO_PTR 0
#else
#define DEFINE_AUTO_PTR 1
#endif

#if DEFINE_AUTO_PTR
#define auto_ptr unique_ptr
#endif

#endif

#include "../JUCE/modules/juce_core/juce_core.h"
