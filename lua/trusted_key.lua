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

key = readAll("../pub.key")
file = io.open("trusted_key.h", "w+")
io.output(file)
io.write("#pragma once\n\n#include <cstdint>\n\nuint8_t trusted_key[] = {")
io.write(key:tohex());
io.write("};")
