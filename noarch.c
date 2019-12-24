#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <pthread.h>

#include "libretro.h"
//#include "formats/rbmp.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

static GLFWwindow *g_win = NULL;
static float g_scale = 3;

static GLfloat g_vertex[] = {
	-1.0f, -1.0f, // left-bottom
	-1.0f,  1.0f, // left-top
	 1.0f, -1.0f, // right-bottom
	 1.0f,  1.0f, // right-top
};
static GLfloat g_texcoords[] ={
	0.0f,  1.0f,
	0.0f,  0.0f,
	1.0f,  1.0f,
	1.0f,  0.0f,
};

static struct {
	GLuint tex_id;
	GLuint pitch;
	GLint tex_w, tex_h;
	GLuint clip_w, clip_h;

	GLuint pixfmt;
	GLuint pixtype;
	GLuint bpp;
} g_video  = {0};

int width = 0;
int height = 0;
int current_frame = 0;

static struct {
	void * handle;
	bool initialized;

	void( * retro_init)(void);
	void( * retro_deinit)(void);
	unsigned( * retro_api_version)(void);
	void( * retro_get_system_info)(struct retro_system_info * info);
	void( * retro_get_system_av_info)(struct retro_system_av_info * info);
	void( * retro_set_controller_port_device)(unsigned port, unsigned device);
	void( * retro_reset)(void);
	void( * retro_run)(void);
	size_t ( * retro_serialize_size)(void);
	bool (* retro_serialize)(void *data, size_t size);
	bool (* retro_unserialize)(const void *data, size_t size);
	//	void retro_cheat_reset(void);
	//	void retro_cheat_set(unsigned index, bool enabled, const char *code);
	bool( * retro_load_game)(const struct retro_game_info * game);
	//	bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info);
	void( * retro_unload_game)(void);
	//	unsigned retro_get_region(void);
	//	void *retro_get_memory_data(unsigned id);
	//	size_t retro_get_memory_size(unsigned id);
}
g_retro;

struct keymap {
	unsigned k;
	unsigned rk;
};

struct keymap g_binds[] = {
	{ GLFW_KEY_X, RETRO_DEVICE_ID_JOYPAD_A },
	{ GLFW_KEY_Z, RETRO_DEVICE_ID_JOYPAD_B },
	{ GLFW_KEY_A, RETRO_DEVICE_ID_JOYPAD_Y },
	{ GLFW_KEY_S, RETRO_DEVICE_ID_JOYPAD_X },
	{ GLFW_KEY_UP, RETRO_DEVICE_ID_JOYPAD_UP },
	{ GLFW_KEY_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN },
	{ GLFW_KEY_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT },
	{ GLFW_KEY_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT },
	{ GLFW_KEY_ENTER, RETRO_DEVICE_ID_JOYPAD_START },
	{ GLFW_KEY_BACKSPACE, RETRO_DEVICE_ID_JOYPAD_SELECT },

	{ 0, 0 }
};

static unsigned g_joy[RETRO_DEVICE_ID_JOYPAD_R3+1] = { 0 };

