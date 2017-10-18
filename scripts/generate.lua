#!/bin/lua
function readAll(file)
    local f = io.open(file, "rb")
	if f then
		local content = f:read("*all")
		f:close()
		return content
	end
end
function string.tohex(str)
    return (str:gsub('.', function (c)
        return string.format('0x%02X,', string.byte(c))
    end))
end

-- Write trusted key
do
	-- @todo This will fail if we are building from scratch
	local key = readAll("../keys/test.pub")

	if key then
		local content = ""
		content = content .. "#include \"trusted_key.h\"\n\n"
		content = content .. "uint8_t trusted_key[] = {" .. key:tohex() .. "};\n\n"
		content = content .. "uint8_t* get_trusted_key() {\n"
		content = content .. "\treturn trusted_key;\n"
		content = content .. "}"

		local file = io.open("../modules/generated/src/trusted_key.cpp", "rb")
		local current = ""
		if file then
			current = file:read("*all")
			file:close()
		end

		if current ~= content then
			local file = io.open("../modules/generated/src/trusted_key.cpp", "w+")
			io.output(file)
			io.write(content)
			print "Regenerating trusted_key.cpp"
			file:close()
		end
	end
end

-- Write version
do
	-- Version number is based on commit count
	local handle = io.popen("git rev-list --count HEAD")
	local version_number = handle:read("a*")
	handle:close()

	-- Version string is based on current git branch
	local handle = io.popen("git describe HEAD --always")
	local version_string = string.sub(handle:read("a*"), 0, -2)
	handle:close()

	local content = ""
	content = content .. "#include \"version.h\"\n\n"
	content = content .. "uint32_t get_version_number() {\n"
	content = content .. "\treturn " .. version_number .. ";\n"
	content = content .. "}\n\n"
	content = content .. "std::string get_version_string() {\n"
	content = content .. "\treturn \"" .. version_string .. "\";\n"
	content = content .. "}"

	local file = io.open("../modules/generated/src/version.cpp", "rb")
	local current = ""
	if file ~= nil then
		current = file:read("*all")
		file:close()
	end

	if current ~= content then
		local file = io.open("../modules/generated/src/version.cpp", "w+")
		io.output(file)
		io.write(content)
		print "Regenerating version.cpp"
		file:close()
	end
end
