-- @todo Add support for folder, this will propably have to come from within blaze
-- @todo Add error handler that tells us on which file the error happend

local inspect = require "scripts.inspect"

local archives = {}
local bins = {}

local parent = nil
local context = nil

local dist_files = {}

local platform = {
	name = "linux-test",
	static = ".a",
	executable = ""
}

-- @todo Assert that all the provided types are correct
-- @todo Make sure we can't have duplicates
function archive(name) 
	parent = {type = "archive", name = name, author = "(unknown)", description = "(none)", version = 1, path = helper.native_path("build/archives/" .. name .. ".flm"), compression = flame.Compression.zlib, key = nil, dependencies = {}, assets = {}}
	table.insert(archives, parent)
	context = parent
end

function lib(name)
	helper.create_directory("build")
	helper.create_directory("build/" .. platform.name)
	helper.create_directory("build/" .. platform.name .. "/static")

	parent = {type = "lib", name = name, path = helper.native_path("build/" .. platform.name .. "/static/" .. name .. platform.static), src = {}, include = {}, links = {}, dependencies = {}}
	table.insert(bins, parent)
	context = parent
end

function executable(name)
	helper.create_directory("build")
	helper.create_directory("build/" .. platform.name)
	helper.create_directory("build/" .. platform.name .. "/bin")

	parent = {type = "executable", name = name, path = helper.native_path("build/" .. platform.name .. "/bin/" .. name .. platform.executable), src = {}, include = {}, links = {}, dependencies = {}}
	table.insert(bins, parent)
	context = parent
end

function author(author)
	parent.author = author
end

function description(description)
	parent.description = description
end

function version_min(version)
	context.version_min = version
end

function version_max(version)
	context.version_max = version
end

function version(version)
	context.version = version
	-- @todo We only have to do this for dependencies
	version_min(version)
	version_max(version)
end

function path(path)
	if parent.type == "archive" then
		context.path = path
	else 
		src(path .. "/src")
		include(path .. "/include")
	end
end

function compression(compression)
	context.compression = compression
end

function key(key)
	parent.key = key
end

function script(filename)
	asset(parent.name .. "/Script")
	path(filename)
	context = parent
end

function dependency(name)
	if parent.type == "archive" then
		local dependency = {name = name, version_min = 0, version_max = 0}
		table.insert(parent.dependencies, dependency)

		context = dependency
	else
		if type(name) == "table" then
			for _, dep in ipairs(name) do
				dependency(dep)
			end
		else
			table.insert(parent.dependencies, name)
		end
	end
end

function links(name)
	table.insert(parent.links, name)
end

function asset(asset_name)
	local asset = {name = asset_name, path = nil, version = parent.version, tasks = {}, compression = parent.compression}
	table.insert(parent.assets, asset)

	context = asset
end

function tasks(tasks)
	context.tasks = tasks
end

function src(path, recursive)
	if recursive ~= false then
		parent.src = helper.list_files_recursive(path)
	else
		parent.src = helper.list_files(path)
	end
end

function include(path)
	table.insert(parent.include, helper.native_path(path))
end

function exclude(path)
	path = helper.native_path(path)
	for i, file in ipairs(parent.src) do
		if file == path then
			table.remove(parent.src, i)
		end
	end
end

function dist()
	table.insert(dist_files, parent.path)
end

function get_data()
	return archives
end

local make = require "scripts.make"

function generate(system)
	print "Generating..."
	system.generate(bins, "build/" .. platform.name)
	print "Done."
end

function package()
	print "Packaging..."
	for _,archive in ipairs(archives) do
		print("Archive " .. archive.name)
		-- @todo It would be nice if we could just turn archive.dependencies into the correct type
		dependencies = flame.new_dependency_list();
		for _,dependency in ipairs(archive.dependencies) do
			print("Dependency " .. dependency.name)
			dependencies:add(dependency.name, dependency.version_min, dependency.version_max)
		end

		local archive_writer = flame.ArchiveWriter.new(archive.name, archive.path, archive.author, archive.description, archive.version, dependencies)

		for _,asset in ipairs(archive.assets) do
			print("Asset " .. asset.name)
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

		print("Signing " .. archive.name)
		archive_writer:sign(helper.load_private_key(archive.key))
		archive_writer:close()
	end
end

-- @todo We need to check if each stage was successful
function build(system)

	-- @todo Only do this if the packager plugin is loaded (Plugin is also @todo)
	-- @todo Loop over all plugins and run them
	package()

	generate(system)

	print "Building..."
	system.build()
	print "Done."

	-- @todo Copy all dist files to the dist folder
	print "Copying..."
	helper.create_directory("dist")
	for _, file in ipairs(dist_files) do
		helper.copy(file, "dist")
	end
	print "Done."
end

function run(target)
	if target == nil then
		-- @todo Get the default from a config file
		target = "game"
	end

	-- @todo We need to get from a config file
	local cmd = "cd dist; ./"
	os.execute(cmd .. target)
end
