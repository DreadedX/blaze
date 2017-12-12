local objects = require "base/Objects"

-- Called when script is loaded
function init()
	objects.register()
	subscription = blaze.ChatSubscription.new(chat_handler)

	durability = 12

	-- In a real game this would be called by some kind of mod loader
	blaze.load_archive("my_first_mod")
end

-- Called every frame
function update()
	log(get_lang():get("pickaxe.name"))
	log(get_lang():get("pickaxe.description", durability, 100))

	durability = durability - 1
end

-- Called on desctructor
function done()
	log("Cleanup for test archive")
	-- subscription:unsubscribe();
end

function chat_handler(event)
	log(string.format("<USERNAME> %s", event.text))
end
