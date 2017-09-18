local builder = require "lua.builder"

priv_key = "keys/test.priv"

builder.build({
	base = {
		path = "archives/base.flm",
		author = "Dreaded_X",
		description = "This archive contains all the required assets for the game engine to function",
		version = 1,
		key = priv_key,
		assets = {},
		dependencies = {}
	},
	test = {
		path = "archives/test.flm",
		author = "Dreaded_X",
		description = "This is the first archive being made using the new scripting stuff",
		version = 1,
		key = "keys/unofficial.priv",
		assets = {
			LuaAsset = { "assets/lua.txt", 1 },
			TestAsset = { "assets/test.txt", 3}
		},
		dependencies = {
			base = 1
		}
	}
})

-- Just some testing

function print_archive_info(archive)
	print("======ARCHIVE======")
	print(string.format("Valid: %s", archive:is_valid()))
	print(string.format("Name: %s", archive:get_name()))
	print(string.format("Author: %s", archive:get_author()))
	print(string.format("Description: %s", archive:get_description()))
	print(string.format("Version: %s", archive:get_version()))
	print(string.format("Official: %s", archive:is_trusted(get_trusted_key())))

	local dependencies = archive:get_dependencies()
	if (dependencies:size() > 0) then
		print("Dependencies: ")
		for _,dependency,version in pairs(dependencies) do
			print(string.format(" %s == %i", dependency, version))
		end
	end

	local assets = archive:get_assets()
	if (assets:size() > 0) then
		print("Assets: ")
		for _,asset in pairs(assets) do
			print(string.format(" %s (%i)", asset:get_name(), asset:get_version()))
			print("Content: \n")
			debug_content(asset:get_data())
		end
	end
end

local base = Archive.new(open_file("archives/base.flm"))
print_archive_info(base);
local test = Archive.new(open_file("archives/test.flm"))
print_archive_info(test);
