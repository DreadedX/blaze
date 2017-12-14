-- @todo Add support for folder, this will propably have to come from within blaze
-- @todo Add error handler that tells us on which file the error happend

local archives = {}
local current = nil
local last = nil

-- @todo Assert that all the provided types are correct
function archive(name) 
	current = {name = nil, author = "(unknown)", description = "(none)", version = 1, path = "build/archives/" .. name .. ".flm", compression = flame.Compression.zlib, key = nil, dependencies = {}, assets = {}}
	table.insert(archives, current)
	last = current
	current.name = name
end

function author(author)
	current.author = author
end

function description(description)
	current.description = description
end

function version(version)
	last.version = version
end

function path(path)
	last.path = path
end

function compression(compression)
	last.compression = compression
end

function key(key)
	current.key = key
end

function script(filename)
	asset(current.name .. "/Script")
	path(filename)
	last = current
end

function dependency(dependency_name)
	local dependency = {name = dependency_name, version = 1}
	table.insert(current.dependencies, dependency)

	last = dependency
end

function asset(asset_name)
	local asset = {name = asset_name, path = nil, version = current.version, tasks = {}, compression = current.compression}
	table.insert(current.assets, asset)
	
	last = asset
end

function tasks(tasks)
	last.tasks = tasks
end

function get_data()
	return archives
end

function build()
	for _,archive in ipairs(archives) do
		-- @todo It would be nice if we could just turn archive.dependencies into the correct type
		dependencies = flame.new_dependency_list();
		for _,dependency in ipairs(archive.dependencies) do
			dependencies:add(dependency.name, dependency.version)
		end

		local archive_writer = flame.ArchiveWriter.new(archive.name, archive.path, archive.author, archive.description, archive.version, dependencies)

		for _,asset in ipairs(archive.assets) do
			workflow = flame.new_workflow()
			tasks = asset.tasks;
			if tasks ~= nil then
				for _, task in ipairs(tasks) do
					workflow:add(task);
				end
			end

			local meta_asset = flame.MetaAsset.new(asset.name, asset.path, asset.version, workflow)
			archive_writer:add(meta_asset, asset.compression)
		end

		archive_writer:sign(helper.load_private_key(archive.key))
		archive_writer:close()
	end
end
