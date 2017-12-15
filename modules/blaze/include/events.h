#pragma once

#include "blaze.h"

#include <unordered_map>
#include <list>
#include <functional>
#include <memory>

#include <iostream>
#include <iterator>
#include <vector>

namespace BLAZE_NAMESPACE {

	class Event {
		public:
			virtual ~Event() {}
	};

	class ChatMessage : public Event {
		public:
			ChatMessage(std::string text);
			const std::string& get_text() const;

		private:
			std::string _text;
	};

	class MissingDependencies : public Event {
		public:
			MissingDependencies(std::string name, std::vector<std::tuple<std::string, uint16_t, uint16_t>> missing);

			const std::vector<std::tuple<std::string, uint16_t, uint16_t>>& get_missing() const;
			const std::string& get_name() const;

		private:
			std::string _name;
			std::vector<std::tuple<std::string, uint16_t, uint16_t>> _missing;
	};

	class Error : public Event {
		public:
			Error(std::string error, std::string file, size_t line);

			const std::string& get_error() const;
			const std::string& get_context() const;

		private:
			std::string _error;
			std::string _context;
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
			static void subscribe(std::function<void(std::shared_ptr<T>)> handler) {
				static_assert(std::is_base_of<Event, ChatMessage>(), "Event type is not derived from base Event class");

				_subscribers[get_uid<T>()].push_back([handler](std::shared_ptr<Event> event){
					handler(std::dynamic_pointer_cast<T>(event));
				});
			}

			template <typename T>
			class Subscription {
				public:
					Subscription(std::function<void(std::shared_ptr<T>)> handler) {
						subscribe(handler);

						_it = _subscribers[get_uid<T>()].end();
						_it--;
					}

					Subscription(const Subscription&) = delete;
					Subscription(Subscription&& o) : _it(std::move(o._it)) {}

					~Subscription() {
						unsubscribe();
					}

					void unsubscribe() {
						if (_valid && !_subscribers.empty()) {
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
