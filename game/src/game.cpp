#include "asset_list.h"
#include "engine.h"

using namespace blaze;

enum class Input : int {
	MOVE_FORWARD,
	MOVE_BACKWARDS
};

template <typename T>
class SimpleEvent : public Event {
	public:
		SimpleEvent(T value) : _value(value) {}

		T get() { return _value; }

	private:
		T _value;
};

void test_handler(std::shared_ptr<ChatMessage> event) {
	std::cout << "<Dreaded_X> " << event->get_text() << '\n';;
}

void key_handler(std::shared_ptr<SimpleEvent<Input>> event) {
	std::cout << "Input: ";
	switch (event->get()) {
		case Input::MOVE_FORWARD:
			std::cout << "MOVE_FORWARD";
			break;
		case Input::MOVE_BACKWARDS:
			std::cout << "MOVE_FORWARD";
			break;
	}
	std::cout << '\n';
}

int main() {
	blaze::initialize({"archives/base.flm", "archives/test.flm"});
	
	flame::AssetData test = get_asset_list().find_asset("TestAsset");

	std::cout << "====ASSETS====\n";
	get_asset_list().debug_list_meta_assets();

	std::cout << "====TEST====\n";
	for (unsigned int i = 0; i < test.get_size(); ++i) {
		std::cout << test[i];
	}
	std::cout << '\n';

	// This is here so we don't have to constantly keep repackaging the assets
	// @todo This should go into a lua script
	auto fh = std::make_shared<flame::FileHandler>("assets/test.lua", std::ios::in);
	flame::MetaAsset lua_asset("LuaTest", fh, 10, flame::MetaAsset::Workflow());
	get_asset_list().add(lua_asset);

	{
		LuaScript script("LuaTest");
		script.run();
	}

	// test event bus
	auto subscription_handle = event_bus::subscribe<ChatMessage>(test_handler);
	// event_bus.unsubscribe<ChatMessage>(subscription_handle);

	event_bus::subscribe<SimpleEvent<Input>>(key_handler);


	event_bus::send(std::make_shared<ChatMessage>("Hello world!"));
	event_bus::send(std::make_shared<SimpleEvent<Input>>(Input::MOVE_FORWARD));
	event_bus::send(std::make_shared<ChatMessage>("This is a test"));
}
