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

			-- Remove whitespaces at start and end
			lines[i] = lines[i]:match("^%s*(.-)%s*$")

			-- Escape characters, needs to be improved and error out on invalid escape characters \0 is banned
			-- \ at the end of the line should continue on the next line
			lines[i] = lines[i]:gsub("\\a", '\a') -- Bell
			lines[i] = lines[i]:gsub("\\b", '\b') -- Backspace
			lines[i] = lines[i]:gsub("\\f", '\f') -- Form feed
			lines[i] = lines[i]:gsub("\\n", '\n') -- Newline
			lines[i] = lines[i]:gsub("\\r", '\r') -- Carriage return
			lines[i] = lines[i]:gsub("\\t", '\t') -- Horizontal tab
			lines[i] = lines[i]:gsub("\\v", '\v') -- Vertical tab
			-- lines[i] = lines[i]:gsub("\\\\", '\\') -- Backslash
			-- lines[i] = lines[i]:gsub("\\\"", '\"') -- Double quote
			-- lines[i] = lines[i]:gsub("\\\'", '\'') -- Single quote
			-- lines[i] = lines[i]:gsub("\\\[", '\[') -- Left square bracket
			-- lines[i] = lines[i]:gsub("\\\]", '\]') -- Right square bracket

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

			assert(matched, "Invalid formating on line " .. i)

		end
	end

	return output
end

return langpack
