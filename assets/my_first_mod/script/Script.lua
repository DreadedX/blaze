local hello = require "my_first_mod/script/Hello"

function init() 
	log "==== Hello world! This is my first mod! ===="

	hello.say("You")
	hello.say("World")
end

function update()
end

function done()
end
