OPENGL_COMMONSOURCES = video_output/opengl/vout_helper.c \
       video_output/opengl/gl_api.c \
       video_output/opengl/gl_api.h \
       video_output/opengl/gl_common.h \
       video_output/opengl/gl_util.c \
       video_output/opengl/gl_util.h \
       video_output/opengl/interop.h \
       video_output/opengl/vout_helper.h \
       video_output/opengl/internal.h \
       video_output/opengl/interop.c video_output/opengl/interop_sw.c \
       video_output/opengl/renderer.c \
       video_output/opengl/renderer.h \
       video_output/opengl/sampler.c \
       video_output/opengl/sampler.h \
       video_output/opengl/sampler_priv.h \
       video_output/opengl/sub_renderer.c \
       video_output/opengl/sub_renderer.h
if HAVE_LIBPLACEBO
OPENGL_COMMONSOURCES += video_output/placebo_utils.c video_output/placebo_utils.h
endif

OPENGL_COMMONCFLAGS = $(LIBPLACEBO_CFLAGS)
OPENGL_COMMONLIBS = $(LIBPLACEBO_LIBS)

### OpenGL ###
libgles2_plugin_la_SOURCES = $(OPENGL_COMMONSOURCES) video_output/opengl/display.c
libgles2_plugin_la_CFLAGS = $(AM_CFLAGS) $(GLES2_CFLAGS) -DUSE_OPENGL_ES2 $(OPENGL_COMMONCFLAGS)
libgles2_plugin_la_LIBADD = $(GLES2_LIBS) $(LIBM) $(OPENGL_COMMONLIBS)
libgles2_plugin_la_LDFLAGS = $(AM_LDFLAGS) -rpath '$(voutdir)'

EXTRA_LTLIBRARIES += libgles2_plugin.la
vout_LTLIBRARIES += $(LTLIBgles2)

libgl_plugin_la_SOURCES = $(OPENGL_COMMONSOURCES) video_output/opengl/display.c
libgl_plugin_la_CFLAGS = $(AM_CFLAGS) $(GL_CFLAGS) $(OPENGL_COMMONCFLAGS)
libgl_plugin_la_LIBADD = $(LIBM) $(OPENGL_COMMONLIBS)
if HAVE_WIN32
libgl_plugin_la_CFLAGS += -DHAVE_GL_CORE_SYMBOLS
libgl_plugin_la_LIBADD += $(GL_LIBS)
endif

libglinterop_vaapi_plugin_la_SOURCES = video_output/opengl/interop_vaapi.c \
       video_output/opengl/interop.h \
       hw/vaapi/vlc_vaapi.c hw/vaapi/vlc_vaapi.h
libglinterop_vaapi_plugin_la_CFLAGS = $(AM_CFLAGS) $(GL_CFLAGS)
libglinterop_vaapi_plugin_la_LIBADD = $(LIBVA_LIBS) $(LIBVA_EGL_LIBS)

libglinterop_vdpau_plugin_la_SOURCES = video_output/opengl/interop_vdpau.c \
       video_output/opengl/interop.h hw/vdpau/picture.c hw/vdpau/vlc_vdpau.h
libglinterop_vdpau_plugin_la_CFLAGS = $(AM_CFLAGS) $(VDPAU_CFLAGS)
libglinterop_vdpau_plugin_la_LIBADD = $(LIBDL) libvlc_vdpau.la $(X_LIBS) $(X_PRE_LIBS) -lX11

if HAVE_GL
vout_LTLIBRARIES += libgl_plugin.la
if HAVE_EGL
if HAVE_VAAPI
vout_LTLIBRARIES += libglinterop_vaapi_plugin.la
endif
endif # HAVE_EGL

if HAVE_VDPAU
vout_LTLIBRARIES += libglinterop_vdpau_plugin.la
endif
endif # HAVE_GL
