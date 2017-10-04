#pragma once

#include "blaze.h"

#include <unordered_map>
#include <list>
#include <functional>
#include <memory>

#include <iostream>
#include <iterator>

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

	namespace event_bus {
		namespace _private {
			// @note The only reason this is here is because we cannot put template functions in cpp file
			// @todo Improve this
			extern std::unordered_map<uint32_t, std::list<std::function<void(std::shared_ptr<Event>)>>> subscribers;
		}

		template <typename T>
		static void send(std::shared_ptr<T> event) {
			uint32_t uid = get_uid<T>();
			for (auto handler : _private::subscribers[uid]) {
				handler(event);
			}
		}

		template <typename T>
		class Subscription {
			public:
				Subscription(std::function<void(std::shared_ptr<T>)> handler) {
					static_assert(std::is_base_of<Event, ChatMessage>(), "Event type is not derived from base Event class");
					uint32_t uid = get_uid<T>();
					_private::subscribers[uid].push_back([handler](std::shared_ptr<Event> event){
							handler(std::dynamic_pointer_cast<T>(event));
					});

					_it = _private::subscribers[uid].end();
					_it--;
				}
				Subscription(const Subscription&) = delete;
				Subscription(Subscription&& o) : _it(std::move(o._it)) {}

				~Subscription() {
					unsubscribe();
				}

				void unsubscribe() {
					if (_valid) {
						uint32_t uid = get_uid<T>();
						_private::subscribers[uid].erase(_it);
						_valid = false;
					}
				}

			private:
				std::list<std::function<void(std::shared_ptr<Event>)>>::iterator _it;
				bool _valid = true;
		};
	}
}
