
all: raveler

.PHONY:
checkDepends:
	@bash -c 'if [ "`which Magick++-config`" == "" ]; then \
		echo -e "\nImageMagick library not found." ; \
		echo -e "On Debian/Ubuntu, try:" ; \
		echo -e " sudo apt install graphicsmagick-libmagick-dev-compat\n"; \
		exit 1 ; fi'

raveler: ravelcli.cc ravelcli.h libraveler.cc libraveler.h checkDepends
	c++ -o "$@" "$<" `Magick++-config --cxxflags --cppflags --ldflags --libs`


volcano3.gray: volcano3.jpg
	convert "$<" -gravity center -extent 1:1 -resize 600 -size 600x600 -depth 8 GRAY:- > "$@"

raveler.html: raveljs.cc raveljs.h libraveler.cc libraveler.h volcano3.gray
	emcc "$<" -o raveler.html -s WASM=1 --preload-file volcano3.gray


