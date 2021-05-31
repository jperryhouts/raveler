# Raveler
#   Copyright (C) 2021 Jonathan Perry-Houts

#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.

#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.

#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

sounds_ogg=$(shell /bin/bash -c 'for n in {0..299}; do printf "web/sounds_ogg/%03d.ogg\n" $$n; done')
sounds_opus=$(shell /bin/bash -c 'for n in {0..299}; do printf "web/sounds_opus/%03d.mp3\n" $$n; done')
sounds_mp3=$(shell /bin/bash -c 'for n in {0..299}; do printf "web/sounds_mp3/%03d.mp3\n" $$n; done')

ifeq ($(strip $(NOMAGICK)),)
# @bash -c 'if [ "`which Magick++-config`" == "" ]; then \
# 		echo -e "\nMagick++ library not found." ; \
# 		echo -e "On Debian/Ubuntu, try:" ; \
# 		echo -e " sudo apt install graphicsmagick-libmagick-dev-compat\n"; \
# 		exit 1 ; fi'
flags=$(shell Magick++-config --cxxflags --cppflags --ldflags --libs)
else
flags=-D NOMAGICK
endif

.PHONY: clean cli wasm checkMagick checkEmscripten sounds sounds_wav sounds_ogg sounds_opus sounds_mp3

clean:
	rm -rf build/wasm build/cli

sounds: $(sounds_ogg) $(sounds_mp3)

cli: build/cli/raveler

wasm: build/wasm/raveler.html

## When generating the sound bites we need some Python packages
## To avoid cluttering the local python environment, the
## necessary TTS tools should be installed in a virtualenv
venv:
	python3 -m virtualenv "venv"
	/bin/bash -c 'source "venv/bin/activate" && \
		pip install TTS PyQt5 ; \
		deactivate'

## Not currently used, but this can convert an image into a raw
## sequence of bytes representing the pixels.
build/%.gray: data/%.jpg
	convert "$<" -gravity center -extent 1:1 -resize 600 -size 600x600 -depth 8 GRAY:- > "$@"

build/cli/raveler: src/ravelcli.cc include/ravelcli.h src/libraveler.cc include/libraveler.h
	mkdir -p `dirname "$@"`
	c++ -o "$@" "src/libraveler.cc" "src/ravelcli.cc" -I./include $(flags)

build/wasm/raveler.html: src/raveljs.cc include/raveljs.h src/libraveler.cc include/libraveler.h
	@bash -c 'if [ "`which em++`" == "" ]; then \
		echo -e "\nEnscripten not found." ; \
		echo -e "On Debian/Ubuntu, try:" ; \
		echo -e " sudo apt install emscripten\n"; \
		exit 1 ; fi'
	mkdir -p `dirname "$@"`
	em++ -o "$@" "src/libraveler.cc" "src/raveljs.cc" -I./include \
		-s WASM=1 -s INITIAL_MEMORY=402653184 \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' \
		-s EXPORTED_FUNCTIONS='["_init","_ravel"]' \
		-s NO_EXIT_RUNTIME=1 \
		-s ASYNCIFY -O3 --closure 1


sounds_ogg: $(sounds_ogg)

sounds_opus: $(sounds_opus)

sounds_mp3: $(sounds_mp3)

## For whatever reason, the tts models all seem to fail occasionally,
## frequently on numbers ending in 7, but not always, and occasionally
## others as well. The resulting .wav file comes out all weird and the
## speech is slurred. It's simple to identify these because they tend
## to be much longer than average. Specifically, a normal audio clip
## will be well under 3 seconds, but the corrupted ones range from 5
## to 13 seconds.
##
## These TTS models are stochastic enough that re-running the speech
## generator on the same inputs will often fix the issue. The make
## recipe here will try up to 5 times to generate an accurate clip
## before failing and moving on. On failure it will create no output.
web/sounds_wav/%.wav:
	@mkdir -p "`dirname "$@"`"
	@tts --version
	@/bin/bash -c 'n=0; ok=0; \
		while [ $$n -lt 5 ] && [ $$ok -eq 0 ]; do \
			tts --text "pin $*." --out_path "$@" \
				--model_name tts_models/en/ljspeech/tacotron2-DDC; \
			duration=`ffprobe "$@" 2>&1 | sed -e "s/,//" | awk '"'"'/Duration/{ \
					split($$2,len,":"); printf("%d", len[1]*3600 + len[2]*60 + len[3]) \
				}'"'"'`; \
			if [ $$duration -lt 4 ]; then ok=1; fi; \
			n=$$((n+1)); \
		done; \
		if [ $$ok -eq 0 ]; then \
			echo "Unable to generate sound: $@"; \
			rm -f "$@"; \
		fi;'

web/sounds_opus/%.opus: web/sounds_wav/%.wav
	@mkdir -p "`dirname "$@"`"
	ffmpeg -i "$<" -c:a libopus -b:a 24k -ac 1 -vn "$@"

web/sounds_ogg/%.ogg: web/sounds_wav/%.wav
	@mkdir -p "`dirname "$@"`"
	ffmpeg -i "$<" -c:a libvorbis -ac 1 -vn "$@"

web/sounds_mp3/%.mp3: web/sounds_wav/%.wav
	@mkdir -p "`dirname "$@"`"
	ffmpeg -i "$<" -c:a libmp3lame -b:a 96k -ac 1 -vn "$@"
