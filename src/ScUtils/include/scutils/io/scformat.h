#ifndef SCFORMAT_H
#define SCFORMAT_H

#include "scutils/text/scstring.h"
#if __has_include("scutils/io/format.h")
#	include "scutils/io/format.h"
#else
#	include "../deps/spdlog/include/spdlog/fmt/bundled/format.h"
#endif

template <>
struct fmt::formatter<ScString> : fmt::formatter<fmt::string_view> {
	template <typename FormatContext>
	auto format(const ScString& s, FormatContext& ctx) const {
		return fmt::formatter<fmt::string_view>::format({ s.data(), s.size() }, ctx);
	}
};

SC_BEGIN_NAMESPACE

template <typename... Args>
inline ScString format(const ScString& fmt, Args &&...args) {
	try {
		ScString result;
		result.reserve(fmt::formatted_size(fmt.data(), std::forward<Args>(args)...));
		fmt::format_to(std::back_inserter(result), fmt.data(), std::forward<Args>(args)...);
		return result;
	}
	catch (const std::exception& e) {
		fprintf(stderr, "[%s]String formatting exception: %s.\n", __FUNCTION__, e.what());
		return {};
	}
}

SC_END_NAMESPACE

#endif // SCFORMAT_H