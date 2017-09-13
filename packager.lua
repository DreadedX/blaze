local mymodule = require "helper"

-- @todo key = "priv.key"
mymodule.build({
	base = {
		path = "archives/base.flm",
		author = "Dreaded_X",
		description = "This archive contains all the required assets for the game engine to function",
		version = 1,
		assets = {},
		dependencies = {}
	},
	lua = {
		path = "archives/test.flm",
		author = "Dreaded_X",
		description = "This is the first archive being made using the new scripting stuff",
		version = 1,
		assets = {
			LuaAsset = { "assets/lua.txt", 1 },
			TestAsset = { "assets/test.txt", 3},
		},
		dependencies = {
			base = 1
		}
	},
})
