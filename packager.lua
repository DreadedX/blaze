local builder = require "scripts.builder"
local langpack = require "scripts.langpack"

priv_key = "build/keys/test.priv"

builder.build({
	{
		name = "base",
		path = "build/archives/base.flm",
		compression = flame.Compression.none,
		author = "Dreaded_X",
		description = "This archive contains all the required assets for the game engine to function",
		version = 1,
		key = priv_key,
		assets = {
			{ name = "base/Script", path = "assets/base/script/Script.lua", version = 1, tasks = { } },
			{ name = "base/language/Dutch", path = "assets/base/language/Dutch.lang", version = 1, tasks = { langpack } },
			{ name = "base/language/English", path = "assets/base/language/English.lang", version = 1, tasks = { langpack } },
		},
		dependencies = { }
	},
	{
		name = "my_first_mod",
		path = "build/archives/my_first_mod.flm",
		compression = flame.Compression.none,
		author = "ModAuthor",
		description = "My first mod!",
		version = 1,
		key = "build/keys/unofficial.priv",
		assets = {
			{ name = "my_first_mod/Script", path = "assets/my_first_mod/script/Script.lua", version = 1, tasks = { } },
			{ name = "my_first_mod/script/Hello", path = "assets/my_first_mod/script/Hello.lua", version = 1, tasks = { } },
		},
		dependencies = {
			{name = "base", version = 1}
		}
	}
})

-- Just some testing

function print_archive_info(archive)
	print("======ARCHIVE======")
	print("Name: " .. archive.name)
	print("Author: " .. archive.author)
	print("Description: " .. archive.description)
	print("Version: " .. archive.version)
	print("Official: " .. tostring(archive:is_trusted(helper.get_trusted_key())))

	local dependencies = archive.dependencies
	if (dependencies:size() > 0) then
		print("Dependencies: ")
		for _,dependency,version in pairs(dependencies) do
			print(' ' .. dependency .. " == " .. version)
		end
	end

	-- local meta_assets = archive.meta_assets
	-- if (meta_assets:size() > 0) then
	-- 	print("Meta Assets: ")
	-- 	for _,meta_asset in pairs(meta_assets) do
	-- 		print("===" .. meta_asset.name .. '=(' .. meta_asset.version .. ")===")
	-- 		helper.debug_content(meta_asset:get_data(flame.new_workflow()))
	-- 	end
	-- end
end

print_archive_info(flame.Archive.new("build/archives/base.flm"));
print_archive_info(flame.Archive.new("build/archives/my_first_mod.flm"));

--------------------------------------------------------------------------------------------------------

-- @todo Add support for folder, this will propably have to come from within blaze
-- @todo Add error handler that tells us on which file the error happend
prototype = {
	test = {
		path = "archives/test.flm",
		author = "Dreaded_X",
		description = "This is the first archive being made using the new scripting stuff",
		version = 1,
		key = "keys/unofficial.priv",
		assets = {
			script = {
				-- Will end up in folder.LuaTest
				LuaTest = { "assets/script/Test.lua", 1, tasks = { } },
			},
			language = {
				-- Will end up in Languages.Dutch
				Dutch = { "assets/language/Dutch.lang", 1, tasks = { langpack } },
				-- Will end up in Languages.English
				English = { "assets/language/en.lang", 1, tasks = { langpack } },
			}
		},
		dependencies = {
			base = 1
		}
	}
}
