function init()
	subscription = blaze.ChatSubscription.new(chat_handler)

	co = coroutine.create(durability)

	-- In a real game this would be called by some kind of mod loader
	blaze.load_archive("my_first_mod")

	money = 1

	s = coroutine.create(script)
end

function durability()
	for i=0,100 do
		coroutine.yield(100-i)
	end
end

function say(str)
	print(str)
	coroutine.yield()
end

function script()
	for i=1,3 do
		say(get_lang():get("tutorial.test." .. i))
	end
end

function update()
	coroutine.resume(s)
	_, d = coroutine.resume(co)
	print(get_lang():get("item.pickaxe.name"))
	print(get_lang():get("item.pickaxe.description", "current", d, "total", 100))

	-- print(get_lang():get("ui.hints.currency", "amount", 10))
	print(get_lang():get("ui.hint.currency", "amount", money, "amount2", money))

	money = money + 1
end

function done()
	print "Cleanup for test archive"
	-- subscription:unsubscribe();
end

function chat_handler(event)
	print("<USERNAME> " .. event.text);
end
