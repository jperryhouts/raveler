
.PHONY: all cli web clean checkMagick checkEmscripten

all: cli web

cli: build/cli/raveler

web: build/web/raveler.html

clean:
	rm -f raveler.data raveler.html raveler.js raveler.wasm

checkMagick:
	@bash -c 'if [ "`which Magick++-config`" == "" ]; then \
		echo -e "\nMagick++ library not found." ; \
		echo -e "On Debian/Ubuntu, try:" ; \
		echo -e " sudo apt install graphicsmagick-libmagick-dev-compat\n"; \
		exit 1 ; fi'

checkEmscripten:
	@bash -c 'if [ "`which em++`" == "" ]; then \
		echo -e "\nEnscripten not found." ; \
		echo -e "On Debian/Ubuntu, try:" ; \
		echo -e " sudo apt install emscripten\n"; \
		exit 1 ; fi'

build/cli/raveler: ravelcli.cc ravelcli.h libraveler.cc libraveler.h checkMagick
	mkdir -p build/cli
	c++ "libraveler.cc" "ravelcli.cc" -o "$@" `Magick++-config --cxxflags --cppflags --ldflags --libs`

%.gray: %.jpg
	convert "$<" -gravity center -extent 1:1 -resize 600 -size 600x600 -depth 8 GRAY:- > "$@"

build/web/raveler.html: raveljs.cc raveljs.h libraveler.cc libraveler.h volcano3.gray checkEmscripten
	mkdir -p build/web
	em++ "libraveler.cc" "raveljs.cc" -o "$@" \
		--preload-file volcano3.gray \
		-s WASM=1 -s INITIAL_MEMORY=402653184 \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' \
		-s EXPORTED_FUNCTIONS='["_init","_ravel","_free_mem"]' \
		-s NO_EXIT_RUNTIME=1 \
		-O3 --closure 1
