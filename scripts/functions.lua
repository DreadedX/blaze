function defaultAction(os_name, action_name) 
	if os.ishost(os_name) then
		_ACTION = _ACTION or action_name
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
function includeFlame()
	includedirs "modules/flame/include"
	filter "kind:not StaticLib"
		links "flame"
	filter {}
	includeCryptoPP()
	includeZlib()
	-- @todo This depends on the platform
	links "pthread"
end
function includeBlaze()
	includedirs "modules/blaze/include"
	filter "kind:not StaticLib"
		links "blaze"
	filter {}
	includeGenerated()
	includeFlame()
	includeLuaBind()
end
function includeLuaBind()
	includedirs "modules/lua-flame/include"
	filter "kind:not StaticLib"
		links "lua-bind"
	filter {}
	includeFlame()
	includeSol2()
end
