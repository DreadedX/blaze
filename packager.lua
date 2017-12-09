-- ARCHIVE BUILDER
require "scripts.builder"

-- TASKS
local langpack = require "scripts.langpack"
local objectlist = require "scripts.objectlist"

local base_objects = { "base/object/bla", "base/object/something" }

archive "base"
	author "Dreaded_X"
	description "This archive contains the base game"
	key "build/keys/test.priv"
	compression(flame.Compression.none)

	script "assets/base/script/Script.lua"

	asset "base/language/Dutch"
		path "assets/base/language/Dutch.lang"
		tasks { langpack }

	asset "base/language/English"
		path "assets/base/language/Dutch.lang"
		tasks { langpack }

	asset "base/Objects"
		path "assets/base/Objects.placeholder"
		tasks { objectlist(base_objects) }

archive "my_first_mod"
	author "Dreaded_X"
	description "My first mod!"
	key "build/keys/unofficial.priv"
	compression(flame.Compression.none)
	version(3)

	script "assets/my_first_mod/script/Script.lua"

	dependency "base"
		version(1)

	asset "my_first_mod/script/Hello"
		path "assets/my_first_mod/script/Hello.lua"

	asset "base/language/Dutch"
		-- @todo Actually change the lang file
		path "assets/base/language/Dutch.lang"
		version(2)

build()

