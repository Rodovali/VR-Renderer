// VR Renderer - Event dispatcher
// Rodolphe VALICON
// 2025

#pragma once

#include "event/Event.h"

namespace vr {

	/// @brief An event dispatcher
	class EventDispatcher final {
	public:
		/// @brief Constructs a dispatcher for the given event.
		/// @param event A const reference to the event to dispatch
		EventDispatcher(const Event& event) : m_event(event) {}

		/// @brief Dispatch the event. If the event is of the given type, the given function is executed.
		/// @tparam E A type implementing the Event interface.
		/// @tparam F A callable type 
		/// @param func The function to execute if the event is of type T
		/// @return 
		template<typename E, typename F>
		void dispatch(const F& func) {
			if (!m_event.m_handled && m_event.getID() == E::getStaticID()) {
				m_event.m_handled = func(static_cast<const E&>(m_event));
			}
		}

	private:
		const Event& m_event;
	};

}
