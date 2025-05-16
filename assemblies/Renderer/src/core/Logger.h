// VR Renderer - Logger
// Rodolphe VALICON
// 2025

#pragma once

#include "utils/Macros.h"

#include <iostream>
#include <format>
#include <string>

namespace vr {
	namespace logger {

		enum class MessageType : uint32_t {
			Fatal = BIT(5),
			Error = BIT(4),
			Warning = BIT(3),
			Info = BIT(2),
			Debug = BIT(1),
			Trace = BIT(0),
		};

		constexpr const char* GetTag(MessageType type) {
			switch (type) {
			case MessageType::Fatal:
				return "\033[38;5;255m\033[48;5;161m FATAL \033[0m";
			case MessageType::Error:
				return "\033[38;5;197m ERROR \033[0m";
			case MessageType::Warning:
				return "\033[38;5;214mWARNING\033[0m";
			case MessageType::Info:
				return "\033[38;5;77m INFO  \033[0m";
			case MessageType::Debug:
				return "\033[38;5;75m DEBUG \033[0m";
			case MessageType::Trace:
				return "\033[38;5;245m TRACE \033[0m";
			default:
				return nullptr;
			}
		}

		template<class ...Types>
		void log(MessageType type, std::format_string<Types...> format_message, Types&& ...args) {
			const char* tag = GetTag(type);

			std::cout << std::format("[{}] {}", tag, std::format(format_message, std::forward<Types>(args)...)) << '\n';
		}

		template<class ...Types>
		void fatal(std::format_string<Types...> format_message, Types&& ...args) {
			log(MessageType::Fatal, format_message, std::forward<Types>(args)...);
		}

		template<class ...Types>
		void error(std::format_string<Types...> format_message, Types&& ...args) {
			log(MessageType::Error, format_message, std::forward<Types>(args)...);
		}

		template<class ...Types>
		void warn(std::format_string<Types...> format_message, Types&& ...args) {
			log(MessageType::Warning, format_message, std::forward<Types>(args)...);
		}

		template<class ...Types>
		void info(std::format_string<Types...> format_message, Types&& ...args) {
			log(MessageType::Info, format_message, std::forward<Types>(args)...);
		}

		template<class ...Types>
		void debug(std::format_string<Types...> format_message, Types&& ...args) {
			log(MessageType::Debug, format_message, std::forward<Types>(args)...);
		}

		template<class ...Types>
		void trace(std::format_string<Types...> format_message, Types&& ...args) {
			log(MessageType::Trace, format_message, std::forward<Types>(args)...);
		}

	}
}
