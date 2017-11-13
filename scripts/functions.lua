function defaultAction(os_name, action_name) 
	if os.ishost(os_name) then
		_ACTION = _ACTION or action_name
	end
end

function includeThreads() 
	if _ACTION == "androidmk" then
	else
		links "pthread"
	end
end
function includePlatform()
	if _ACTION == "androidmk" then
		links "log"
	end
end
-- @todo We should only build when zlib is not available on system
function includeZlib()
	links "z"
end
function includeCryptoPP()
	includedirs "third_party/cryptopp"
	filter "kind:not StaticLib"
		links "cryptopp"
	filter {}
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
function includeSol2()
	includedirs "third_party/sol2"
	includeLua()
end
function includeGenerated()
	includedirs "modules/generated/include"
	filter "kind:not StaticLib"
		links "generated"
	filter {}
end
function includeCrypto()
	includedirs "modules/crypto/include"
	filter "kind:not StaticLib"
		links "crypto"
	filter {}
	includeBigInt()
end
function includeFlame()
	includedirs "modules/flame/include"
	filter "kind:not StaticLib"
		links "flame"
	filter {}
	includeCryptoPP()
	includeCrypto()
	includeZlib()
	includeThreads()
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
