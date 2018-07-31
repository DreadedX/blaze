local function generate(bins, path)
	local makefile = ""

	-- makefile = makefile .. "CXX=g++\n"
	-- makefile = makefile .. "AR=ar\n"
	-- makefile = makefile .. "LD=ld\n"

	makefile = makefile .. "PROJECTS :="

	for _, bin in ipairs(bins) do
		makefile = makefile .. " " .. bin.name
	end

	makefile = makefile .. "\n\n"
	makefile = makefile .. ".PHONY: all $(PROJECTS)\n\n"
	makefile = makefile .. "all: $(PROJECTS)\n\n"

	helper.create_directory(path .. "/obj");
	
	for i, bin in ipairs(bins) do
		for _, dependency in ipairs(bin.dependencies) do
			for _, module in ipairs(bins) do
				if (module.name == dependency) then
					for _, mod_dependency in ipairs(module.dependencies) do
						local exists = false
						for _, cur_dependecy in ipairs(bin.dependencies) do
							if (mod_dependency == cur_dependecy) then
								exists = true
							end
						end
						if not exists then
							table.insert(bin.dependencies, mod_dependency)
						end
					end
				end
			end
		end

		makefile = makefile .. bin.name .. ":"
		-- @todo Is this really needed, probably for parallel
		for i, dependency in ipairs(bin.dependencies) do
			makefile = makefile .. " " .. dependency
		end
		makefile = makefile .. '\n'
		makefile = makefile .. "\t@${MAKE} --no-print-directory -s -C . -f " .. bin.name .. ".make " .. bin.name .. "\n"

		local subfile = ""

		local objdir = helper.native_path(path .. "/obj/" .. bin.name)
		helper.create_directory(objdir);
		subfile = subfile .. "OBJDIR := " .. objdir .. "\n"
		subfile = subfile .. "OBJ :="
		for _, file in ipairs(bin.src) do
			subfile = subfile .. " $(OBJDIR)/" .. helper.change_extension(file, ".o")
		end
		subfile = subfile .. "\n\n"

		if bin.type == "executable" then
			subfile = subfile .. "DEPS :="
			for _, dependency in ipairs(bin.dependencies) do
				for _, module  in ipairs(bins) do
					if (module.name == dependency) then
						subfile = subfile .. " " .. module.path
					end
				end
			end
			subfile = subfile .. "\n\n"
		end

		subfile = subfile .. "INCLUDES +="
		for _, include in ipairs(bin.include) do
			subfile = subfile .. " -I" .. include
		end
		for _, dependency in ipairs(bin.dependencies) do
			for _, module  in ipairs(bins) do
				for _, include in ipairs(module.include) do
					if (module.name == dependency) then
						subfile = subfile .. " -I" .. include
					end
				end
			end
		end
		subfile = subfile .. "\n\n"

		subfile = subfile .. "DEFINES := -DDEBUG -DLUA_USE_LINUX\n"
		subfile = subfile .. "CPPFLAGS := -std=c++17\n"
		subfile = subfile .. "ALL_CPPFLAGS := $(CPPFLAGS) $(INCLUDES) $(DEFINES)\n"
		subfile = subfile .. "CFLAGS := -std=c11\n"
		subfile = subfile .. "ALL_CFLAGS := $(CFLAGS) $(INCLUDES) $(DEFINES)\n"
		if bin.type == "executable" then
			subfile = subfile .. "LDFLAGS :="
			for _, link in ipairs(bin.links) do
				subfile = subfile .. " -l" .. link
			end
			for _, dependency in ipairs(bin.dependencies) do
				for _, module  in ipairs(bins) do
					for _, link in ipairs(module.links) do
						if (module.name == dependency) then
							subfile = subfile .. " -l" .. link
						end
					end
				end
			end
		end
		subfile = subfile .. "\n"

		for j, file in ipairs(bin.src) do
			subfile = subfile .. "$(OBJDIR)/" .. helper.change_extension(file, ".o") .. ": " .. file .. "\n"
			subfile = subfile .. "\t@echo -e \"\\033[93mCompiling     \\033[39m" .. string.sub(file, #helper.native_path("") + 2) .. "\"\n"
			if helper.get_extension(file) == ".c" then
				subfile = subfile .. "\t$(CC) -c -o $@ $< $(ALL_CFLAGS)\n"
			else
				subfile = subfile .. "\t$(CXX) -c -o $@ $< $(ALL_CPPFLAGS)\n"
			end
		end

		if bin.type == "lib" then
			subfile = subfile .. "\n"
			subfile = subfile .. bin.name .. ": $(OBJ)\n"
			subfile = subfile .. "\t@echo -e \"\\033[92mLinking       \\033[39m" .. bin.name .. "\"\n"
			subfile = subfile .. "\t$(AR) rcs " .. bin.path .. " $^"
		else
			subfile = subfile .. "\n"
			subfile = subfile .. bin.name .. ": $(OBJ) $(DEPS)\n"
			subfile = subfile .. "\t@echo -e \"\\033[92mLinking       \\033[39m" .. bin.name .. "\"\n"
			-- @todo Figure out how exactly we are going to do -pthread and -lstdc++fs
			subfile = subfile .. "\t$(CXX) -o " .. bin.path .. " $^ $(LDFLAGS) -pthread"
		end

		local file = io.open("build/linux-test/" .. bin.name .. ".make", "w+")
		io.output(file)
		io.write(subfile)
		file:close()

		makefile = makefile .. "\n"
	end

	local file = io.open("build/linux-test/Makefile", "w+")
	io.output(file)
	io.write(makefile)
	file:close()
end

local function build() 
	os.execute("make -C build/linux-test")
end

return {
	generate = generate,
	build = build
}
