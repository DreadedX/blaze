local helper = require "lua.helper" 

local builder = {}

-- @todo Improve this
local archive_template = {
	path = "string",
	author = "string",
	description = "string",
	version = "number",
	key = "string",
	assets = {{ "string", "number", "string" }},
	dependencies = { "number" }
}

function builder.build (archives)
	if (helper.verify(archives, archive_template)) then
		for archive_name,archive_config in pairs(archives) do
			local archive = ArchiveWriter.new(archive_name, archive_config.path, archive_config.author, archive_config.description, archive_config.version)

			for dependency,version in pairs(archive_config.dependencies) do
				archive:add_dependency(dependency, version)
			end

			archive:initialize()

			for asset,asset_data in pairs(archive_config.assets) do
				workflow = Workflow.new()
				task = asset_data[3];
				if task ~= "" then
					workflow.tasks:add(get_external_task("build/langpack/bin/Linux/Debug/liblangpack.so"));
				end
				local asset = MetaAsset.new(asset, asset_data[1], asset_data[2], workflow)
				archive:add(asset)
			end

			print(archive_config.key)
			archive:finalize(load_private_key(archive_config.key))
		end
	else
		print("ERROR")
	end
end

return builder
