function chat_handler(event)
	print(string.format("CHAT MESSAGE: %s", event:get_text()))
end

-- Called when script is loaded
function init()
	it = subscribe_chat_event(chat_handler)
end

-- Called every frame
function update()
end

-- Called on desctructor
function done()
end

init()

-- unsubscribe_chat_event(it)
