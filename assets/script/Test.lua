-- Called when script is loaded
function init()
	print("This is the init script of the test archive")
	subscription = blaze.ChatSubscription.new(chat_handler)

	durability = 12
end

-- Called every frame
function update()
	print(get_lang():get("pickaxe.name"))
	print(get_lang():get("pickaxe.description", durability, 100))

	durability = durability - 1
end

-- Called on desctructor
function done()
	print("Cleanup for test archive")
	-- subscription:unsubscribe();
end

function chat_handler(event)
	print(string.format("<USERNAME> %s", event.text))
end
