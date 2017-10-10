local function langpack(input)
	i = 1
	storing = true
	lines = {}

	for _, byte in ipairs(input) do
		-- Ignore lines with comments
		if string.char(byte) == ';' then
			storing = false
		end

		if string.char(byte) == '\n' then
			if not storing then
				storing = true
			end
			if not lines[i] then
				lines[i] = ""
			end
			i = i + 1
		elseif storing then
			if lines[i] == nil then
				lines[i] = ""
			end
			lines[i] = lines[i] .. string.char(byte)
		end

	end

	output = helper.new_byte_vector()

	section = ""
	for i,line in ipairs(lines) do
		line = line:match("^%s*(.-)%s*$")

		if line ~= "" then
			matched = false

			-- Line must end directly after section name
			new_section = line:match('^%[([^%[%]]+)%]$')
			if new_section then
				section = new_section
				matched = true
			end

			key, value = line:match('^([%w|_]+)%s-=%s-(.+)$')
			if key and value then
				if section ~= "" then
					key = section .. '.' .. key
				end

				for i = 1,#key do
					output:add(string.byte(key, i))
				end
				output:add(0)
				for i = 1,#value do
					output:add(string.byte(value, i))
				end
				output:add(0)

				matched = true
			end

			assert(matched, "Invalid formating on line " .. i)

		end
	end

	return output
end

return langpack
