local helper = {}

local function verify_error_type(name, archive_name, found, expected)
	print('\'' .. name .. "' in archive '" .. archive_name .. "' has type '" .. found .. "', expected '" .. expected .. '\'')
end
local function verify_error_missing(archive_name, expected, name)
	print("Archive '" .. archive_name.. "' is missing '" .. expected .. "' '" .. name .. '\'')
end

-- @todo Refactor this, maybe using recursion
-- @todo Is assets are not given a name it will cause an exception
-- @todo This is GARBAGE just redo everything
function helper.verify(archives, archive_template)
	valid = true
	for archive_name,archive in pairs(archives) do
		if (type(archive) == "table") then
			for key,expected in pairs(archive_template) do
				found = type(archive[key])
				if (found == "nil") then
					verify_error_missing(archive_name, expected, key)
					valid = false;
				elseif (found == "table" and type(expected) == "table") then
					expected2 = archive_template[key][next(archive_template[key])]
					for key2,value in pairs(archive[key]) do
						found2 = type(value)
						if (found2 == "table" and type(expected2) == "table") then
							for i,expected3 in ipairs(expected2) do
								found3 = type(value[i])
								if (found3 ~= expected3) then
									verify_error_type(string.format("%s.%s.%s", key, key2, i), archive_name, found3, expected3)
									valid = false
								end
							end
						elseif (found2 ~= expected2) then
							verify_error_type(string.format("%s.%s", key, key2), archive_name, found2, expected2)
							valid = false
						end
					end
				elseif (found ~= expected) then
					verify_error_type(key, archive_name, found, expected)
					valid = false
				end
			end
		else
			print("Archive '" .. archive_name .. "' is '" .. type(archive) .. "', should be 'table'")
		end
	end

	return valid
end

return helper
