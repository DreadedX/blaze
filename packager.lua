plugin "packager@blaze"
plugin "langpack@blaze"
plugin "glsl@blaze"
plugin "image@blaze"
plugin "obj@blaze"

meta "archives"
	dependency("base", "my_first_mod")

archive "base"
	author "Dreaded_X"
	description "This archive contains the base game"
	-- key "build/keys/test.priv"
	-- key "keys/official.pem"
	-- compression(flame.Compression.none)
	compression(0)
	version(1)

	script "assets/base/script/Script.lua"

	asset "base/language/Dutch"
		path "assets/base/language/Dutch.lang"
		task (langpack.parser)

	asset "base/language/English"
		path "assets/base/language/English.lang"
		task (langpack.parser)

	-- @todo Add task for compiling shaders
	asset "base/shader/Vertex"
		path "assets/base/shader/triangle.vert"
		task(shader.compiler)

	asset "base/shader/Fragment"
		path "assets/base/shader/triangle.frag"
		task(shader.compiler)

	asset "base/texture/Test"
		path "assets/base/texture/test.png"
		task(image.load)

	asset "base/model/Chalet"
		path "assets/base/model/chalet.obj"
		task(obj.load)

	asset "base/texture/Chalet"
		path "assets/base/texture/chalet.jpg"
		task(image.load)

archive "my_first_mod"
	author "Dreaded_X"
	description "My first mod!"
	-- key "build/keys/unofficial.priv"
	-- key "keys/official.pem"
	-- compression(flame.Compression.none)
	compression(0)
	version(3)

	script "assets/my_first_mod/script/Script.lua"

	-- @todo This overrides an existing command
	requires "base"
		version_min (1)

	asset "my_first_mod/script/Hello"
		path "assets/my_first_mod/script/Hello.lua"

	asset "base/language/Dutch"
		path "assets/my_first_mod/language/Dutch.lang"
		task (langpack.parser)
		version(10)
