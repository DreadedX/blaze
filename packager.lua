local builder = require "lua.builder"

priv_key = "keys/test.priv"

-- @todo Figure out how to actually process data here
function test(input)
	for i=1,10 do
		print(input[i])
	end
	output = helper.new_byte_vector()
	output[1] = 0
	output[2] = 1

	return input
end

-- @todo This needs to be platform independent
langpack = helper.get_external_task("build/langpack/bin/Linux/Debug/liblangpack.so")

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
			LuaTest = { "assets/test.lua", 1, tasks = { test } },
			-- Language packs
			Dutch = { "assets/lang/nl.lang", 1, tasks = { langpack } },
			English = { "assets/lang/en.lang", 1, tasks = { langpack } },
		},
		dependencies = {
			base = 1
		}
	}
})

-- @todo Add support for folder, this will propably have to come from within blaze
prototype = {
	test = {
		path = "archives/test.flm",
		author = "Dreaded_X",
		description = "This is the first archive being made using the new scripting stuff",
		version = 1,
		key = "keys/unofficial.priv",
		assets = {
			folder = {
				-- Will end up in folder.LuaTest
				LuaTest = { "assets/test.lua", 1, tasks = { test } },
			},
			Languages = {
				-- Will end up in Languages.Dutch
				Dutch = { "assets/lang/nl.lang", 1, tasks = { langpack } },
				-- Will end up in Languages.English
				English = { "assets/lang/en.lang", 1, tasks = { langpack } },
			}
		},
		dependencies = {
			base = 1
		}
	}
}


-- Just some testing

function print_archive_info(archive)
	print("======ARCHIVE======")
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

	local meta_assets = archive:get_meta_assets()
	if (meta_assets:size() > 0) then
		print("Meta Assets: ")
		for _,meta_asset in pairs(meta_assets) do
			print(string.format(" %s (%i)", meta_asset:get_name(), meta_asset:get_version()))
			print("Content: \n")
			debug_content(meta_asset:get_data(Workflow.new()))
		end
	end
end

-- local base = Archive.new(open_file("archives/base.flm"))
-- print_archive_info(base);
-- local test = Archive.new(open_file("archives/test.flm"))
-- print_archive_info(test);
