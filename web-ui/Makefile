
sounds_ogg=$(shell /bin/bash -c 'for n in {0..299}; do printf "sounds_ogg/%03d.ogg\n" $$n; done')
sounds_opus=$(shell /bin/bash -c 'for n in {0..299}; do printf "sounds_opus/%03d.mp3\n" $$n; done')
sounds_mp3=$(shell /bin/bash -c 'for n in {0..299}; do printf "sounds_mp3/%03d.mp3\n" $$n; done')

.PHONY:
sounds: $(sounds_ogg) $(sounds_mp3)

.PHONY:
sounds_ogg: $(sounds_ogg)

.PHONY:
sounds_opus: $(sounds_opus)

.PHONY:
sounds_mp3: $(sounds_mp3)

## To avoid cluttering the local python environment, the
## necessary TTS tools should be installed in a virtualenv
## sandbox.

venv:
	python3 -m virtualenv "venv"
	/bin/bash -c 'source "venv/bin/activate" && \
		pip install TTS PyQt5 ; \
		deactivate'

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

sounds_wav/%.wav: venv
	@mkdir -p "`dirname "$@"`"
	@/bin/bash -c 'source "venv/bin/activate" ; \
		n=0; ok=0; \
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
		fi; \
		deactivate'

sounds_opus/%.opus: sounds_wav/%.wav
	mkdir -p sounds_opus
	ffmpeg -i "$<" -c:a libopus -b:a 24k -ac 1 -vn "$@"

sounds_ogg/%.ogg: sounds_wav/%.wav
	mkdir -p sounds_ogg
	ffmpeg -i "$<" -c:a libvorbis -ac 1 -vn "$@"

sounds_mp3/%.mp3: sounds_wav/%.wav
	mkdir -p sounds_mp3
	ffmpeg -i "$<" -c:a libmp3lame -b:a 96k -ac 1 -vn "$@"