#define load_sym(V, S) do {\
	if (!((*(void**)&V) = dlsym(g_retro.handle, #S))) \
		die("[noarch] Failed to load symbol '" #S "'': %s", dlerror()); \
	} while (0)
#define load_retro_sym(S) load_sym(g_retro.S, S)

static void die(const char * fmt, ...) {
	char buffer[4096];

	va_list va;
	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	fputs(buffer, stderr);
	fputc('\n', stderr);
	fflush(stderr);

	exit(EXIT_FAILURE);
}

static void refresh_vertex_data() {

	GLfloat *coords = g_texcoords;
	coords[1] = coords[5] = (float)g_video.clip_h / g_video.tex_h;
	coords[4] = coords[6] = (float)g_video.clip_w / g_video.tex_w;
}


static void resize_cb(GLFWwindow *win, int w, int h) {
	glViewport(0, 0, w, h);
}


static void create_window(int width, int height) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	g_win = glfwCreateWindow(width, height, "nanoarch", NULL, NULL);
	if (!g_win)
		die("Failed to create window.");

	glfwSetFramebufferSizeCallback(g_win, resize_cb);

	glfwMakeContextCurrent(g_win);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		die("Failed to initialize glew");

	glfwSwapInterval(1);

	printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glEnable(GL_TEXTURE_2D);

//	refresh_vertex_data();

	resize_cb(g_win, width, height);
}


static void resize_to_aspect(double ratio, int sw, int sh, int *dw, int *dh) {
	*dw = sw;
	*dh = sh;

	if (ratio <= 0)
		ratio = (double)sw / sh;

	if ((float)sw / sh < 1)
		*dw = *dh * ratio;
	else
		*dh = *dw / ratio;
}


static void video_configure(const struct retro_game_geometry *geom) {
	int nwidth, nheight;

	resize_to_aspect(geom->aspect_ratio, geom->base_width * 1, geom->base_height * 1, &nwidth, &nheight);

	nwidth *= g_scale;
	nheight *= g_scale;

	if (!g_win)
		create_window(nwidth, nheight);

	if (g_video.tex_id)
		glDeleteTextures(1, &g_video.tex_id);

	g_video.tex_id = 0;

	if (!g_video.pixfmt)
		g_video.pixfmt = GL_UNSIGNED_SHORT_5_5_5_1;

	glfwSetWindowSize(g_win, nwidth, nheight);

	glGenTextures(1, &g_video.tex_id);

	if (!g_video.tex_id)
		die("Failed to create the video texture");

	g_video.pitch = geom->base_width * g_video.bpp;

	glBindTexture(GL_TEXTURE_2D, g_video.tex_id);

//	glPixelStorei(GL_UNPACK_ALIGNMENT, s_video.pixfmt == GL_UNSIGNED_INT_8_8_8_8_REV ? 4 : 2);
//	glPixelStorei(GL_UNPACK_ROW_LENGTH, s_video.pitch / s_video.bpp);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, geom->max_width, geom->max_height, 0,
			g_video.pixtype, g_video.pixfmt, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	g_video.tex_w = geom->max_width;
	g_video.tex_h = geom->max_height;
	g_video.clip_w = geom->base_width;
	g_video.clip_h = geom->base_height;

	refresh_vertex_data();
}


static bool video_set_pixel_format(unsigned format) {
	if (g_video.tex_id)
		die("Tried to change pixel format after initialization.");

	switch (format) {
	case RETRO_PIXEL_FORMAT_0RGB1555:
		g_video.pixfmt = GL_UNSIGNED_SHORT_5_5_5_1;
		g_video.pixtype = GL_BGRA;
		g_video.bpp = sizeof(uint16_t);
		break;
	case RETRO_PIXEL_FORMAT_XRGB8888:
		g_video.pixfmt = GL_UNSIGNED_INT_8_8_8_8_REV;
		g_video.pixtype = GL_BGRA;
		g_video.bpp = sizeof(uint32_t);
		break;
	case RETRO_PIXEL_FORMAT_RGB565:
		g_video.pixfmt  = GL_UNSIGNED_SHORT_5_6_5;
		g_video.pixtype = GL_RGB;
		g_video.bpp = sizeof(uint16_t);
		break;
	default:
		die("Unknown pixel type %u", format);
	}

	return true;
}


static void video_refresh(const void *data, unsigned width, unsigned height, unsigned pitch) {
	if (g_video.clip_w != width || g_video.clip_h != height) {
		g_video.clip_h = height;
		g_video.clip_w = width;

		refresh_vertex_data();
	}

	glBindTexture(GL_TEXTURE_2D, g_video.tex_id);

	if (pitch != g_video.pitch) {
		g_video.pitch = pitch;
		glPixelStorei(GL_UNPACK_ROW_LENGTH, g_video.pitch / g_video.bpp);
	}

	if (data) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
						g_video.pixtype, g_video.pixfmt, data);
	}
}


static void video_render() {
	glBindTexture(GL_TEXTURE_2D, g_video.tex_id);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, g_vertex);
	glTexCoordPointer(2, GL_FLOAT, 0, g_texcoords);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


static void video_deinit() {
	if (g_video.tex_id)
		glDeleteTextures(1, &g_video.tex_id);

	g_video.tex_id = 0;
}

static void audio_init(int frequency) {
	(void)frequency;
}

static void audio_deinit() {}

static void core_log(enum retro_log_level level,
	const char * fmt, ...) {
	char buffer[4096] = {
		0
	};
	static
	const char * levelstr[] = {
		"dbg",
		"inf",
		"wrn",
		"err"
	};
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (level == 0)
		return;

	fprintf(stderr, "[%s] %s", levelstr[level], buffer);
	fflush(stderr);

	if (level == RETRO_LOG_ERROR)
		exit(EXIT_FAILURE);
}

