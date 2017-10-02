#pragma once

#include "blaze.h"

#include <unordered_map>
#include <list>
#include <functional>
#include <memory>

namespace BLAZE_NAMESPACE {

	namespace {
		uint32_t new_uid() {
			static uint32_t i = 0;
			i++;
			return i;
		}

		template <typename T>
		uint32_t get_uid() {
			static uint32_t uid = new_uid();
			return uid;
		}
	}

	class Event {
		public:
			virtual ~Event() {}
	};

	class ChatMessage : public Event {
		public:
			ChatMessage(std::string text) : _text(text) {}
			const std::string& get_text() const {
				return _text;
			}

		private:
			std::string _text;
	};

	class EventBus {
		public:
			// @todo This needs to return a handler that on destruction unsubscribes
			template <typename T>
			auto subscribe(std::function<void(std::shared_ptr<T>)> handler) {
				static_assert(std::is_base_of<Event, ChatMessage>(), "Event type is not derived from base Event class");
				uint32_t uid = get_uid<T>();
				_subscribers[uid].push_back([handler](std::shared_ptr<Event> event){
					handler(std::reinterpret_pointer_cast<T>(event));
				});

				auto it = _subscribers[uid].end();
				it--;
				return it;
			}

			template <typename T>
			auto unsubscribe(std::list<std::function<void(std::shared_ptr<Event>)>>::iterator it) {
				uint32_t uid = get_uid<T>();
				_subscribers[uid].erase(it);
			}

			template <typename T>
			void send(std::shared_ptr<T> event) {
				uint32_t uid = get_uid<T>();
				for (auto handler : _subscribers[uid]) {
					handler(event);
				}
			}

		private:
			std::unordered_map<std::string, uint32_t> _event_ids;

			// This is terrible
			std::unordered_map<uint32_t, std::list<std::function<void(std::shared_ptr<Event>)>>> _subscribers;
	};
}
