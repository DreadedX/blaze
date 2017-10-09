-- Called when script is loaded
function init()
	print("Init code goes here");
	subscription = blaze.ChatSubscription.new(chat_handler);
end

-- Called every frame
function update()
	print("Update code goes here");
end

-- Called on desctructor
function done()
	print("Cleanup code goes here");
	subscription:unsubscribe();
end

function chat_handler(event)
	print(string.format("CHAT MESSAGE: %s", event:get_text()));
end
