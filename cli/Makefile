
all: img2thread

.PHONY:
checkDepends:
	@bash -c 'if [ "`which Magick++-config`" == "" ]; then \
		echo -e "\nImageMagick library not found." ; \
		echo -e "On Debian/Ubuntu, try:" ; \
		echo -e " sudo apt install graphicsmagick-libmagick-dev-compat\n"; \
		exit 1 ; fi'

img2thread: img2thread.cc img2thread.h checkDepends
	c++ -o "$@" "$<" `Magick++-config --cxxflags --cppflags --ldflags --libs`

