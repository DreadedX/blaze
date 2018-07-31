-- ARCHIVE BUILDER
-- @todo This should be auto included from packager
-- require "scripts.builder"

-- TASKS
local langpack = require "scripts.langpack"
local objectlist = require "scripts.objectlist"

local base_objects = { "base/object/bla", "base/object/something" }

-- @todo We need to split of all the packager related code into a seperate plugin,
-- that way we can easily build the build tool without having a lot of dependencies

-- plugin('packager')
-- if (plugin.packager) then
archive "base"
	dist()
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
-- end

-- THIS IS FOR OUR BUILD TOOL
-- @todo Maybe warn for duplicate dependecie
-- Third Party
lib "fmt"
	src "third_party/fmt/fmt"
	include "third_party/fmt"

lib "bigint"
	src "third_party/bigint"
	include "third_party/bigint"

	exclude "third_party/bigint/sample.cc"
	exclude "third_party/bigint/testsuite.cc"

lib "lua"
	-- @todo Make this
	-- wget "https://www.lua.org/ftp/lua-5.3.4.tar.gz"
	-- src "third_party/lua/src"
	-- include "third_party/lua/src"
	src "third_party/lua"
	include "third_party/lua"

	exclude "third_party/lua/lua.c"

	links "dl"

lib "zlib"
	-- @todo Make this
	-- git "https://github.com/madler/zlib"
	src("third_party/zlib", false)
	include "third_party/zlib"

-- Modules
lib "logger"
	path "modules/logger"
	dependency "fmt"

lib "parser"
	path "modules/parser"
	dependency "logger"

lib "crypto"
	path "modules/crypto"
	dependency {"logger", "bigint"}

lib "generated"
	path "modules/generated"
	dependency "crypto"

lib "flame"
	path "modules/flame"
	dependency {"zlib", "crypto"}

-- @todo We need to merge the blaze part this into blaze, because now we have a weird dependency cycle
lib "lua-bind"
	path "modules/lua-bind"
	dependency {"lua", "flame"}

	include "modules/blaze/include"
	include "third_party/sol2"

lib "blaze"
	path "modules/blaze"
	dependency {"lua-bind", "generated"}

	-- Include platform headers
	include "modules/blaze/platform/android/include"

executable "game"
	dist()
	path "game"
	dependency "blaze"

	--args "test"

executable "packager"
	path "tools/packager"
	dependency {"lua-bind", "generated"}

	links "stdc++fs"

executable "keygen"
	dist()
	path "tools/keygen"
	-- @todo We only depend on flame for binary_helper so maybe that should be split off
	dependency {"logger", "parser", "crypto", "flame"}
