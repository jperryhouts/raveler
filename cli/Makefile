
all: raveler

.PHONY:
checkDepends:
	@bash -c 'if [ "`which Magick++-config`" == "" ]; then \
		echo -e "\nImageMagick library not found." ; \
		echo -e "On Debian/Ubuntu, try:" ; \
		echo -e " sudo apt install graphicsmagick-libmagick-dev-compat\n"; \
		exit 1 ; fi'

raveler: raveler.cc raveler.h checkDepends
	c++ -o "$@" "$<" `Magick++-config --cxxflags --cppflags --ldflags --libs`

