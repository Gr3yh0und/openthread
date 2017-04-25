include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

EXTRA_DIST              = \
                          coap.h \
                          $(NULL)

lib_LIBRARIES           = libmicrocoap.a

libmicrocoap_a_CPPFLAGS = \
                          -I$(top_srcdir)/third_party/microcoap \
                          $(CPPFLAGS)                           \
                          $(NULL)

libmicrocoap_a_SOURCES  = \
                          coap.c                          \
                          coap_parse.c                    \
                          resources.c                     \
                          $(NULL)

noinst_HEADERS          =  \
                           coap.h          \
                           inet.h          \
                           $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
