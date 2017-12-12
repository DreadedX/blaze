function includePlatform()
	filter "options:platform=android"
		links "android"
		links "log"
		includedirs "modules/blaze/platform/android/include"
		includedirs "modules/blaze/include"
	filter "options:platform=linux"
		links "pthread"
	filter {}
end
-- @todo We should only build when zlib is not available on system
function includeZlib()
	links "z"
end
function includeBigInt()
	includedirs "third_party/bigint"
	filter "kind:not StaticLib"
		links "bigint"
	filter {}
end
-- @todo We should only build when lua is not available on system
function includeLua()
	includedirs "third_party/lua"
	filter "kind:not StaticLib"
		links "lua"
	filter {}
end
function includeFmt()
	includedirs "third_party/fmt"
	filter "kind:not StaticLib"
		links "fmt"
	filter {}
end
function includeSol2()
	includedirs "third_party/sol2"
	includeLua()
end
function includeGenerated()
	includedirs "modules/generated/include"
	filter "kind:not StaticLib"
		links "generated"
	filter {}
	includeCrypto()
end
function includeCrypto()
	includedirs "modules/crypto/include"
	filter "kind:not StaticLib"
		links "crypto"
	filter {}
	includeBigInt()
	includeLogger()
end
function includeLogger()
	includedirs "modules/logger/include"
	filter "kind:not StaticLib"
		links "logger"
	filter {}
	includeFmt()
	includePlatform()
end
function includeFlame()
	includedirs "modules/flame/include"
	filter "kind:not StaticLib"
		links "flame"
	filter {}
	includeCrypto()
	includeZlib()

	includePlatform()
end
function includeBlaze()
	includedirs "modules/blaze/include"
	filter "kind:not StaticLib"
		links "blaze"
	filter {}
	includeGenerated()
	includeLuaBind()
	includeFlame()
end
function includeLuaBind()
	includedirs "modules/lua-bind/include"
	filter "kind:not StaticLib"
		links "lua-bind"
	filter {}
	includeSol2()
end
