#pragma once

#include "blaze.h"

#include <unordered_map>
#include <list>
#include <functional>
#include <memory>

#include <iostream>
#include <iterator>

namespace BLAZE_NAMESPACE {

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

	class MissingDependencies : public Event {
		public:
			MissingDependencies(std::string name, std::vector<std::pair<std::string, uint16_t>> missing) : _name(name), _missing(missing) {}

			const auto& get_missing() const {
				return _missing;
			}

			const auto& get_name() const {
				return _name;
			}

		private:
			std::string _name;
			std::vector<std::pair<std::string, uint16_t>> _missing;
	};

	class event_bus {
		public:
			template <typename T>
			static void send(std::shared_ptr<T> event) {
				uint32_t uid = get_uid<T>();
				for (auto handler : _subscribers[uid]) {
					handler(event);
				}
			}

			template <typename T>
			class Subscription {
				public:
					// @todo Make a subscribe function that return this class
					Subscription(std::function<void(std::shared_ptr<T>)> handler) {
						static_assert(std::is_base_of<Event, ChatMessage>(), "Event type is not derived from base Event class");
						uint32_t uid = get_uid<T>();

						_subscribers[uid].push_back([handler](std::shared_ptr<Event> event){
							handler(std::dynamic_pointer_cast<T>(event));
						});

						_it = _subscribers[uid].end();
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
							_subscribers[uid].erase(_it);
							_valid = false;
						}
					}

				private:

					std::list<std::function<void(std::shared_ptr<Event>)>>::iterator _it;
					bool _valid = true;
			};

		private:
			// @todo Make sure this does not create multiple id's for one type
			template <typename T>
			static uint32_t get_uid() {
				static uint32_t uid = _id_counter;
				_id_counter++;
				return uid;
			}

			// @todo Improve this
			static std::unordered_map<uint32_t, std::list<std::function<void(std::shared_ptr<Event>)>>> _subscribers;
			static uint32_t _id_counter;
	};
}
