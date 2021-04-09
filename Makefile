
all: img2thread

img2thread: img2thread.cc img2thread.h
	c++ -o "$@" "$<" `Magick++-config --cxxflags --cppflags --ldflags --libs`

