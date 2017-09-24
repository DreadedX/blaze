local helper = require "lua.helper" 

local builder = {}

local archive_template = {
	path = "string",
	author = "string",
	description = "string",
	version = "number",
	key = "string",
	assets = {{ "string", "number" }},
	dependencies = { "number" }
}

function builder.build (archives)
	if (helper.verify(archives, archive_template)) then
		for archive_name,archive_data in pairs(archives) do
			local file = open_new_file(archive_data.path)
			local archive = Archive.new(file, archive_name, archive_data.author, archive_data.description, archive_data.version)

			for dependency,version in pairs(archive_data.dependencies) do
				archive:add_dependency(dependency, version)
			end

			archive:initialize()

			for asset,asset_data in pairs(archive_data.assets) do
				local asset = Asset.new(asset, open_file(asset_data[1]), asset_data[2], Workflow.new())
				archive:add(asset)
			end

			print(archive_data.key)
			archive:finalize(load_private_key(archive_data.key))
			file:close()
		end
	else
		print("ERROR")
	end
end

return builder