static bool core_environment(unsigned cmd, void * data) {
	bool * bval;

	switch (cmd) {
	case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
		{
			struct retro_log_callback * cb = (struct retro_log_callback * ) data;
			cb->log = core_log;
			break;
		}
	case RETRO_ENVIRONMENT_GET_CAN_DUPE:
		bval = (bool * ) data; * bval = true;
		break;
	case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
		{
            const enum retro_pixel_format *fmt = (enum retro_pixel_format *)data;

            if (*fmt > RETRO_PIXEL_FORMAT_RGB565)
                return false;

            return video_set_pixel_format(*fmt);
		}
	case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
	case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
		* (const char * * ) data = ".";
		return true;

	default:
		core_log(RETRO_LOG_DEBUG, "Unhandled env #%u", cmd);
		return false;
	}

	return true;
}

static void core_video_refresh(const void * data, unsigned width, unsigned height, size_t pitch) {
	(void)data;
	(void)width;
	(void)height;
	(void)pitch;
	//puts("[noarch] video refresh");
	/*if (current_frame % 1 == 0) {
		char filename[256];
		sprintf(filename, "test_frames/frame_%03d.bmp", current_frame);
		rbmp_save_image(filename, data, width, height, pitch, RBMP_SOURCE_TYPE_RGB565);
	}*/
	current_frame++;
	if (data)
		video_refresh(data, width, height, pitch);
}

void* savestate1;
size_t savestate1_size;

static void core_input_poll(void) {
	int i;
	for (i = 0; g_binds[i].k || g_binds[i].rk; ++i)
		g_joy[g_binds[i].rk] = (glfwGetKey(g_win, g_binds[i].k) == GLFW_PRESS);

	// Quit nanoarch when pressing the Escape key.
	if (glfwGetKey(g_win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(g_win, true);
	}
    
    // reset
    if (glfwGetKey(g_win, GLFW_KEY_R) == GLFW_PRESS) {
        g_retro.retro_reset();
    }
    
    // save
    if (glfwGetKey(g_win, GLFW_KEY_F1) == GLFW_PRESS) {
        g_retro.retro_serialize(savestate1, g_retro.retro_serialize_size());
    }
    
    // load
    if (glfwGetKey(g_win, GLFW_KEY_F2) == GLFW_PRESS) {
        g_retro.retro_unserialize(savestate1, g_retro.retro_serialize_size());
    }
}

static int16_t core_input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
	(void)port;
	(void)device;
	(void)index;
	(void)id;
	if (port || index || (device != RETRO_DEVICE_JOYPAD))
		return 0;
	return g_joy[id];
}

static void core_audio_sample(int16_t left, int16_t right) {
	(void)left;
	(void)right;
}

static size_t core_audio_sample_batch(const int16_t * data, size_t frames) {
	(void)data;
	(void)frames;
	return 0;
}

typedef void (RETRO_CALLCONV *remoteStubMain)();
typedef void (RETRO_CALLCONV *remoteInit)();
typedef void (RETRO_CALLCONV *remoteSetProtocol)(int p);
typedef void (RETRO_CALLCONV *remoteSetPort)(int port);

remoteStubMain remote_stub_main;
remoteInit remote_init;
remoteSetProtocol remote_set_protocol;
remoteSetPort remote_set_port;

static void* remote_main(void* nothing) {
	puts("[noarch] Running Main function for Remote thread.");
        
    remote_init();
	remote_stub_main();

	return NULL;
}

pthread_t gdb_thread;

