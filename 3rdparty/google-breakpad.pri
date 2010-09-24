
win32 {

    DEFINES += GOOGLE_BREAKPAD_ENABLED

    LIBS += -lwininet

    SOURCES += $${PWD}/google-breakpad/src/client/windows/handler/exception_handler.cc \
               $${PWD}/google-breakpad/src/client/windows/crash_generation/client_info.cc \
               $${PWD}/google-breakpad/src/client/windows/crash_generation/crash_generation_client.cc \
               $${PWD}/google-breakpad/src/client/windows/crash_generation/crash_generation_server.cc \
               $${PWD}/google-breakpad/src/client/windows/crash_generation/minidump_generator.cc \
               $${PWD}/google-breakpad/src/client/windows/sender/crash_report_sender.cc \
               $${PWD}/google-breakpad/src/common/windows/guid_string.cc \
               $${PWD}/google-breakpad/src/common/windows/http_upload.cc

    INCLUDEPATH += $${PWD}/google-breakpad/src/ \
                   $${PWD}/google-breakpad/src/client/windows

}
