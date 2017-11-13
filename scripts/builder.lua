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
			-- @todo It would be nice if we could just turn archive.dependencies into the correct type
			dependencies = flame.new_dependency_list();
			for _,dependency in ipairs(archive.dependencies) do
				dependencies:add(dependency.name, dependency.version)
			end

			local archive_writer = flame.ArchiveWriter.new(archive.name, archive.path, archive.author, archive.description, archive.version, archive.compression, dependencies)

			for _,asset in ipairs(archive.assets) do
				workflow = flame.new_workflow()
				tasks = asset.tasks;
				if tasks ~= nil then
					for _, task in ipairs(tasks) do
						workflow:add(task);
					end
				end

				local meta_asset = flame.MetaAsset.new(asset.name, asset.path, asset.version, workflow)
				archive_writer:add(meta_asset)
			end

			archive_writer:sign(helper.load_private_key(archive.key))
			archive_writer:close()
		end
	-- else
	-- 	print("ERROR")
	-- end
end

return builder
