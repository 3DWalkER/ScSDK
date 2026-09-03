#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/fmt/bundled/format.h>
#include <array>
#include <string>

// 自定义日志级别名称
const std::array<std::string, 6> custom_level_names = {
	"追踪", "调试", "信息", "警告", "错误", "严重错误"
};

// 自定义日志级别对应的颜色
const std::array<std::string, 6> custom_level_colors = {
	"gray", "blue", "green", "yellow", "red", "darkred"
};

std::string time_point_to_string(const std::chrono::system_clock::time_point& time_point) {
	// 将 time_point 转换为 std::time_t
	std::time_t time_t = std::chrono::system_clock::to_time_t(time_point);

	// 将 std::time_t 转换为 std::tm
	std::tm* local_time = std::localtime(&time_t);

	// 格式化输出
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
	return std::string(buffer);
}

// 自定义日志级别格式化器
class CustomLevelFormatter : public spdlog::custom_flag_formatter {
public:
	void format(const spdlog::details::log_msg &msg, const std::tm &, spdlog::memory_buf_t &dest) override {
		const auto level = static_cast<size_t>(msg.level);
		std::string payload_str(msg.payload.data(), msg.payload.size());

		// 将 spdlog::log_clock::time_point 转换为 std::chrono::system_clock::time_point
		auto sys_time_point = std::chrono::time_point_cast<std::chrono::system_clock::duration>(msg.time);
		// 将 std::chrono::system_clock::time_point 转换为 std::time_t
		auto time_t = std::chrono::system_clock::to_time_t(sys_time_point);

		std::string time = time_point_to_string(msg.time);
		fmt::format_to(std::back_inserter(dest), "<a style='color:{}'>【{} | {}】 {}<br/>",
			custom_level_colors[level], custom_level_names[level], time, payload_str);
		//fmt::format_to(std::back_inserter(dest), "");
	}

	std::unique_ptr<custom_flag_formatter> clone() const override {
		return spdlog::details::make_unique<CustomLevelFormatter>();
	}
};

int main() {

	auto logger = std::make_shared<spdlog::logger>("Console", std::move(std::make_shared<spdlog::sinks::stdout_color_sink_mt>()));
	// 创建控制台日志器
	auto console = spdlog::stdout_color_mt("console");

	//// 创建自定义格式化器
	//auto formatter = std::make_unique<spdlog::pattern_formatter>();
	//formatter->add_flag<CustomLevelFormatter>('L').set_pattern("%L");

	//// 设置日志器的格式化器
	//console->set_formatter(std::move(formatter));

	//// 设置默认日志器
	//spdlog::set_default_logger(console);

	// 记录不同级别的日志
	//spdlog::trace("这是一条追踪日志");
	//spdlog::debug("这是一条调试日志");
	//spdlog::info("这是一条信息日志");
	//spdlog::warn("这是一条警告日志");
	//spdlog::error("这是一条错误日志");
	//spdlog::critical("这是一条严重错误日志");
	logger->log(spdlog::level::trace, "这是一条信息日志");
	return 0;
}