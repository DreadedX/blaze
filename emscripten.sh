$EMSCRIPTEN/emmake make -C build.web -j10
mv build.web/bin/Linux/Debug/game build.web/bin/Linux/Debug/game.bc
$EMSCRIPTEN/emcc build.web/bin/Linux/Debug/game.bc -o web.html -g --embed-file archives/base.flm --embed-file archives/my_first_mod.flm
# zip web.zip web.html web.js archives/base.flm archives/my_first_mod.flm
# cp web.zip ~/Documenten/Dropbox
