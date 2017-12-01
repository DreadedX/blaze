premake5 --platform=web

FILES=build/web/*
for f in $FILES; do
  sed -i 's/-rcs/rcs/' $f
done

$EMSCRIPTEN/emmake make -C build/web -j10
mv build/web/bin/Debug/game build/web/bin/Debug/game.bc
$EMSCRIPTEN/emcc build/web/bin/Debug/game.bc -o build/game.js -g --embed-file build/archives/base.flm --embed-file build/archives/my_first_mod.flm
