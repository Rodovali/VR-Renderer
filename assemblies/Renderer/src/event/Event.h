// VR Renderer - Event
// Rodolphe VALICON
// 2025

#pragma once

#include <cstdint>
#include <string>

namespace vr {
	class EventDispatcher;

	/// @brief Interface for a generic Event.
	class Event {
		friend class EventDispatcher;
	public:
		virtual ~Event() {}

		/// @brief Provides the unique numerical ID of the event, generated at compile time.
		/// @return A 64-bit integer representing the event ID.
		virtual uint64_t getID() const = 0;

		/// @brief Provides the name of the event.
		/// @return A null terminated C-string containing the name of the event.
		virtual const char* getName() const = 0;

		/// @brief Formats the event and its payload into a string, for display.
		/// @return a std::string containing the formatted content.
		virtual std::string toString() const = 0;

		bool isHandled() const { return m_handled; }

	private:
		mutable bool m_handled = false;
	};

}

// The following macros are used to easily create and register events.
// HACK: Using static byte pointer as compile-time unique event id.

/// @brief Automatically fills the interface methods of an event class.
/// @param type The event type.
#define VR_EVENT_CLASS(type) \
	private: \
		static uint8_t s_eventID; \
	public: \
		static uint64_t getStaticID() { return reinterpret_cast<uint64_t>(&s_eventID); } \
		virtual uint64_t getID() const override { return getStaticID(); } \
		virtual const char* getName() const override { return #type; }

/// @brief Registers an event class by initializing its static event ID field.
/// Should be placed in a cpp file, as this action should be compiled.
#define VR_EVENT_REGISTER(type) uint8_t type::s_eventID
