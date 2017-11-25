#pragma once

#include "blaze.h"

#include <string>

namespace BLAZE_NAMESPACE {

	class Platform {
		public:
			virtual const std::string get_base_path() const = 0;
			virtual bool has_async_support() const = 0;
	};

	class Linux : public Platform {
		public:
			const std::string get_base_path() const override {
				return ".";
			}

			bool has_async_support() const override {
				return true;
			}
	};

	class Android : public Platform {
		public:
			const std::string get_base_path() const override {
				return "/storage/emulated/0/Android/data/nl.mtgames.blazebootstrap/files";
			}

			bool has_async_support() const override{
				return true;
			}
	};

	class Web : public Platform {
		public:
			const std::string get_base_path() const override {
				return ".";
			}

			bool has_async_support() const override{
				return false;
			}
	};
}
