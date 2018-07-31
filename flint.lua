-- Load flint plugins
-- plugin "packager"

plugin "plugin_default.so"

run_dir "build/archives"

lib "fmt"
	src "*third_party/fmt/fmt"
	include "third_party/fmt"

lib "bigint"
	src "*third_party/bigint"
	include "third_party/bigint"

lib "lua"
	src("*third_party/lua", "-third_party/lua/lua.c")
	include "third_party/lua"

	if platform.name == "linux" then
		link "dl"
	end

lib "zlib"
	src("*third_party/zlib", "-third_party/zlib/{gzlib.c,gzwrite.c,gzread.c}")
	include "third_party/zlib"

lib "logger"
	path "modules/logger"
	dependency "fmt"

lib "parser"
	path "modules/parser"
	dependency "logger"

lib "crypto"
	path "modules/crypto"
	dependency("logger", "bigint")

lib "generated"
	path "modules/generated"
	dependency "crypto"

lib "flame"
	path "modules/flame"
	dependency("zlib", "crypto")

lib "lua-bind"
	path "modules/lua-bind"
	dependency("lua", "flame")

	include("modules/blaze/include", "third_party/sol2")

lib "blaze"
	path "modules/blaze"
	dependency("lua-bind", "generated")

	include "modules/blaze/platform/android/include"

executable "game"
	path "game"
	dependency "blaze"

default "game"

dist "game"

print(platform.name)
