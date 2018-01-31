local objects = require "base/Objects"

local test = require "~bla"
-- @todo See engine.cpp on what we are going to do with this
-- local ChatSubscription = require "~ChatSubscription"

-- @todo We can implement a bunch of usefull functions here, like getting the language, we can propably even make this a usertype or something
-- @todo Also this needs to come from a require
local Script = {}
-- @todo These should all throw errors
function Script:init()
	print("init() not implemented")
end
function Script:update()
	print("update() not implemented")
end
function Script:done()
	print("done() not implemented")
end

function Script:new(o)
	o = o or {}
	setmetatable(o, self);
	self.__index = self;
	return o
end

-- local script = require "~Script"
local script = Script:new();

function script:init()
	objects.register()
	subscription = blaze.ChatSubscription.new(self.chat_handler)

	durability = 12

	-- In a real game this would be called by some kind of mod loader
	blaze.load_archive("my_first_mod")
end

function script:update()
	print(get_lang():get("pickaxe.name"))
	print(get_lang():get("pickaxe.description", durability, 100))

	durability = durability - 1
end

function script:done()
	print "Cleanup for test archive"
	-- subscription:unsubscribe();
end

function script.chat_handler(event)
	print("<USERNAME> " ..  event.text)
end

-- return script
-- @todo Move this into the engine, instead return the script object

function init()
	script:init()
end

function update()
	script:update()
end

function done()
	script:done()
end

