local lua_helper = require "lua.helper" 

local builder = {}

-- @todo Improve this
local archive_template = {
	path = "string",
	compression = "number",
	author = "string",
	description = "string",
	version = "number",
	key = "string",
	assets = {{ "string", "number" }},
	dependencies = { "number" }
}

function builder.build (archives)
	if (lua_helper.verify(archives, archive_template)) then
		for archive_name,archive_config in pairs(archives) do
			file = helper.open_new_file(archive_config.path)
			local archive = flame.ArchiveWriter.new(archive_name, file, archive_config.author, archive_config.description, archive_config.version, archive_config.compression)

			for dependency,version in pairs(archive_config.dependencies) do
				archive:add_dependency(dependency, version)
			end

			archive:initialize()

			for asset,asset_data in pairs(archive_config.assets) do
				workflow = flame.Workflow.new()
				tasks = asset_data["tasks"];
				if tasks ~= nil then
					for _, task in ipairs(tasks) do
						workflow.tasks:add(task);
					end
				end
				local asset = flame.MetaAsset.new(asset, asset_data[1], asset_data[2], workflow)
				archive:add(asset)
			end

			print(archive_config.key)
			archive:finalize(helper.load_private_key(archive_config.key))

			file:close()
		end
	else
		print("ERROR")
	end
end

return builder
