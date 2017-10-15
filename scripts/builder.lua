local lua_helper = require "scripts.helper" 

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
	-- @todo We need to first make the verify thing better
	-- if (lua_helper.verify(archives, archive_template)) then
		for _,archive in ipairs(archives) do
			file = helper.open_new_file(archive.path)
			local archive_writer = flame.ArchiveWriter.new(archive.name, file, archive.author, archive.description, archive.version, archive.compression)

			for _,dependency in ipairs(archive.dependencies) do
				archive_writer:add_dependency(dependency.name, dependency.version)
			end

			archive_writer:initialize()

			for _,asset in ipairs(archive.assets) do
				workflow = flame.Workflow.new()
				tasks = asset.tasks;
				if tasks ~= nil then
					for _, task in ipairs(tasks) do
						workflow.tasks:add(task);
					end
				end

				local meta_asset = flame.MetaAsset.new(asset.name, asset.path, asset.version, workflow)
				archive_writer:add(meta_asset)
			end

			archive_writer:finalize(helper.load_private_key(archive.key))

			file:close()
		end
	-- else
	-- 	print("ERROR")
	-- end
end

return builder
