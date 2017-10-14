local function escape_keys(line)
	-- Escape characters, needs to be improved and error out on invalid escape characters \0 is banned
	-- \ at the end of the line should continue on the next line
	line = line:gsub("\\a", '\a') -- Bell
	line = line:gsub("\\b", '\b') -- Backspace
	line = line:gsub("\\f", '\f') -- Form feed
	line = line:gsub("\\n", '\n') -- Newline
	line = line:gsub("\\r", '\r') -- Carriage return
	line = line:gsub("\\t", '\t') -- Horizontal tab
	line = line:gsub("\\v", '\v') -- Vertical tab
	-- line = line:gsub("\\\\", '\\') -- Backslash
	-- line = line:gsub("\\\"", '\"') -- Double quote
	-- line = line:gsub("\\\'", '\'') -- Single quote
	-- line = line:gsub("\\\[", '\[') -- Left square bracket
	-- line = line:gsub("\\\]", '\]') -- Right square bracket
	return line
end

local function langpack(input)
	i = 1
	storing = true

	line = ""
	section = ""
	output = helper.new_byte_vector()

	for _, byte in ipairs(input) do
		-- Ignore lines with comments
		if string.char(byte) == ';' then
			storing = false
		end

		if string.char(byte) == '\n' then
			if not storing then
				storing = true
			end

			-- Remove whitespaces at start and end
			-- @todo Just no store leading whitespace
			line = line:match("^%s*(.-)%s*$")

			line = escape_keys(line)

			if string.sub(line, -1, -1) == '\\' then
				line = string.sub(line, 0, -2)
				-- @todo Properly remove whitespace before \
			else
				matched = false

				if line == "" then
					matched = true
				end

				print(line)

				-- Line must end directly after section name
				new_section = line:match('^%[([^%[%]]+)%]$')
				if new_section then
					section = new_section
					matched = true
				end

				key, value = line:match('^([%w|_]+)%s-=%s-(.+)$')
				if key and value then
					-- Remove leading spaces in value
					value = value:match("^%s*(.-)%s*$")
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

				-- @todo This is completely wrong if multiline stuff is used, solution, mrege this with the other loop and keep an actual counter of the line number
				assert(matched, "Invalid formating on line " .. i)

				line = ""
			end
			i = i + 1
		elseif storing then
			line = line .. string.char(byte)
		end
	end

	return output
end

return langpack
