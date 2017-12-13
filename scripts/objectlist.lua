local function objectlist(objects)
	return function(input)
		local script = "local objects = {}\n"
		script = script .. "function objects.register()\n"

		for _, object_name in ipairs(objects) do
			script = script .. "print(\"Registering: " .. object_name .. "\")\n"
		end

		script = script .. "end\n"
		script = script .. "return objects"

		output = helper.new_byte_vector()

		for i = 1,#script do
			output:add(string.byte(script, i))
		end

		return output
	end
end

return objectlist
