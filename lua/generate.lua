#!/usr/sbin/lua
function readAll(file)
    local f = io.open(file, "rb")
    local content = f:read("*all")
    f:close()
    return content
end
function string.tohex(str)
    return (str:gsub('.', function (c)
        return string.format('0x%02X,', string.byte(c))
    end))
end

-- Write trusted key
do
	local key = readAll("../keys/test.pub")
	local file = io.open("../modules/generated/src/trusted_key.cpp", "w+")
	io.output(file)
	io.write("#include \"trusted_key.h\"\n\n")
	io.write("uint8_t trusted_key[] = {" .. key:tohex() .. "};\n\n")
	io.write("uint8_t* get_trusted_key() {\n")
	io.write("\treturn trusted_key;\n")
	io.write("}")
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

	local file = io.open("../modules/generated/src/version.cpp", "w+")
	io.output(file)
	io.write("#include \"version.h\"\n\n")
	io.write("uint32_t get_version_number() {\n")
	io.write("\treturn " .. version_number .. ";\n")
	io.write("}\n\n")
	io.write("std::string get_version_string() {\n")
	io.write("\treturn \"" .. version_string .. "\";\n")
	io.write("}")
end
