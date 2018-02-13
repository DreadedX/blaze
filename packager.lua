-- ARCHIVE BUILDER
-- @todo This should be auto included from packager
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
	version(1)

	script "assets/base/script/Script.lua"

	asset "base/language/Dutch"
		path "assets/base/language/Dutch.lang"
		tasks { langpack }

	asset "base/language/English"
		path "assets/base/language/English.lang"
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
		version_min(1)

	asset "my_first_mod/script/Hello"
		path "assets/my_first_mod/script/Hello.lua"

	asset "base/language/Dutch"
		path "assets/my_first_mod/language/Dutch.lang"
		tasks { langpack }
		version(10)

-- @todo This should automatically be called from packager
build()

-- @todo Eventually we are going create our own build system so we can use one file to build everything
-- src for .cpp files and include voor .h files
-- lib "flame"
-- 	path "modules/flame"
--
-- lib "blaze"
-- 	path "modules/blaze"
-- 	include "flame"
--
-- executable "game"
-- 	path "game"
-- 	-- This auto includes everything that blaze includes
-- 	include "blaze"

	args "test"
