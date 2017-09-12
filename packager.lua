local function verify (archive)
	verified = true
	if (archive.path == nil) then
		print "Archive is missing a path"
		verified = false
	end
	if (archive.author == nil) then
		print "Archive is missing an author"
		verified = false
	end
	if (archive.description == nil) then
		print "Archive is missing a description"
		verified = false
	end
	if (archive.version == nil) then
		print "Archive is missing a version"
		verified = false
	end

	for asset,asset_data in pairs(archive.assets) do
		if (asset_data[1] == nil) then
			print(string.format("Archive asset '%s' is missing a path", asset))
			verified = false
		end
		if (asset_data[2] == nil) then
			print(string.format("Archive asset '%s' is missing a version", asset))
			verified = false
		end
	end

	-- @todo Check type info or something maybe
	-- for dependency,version in pairs(archive.assets) do
	-- end

	return verified
end

function build (archives)
	for archive,archive_data in pairs(archives) do
		if (verify(archive_data)) then
			local archive = Archive.new(open_new_file(archive_data.path), archive, archive_data.author, archive_data.description, archive_data.version)

			for dependency,version in pairs(archive_data.dependencies) do
				archive:add_dependency(dependency, version)
			end

			archive:initialize()

			for asset,asset_data in pairs(archive_data.assets) do
				archive:add(Asset.new(asset, open_file(asset_data[1]), asset_data[2]))
			end

			archive:finalize(load_private_key("priv.key"))
		else
			print(string.format("Skipping archive '%s'", archive))
		end
	end
end

build({
	lua = {
		path = "test.flm",
		author = "Dreaded_X",
		description = "This is the first archive being made using the new scripting stuff",
		version = 1,
		assets = {
			LuaAsset = { "assets/lua.txt", 1 },
			TestAsset = { "assets/test.txt", 3}
		},
		dependencies = {
			base = 1
		}
	}
})
