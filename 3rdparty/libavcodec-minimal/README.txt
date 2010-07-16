This is a stripped down version of FFMPEG that has been compiled with the following options to
only include BINK and MP3 support, as required by EvilTemple:

./configure \
--enable-memalign-hack \
--arch=x86 \
--target-os=mingw32 \
--cross-prefix=i586-mingw32msvc- \
--enable-cross-compile \
--enable-shared \
--disable-ffplay \
--disable-ffprobe \
--disable-ffserver \
--disable-avdevice \
--disable-network \
--disable-everything \
--enable-decoder=bink \
--enable-decoder=binkaudio_dct \
--enable-decoder=binkaudio_rdft \
--enable-decoder=mp3 \
--enable-demuxer=bink \
--enable-demuxer=mp3 \
--enable-parser=mpegaudio \
--enable-protocol=file \
--enable-bsf=mp3_header_decompress

Please see http://www.ffmpeg.org for more information.