premake5 --platform=web

FILES=build/web/*
for f in $FILES; do
  sed -i 's/-rcs/rcs/' $f
done

$EMSCRIPTEN/emmake make -C build/web -j10 config=release
mv build/web/bin/Release/game build/web/bin/Release/game.bc
$EMSCRIPTEN/emcc build/web/bin/Release/game.bc -o build/game.js -O2 -g --embed-file build/archives/base.flm --embed-file build/archives/my_first_mod.flm