static void core_load(const char * sofile) {
	void( * set_environment)(retro_environment_t) = NULL;
	void( * set_video_refresh)(retro_video_refresh_t) = NULL;
	void( * set_input_poll)(retro_input_poll_t) = NULL;
	void( * set_input_state)(retro_input_state_t) = NULL;
	void( * set_audio_sample)(retro_audio_sample_t) = NULL;
	void( * set_audio_sample_batch)(retro_audio_sample_batch_t) = NULL;

	memset( & g_retro, 0, sizeof(g_retro));
	g_retro.handle = dlopen(sofile, RTLD_LAZY);

	if (!g_retro.handle)
		die("[noarch] Failed to load core: %s", dlerror());

	dlerror();

	load_retro_sym(retro_init);
	load_retro_sym(retro_deinit);
	load_retro_sym(retro_api_version);
	load_retro_sym(retro_get_system_info);
	load_retro_sym(retro_get_system_av_info);
	load_retro_sym(retro_set_controller_port_device);
	load_retro_sym(retro_reset);
	load_retro_sym(retro_run);
	load_retro_sym(retro_load_game);
	load_retro_sym(retro_unload_game);
    load_retro_sym(retro_serialize_size);
    load_retro_sym(retro_serialize);
    load_retro_sym(retro_unserialize);

	load_sym(set_environment, retro_set_environment);
	load_sym(set_video_refresh, retro_set_video_refresh);
	load_sym(set_input_poll, retro_set_input_poll);
	load_sym(set_input_state, retro_set_input_state);
	load_sym(set_audio_sample, retro_set_audio_sample);
	load_sym(set_audio_sample_batch, retro_set_audio_sample_batch);

//	load_sym(remote_stub_main, remoteStubMain);
//	load_sym(remote_init, remoteInit);
//	load_sym(remote_set_protocol, remoteSetProtocol);
//	load_sym(remote_set_port, remoteSetPort);

	set_environment(core_environment);
	set_video_refresh(core_video_refresh);
	set_input_poll(core_input_poll);
	set_input_state(core_input_state);
	set_audio_sample(core_audio_sample);
	set_audio_sample_batch(core_audio_sample_batch);

	g_retro.retro_init();
	g_retro.initialized = true;

	puts("[noarch] Core loaded");
//
//	remote_set_protocol(0);
//	remote_set_port(55555);
//
//	/* create a second thread which executes inc_x(&x) */
//	if(pthread_create(&gdb_thread, NULL, remote_main, NULL)) {
//		fprintf(stderr, "Error creating thread\n");
//		return;
//	}
}

static void core_load_game(const char * filename) {
	struct retro_system_timing timing = {
		60.0f, 10000.0f
	};
	struct retro_game_geometry geom = {
		0
	};
	struct retro_system_av_info av = {
		geom, timing
	};
	struct retro_system_info system = {
		0, 0, 0, false, false
	};
	struct retro_game_info info = {
		filename,
		0,
		0,
		NULL
	};
	FILE * file = fopen(filename, "rb");

	if (!file)
		goto libc_error;

	fseek(file, 0, SEEK_END);
	info.size = ftell(file);
	rewind(file);

	g_retro.retro_get_system_info( & system);

	if (!system.need_fullpath) {
		info.data = malloc(info.size);

		if (!info.data || !fread((void * ) info.data, info.size, 1, file))
			goto libc_error;
	}

	if (!g_retro.retro_load_game( & info))
		die("[noarch] The core failed to load the content.");

	g_retro.retro_get_system_av_info( & av);

	video_configure( & av.geometry);
	audio_init(av.timing.sample_rate);

	puts("[noarch] game loaded!");
    
    savestate1 = malloc(g_retro.retro_serialize_size());

	return;

	libc_error:
		die("[noarch] Failed to load content '%s'", filename);
}

static void core_unload() {
	if (g_retro.initialized)
		g_retro.retro_deinit();

	if (g_retro.handle)
		dlclose(g_retro.handle);
}

long time_in_milis() {
	struct timeval  tv;
	gettimeofday(&tv, NULL);

	return ((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000); 
}

int main(int argc, char * argv[]) {
	if (argc < 3)
		die("Usage: %s <core> <game>", argv[0]);

	if (!glfwInit())
		die("Failed to initialize glfw");

	core_load(argv[1]);
	core_load_game(argv[2]);
    
    g_retro.retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
    
    char win_title[256];
    
    int num_frames = 0;
    long frames_start = time_in_milis();
    long original_start = time_in_milis();
    double fps = 0.0;

	while (!glfwWindowShouldClose(g_win)) {
        long start = time_in_milis();

		glfwPollEvents();

		g_retro.retro_run();

		glClear(GL_COLOR_BUFFER_BIT);

		video_render();

		glfwSwapBuffers(g_win);
        
        num_frames++;
        
        long diff = time_in_milis() - start;
        long frames_diff = time_in_milis() - frames_start;

        if (frames_diff >= 1000) {
            frames_start = time_in_milis();
            fps = num_frames / (frames_diff / 1000.0);
            num_frames = 0;
        }
        
        sprintf(win_title, "nanoarch v0.1 | %0.1fs elapsed, %ldms frametime, %0.2f fps", (time_in_milis() - original_start) / 1000.0, diff, fps);
                
        glfwSetWindowTitle(g_win, win_title);
    }

	//printf("[noarch] Time Taken to render 2000 frames: %ldms\n", diff);

	core_unload();
	audio_deinit();
	video_deinit();

	glfwTerminate();

	return 0;
}
