local mymodule = {}

local archive_template = {
	path = "string",
	author = "string",
	description = "string",
	version = "number",
	assets = {{ "string", "number" }},
	dependencies = { "number" }
}

local function verify_error_type(name, archive_name, found, expected)
	print(string.format("'%s' in archive '%s' has type '%s', expected '%s'", name, archive_name, found, expected))
end
local function verify_error_missing(archive_name, expected, name)
	print(string.format("Archive '%s' is missing '%s' '%s'", archive_name, expected, name))
end

-- @todo Refactor this, maybe using recursion
local function verify(archives)
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
			print(string.format("Archive '%s' is '%s', should be 'table'", archive_name, type(archive)))
		end
	end

	return valid
end

function mymodule.build (archives)
	if (verify(archives)) then
		for archive,archive_data in pairs(archives) do
			local archive = Archive.new(open_new_file(archive_data.path), archive, archive_data.author, archive_data.description, archive_data.version)

			for dependency,version in pairs(archive_data.dependencies) do
				archive:add_dependency(dependency, version)
			end

			archive:initialize()

			for asset,asset_data in pairs(archive_data.assets) do
				archive:add(Asset.new(asset, open_file(asset_data[1]), asset_data[2]))
			end

			archive:finalize(load_private_key("priv.key"))
		end
	else
		print("ERROR")
	end
end

return mymodule
