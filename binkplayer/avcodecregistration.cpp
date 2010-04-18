
#include <stdio.h>

extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/avutil.h"
}

class AvCodecRegistration {
public:
    AvCodecRegistration() {
        fprintf(stderr, "Registering AV Codecs.\n");
        av_register_all();
    }
};

static AvCodecRegistration AV_CODEC_REGISTRATION;
