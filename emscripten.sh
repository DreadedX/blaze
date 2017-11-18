$EMSCRIPTEN/emmake make -C build.web -j10
mv build.web/bin/Linux/Debug/game build.web/bin/Linux/Debug/game.bc
$EMSCRIPTEN/emcc build.web/bin/Linux/Debug/game.bc -o web.html -g --preload-file archives/base.flm --preload-file archives/my_first_mod.flm
