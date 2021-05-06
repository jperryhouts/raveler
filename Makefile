
.PHONY: all cli wasm clean checkMagick checkEmscripten

all: cli wasm

cli: build/cli/raveler

wasm: build/wasm/raveler.html

clean:
	rm -rf build/wasm build/cli

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

build/%.gray: data/%.jpg
	convert "$<" -gravity center -extent 1:1 -resize 600 -size 600x600 -depth 8 GRAY:- > "$@"

build/cli/raveler: src/ravelcli.cc include/ravelcli.h src/libraveler.cc include/libraveler.h
	@make checkMagick
	mkdir -p `dirname "$@"`
	c++ -o "$@" "src/libraveler.cc" "src/ravelcli.cc" -I./include `Magick++-config --cxxflags --cppflags --ldflags --libs`

build/wasm/raveler.html: src/raveljs.cc include/raveljs.h src/libraveler.cc include/libraveler.h
	@make checkEmscripten
	mkdir -p `dirname "$@"`
	em++ -o "$@" "src/libraveler.cc" "src/raveljs.cc" -I./include \
		-s WASM=1 -s INITIAL_MEMORY=402653184 \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' \
		-s EXPORTED_FUNCTIONS='["_init","_ravel"]' \
		-s NO_EXIT_RUNTIME=1 \
		-s ASYNCIFY -O3 --closure 1
