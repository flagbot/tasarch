//
//  Core.cpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#include "Core.hpp"
#include <glib.h>
#include <dlfcn.h>
#include <fstream>

Core* Core::Current = nullptr;

static void input_poll()
{
    
}

static int16_t input_state(unsigned port, unsigned device,
unsigned index, unsigned id)
{
    return 0;
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

Core::Core(void* core_handle) : core_handle(core_handle)
{
    g_message("Loading symbols for core");
    // Init
    LoadSymbol("retro_init", (void**)&this->RetroInit);
    LoadSymbol("retro_deinit", (void**)&this->RetroDeinit);
    // Info
    LoadSymbol("retro_api_version", (void**)&this->RetroAPIVersion);
    LoadSymbol("retro_get_system_info", (void**)&this->RetroGetSystemInfo);
    LoadSymbol("retro_get_system_av_info", (void**)&this->RetroGetSystemAVInfo);
    LoadSymbol("retro_set_controller_port_device", (void**)&this->RetroSetControllerPortDevice);
    // Running
    LoadSymbol("retro_reset", (void**)&this->RetroReset);
    LoadSymbol("retro_run", (void**)&this->RetroRun);
    // Serialization
    LoadSymbol("retro_serialize_size", (void**)&this->RetroSerializeSize);
    LoadSymbol("retro_serialize", (void**)&this->RetroSerialize);
    LoadSymbol("retro_unserialize", (void**)&this->RetroUnserialize);
    // Game
    LoadSymbol("retro_load_game", (void**)&this->RetroLoadGame);
    LoadSymbol("retro_unload_game", (void**)&this->RetroUnloadGame);
    // Callbacks
    LoadSymbol("retro_set_environment", (void**)&this->RetroSetEnvironment);
    LoadSymbol("retro_set_video_refresh", (void**)&this->RetroSetVideoRefresh);
    LoadSymbol("retro_set_input_poll", (void**)&this->RetroSetInputPoll);
    LoadSymbol("retro_set_input_state", (void**)&this->RetroSetInputState);
    LoadSymbol("retro_set_audio_sample", (void**)&this->RetroSetAudioSample);
    LoadSymbol("retro_set_audio_sample_batch", (void**)&this->RetroSetAudioSampleBatch);
    
    g_message("Loaded all symbols for core. API Version: %d", RetroAPIVersion());
    
    Current = this;
    
    RetroSetEnvironment([](unsigned cmd, void* data) {
        return Current->RetroCoreEnvironment(cmd, data);
    });
    
    RetroSetInputPoll(input_poll);
    RetroSetInputState(input_state);
    RetroSetAudioSample(core_audio_sample);
    RetroSetAudioSampleBatch(core_audio_sample_batch);
    
    RetroInit();
    
    retro_system_info system;
    RetroGetSystemInfo(&system);
    library_name = system.library_name;
    library_version = system.library_version;
    valid_exntensions = system.valid_extensions;
    need_fullpath = system.need_fullpath;
    block_extract = system.block_extract;
    
    g_message("Initialized Core: %s (%s)", library_name.c_str(), library_version.c_str());
}

Core::~Core()
{
    RetroDeinit();
    dlclose(core_handle);
}

void Core::LoadSymbol(std::string name, void** var)
{
    void* symbol = dlsym(core_handle, name.c_str());
    
    if (!symbol) {
        g_error("Failed to load symbol %s: %s", name.c_str(), dlerror());
    } else {
        *var = symbol;
    }
}

static void core_log(enum retro_log_level level, const char* fmt, ...)
{
    const GLogLevelFlags levelflags[] = {
        GLogLevelFlags::G_LOG_LEVEL_DEBUG,
        GLogLevelFlags::G_LOG_LEVEL_MESSAGE,
        GLogLevelFlags::G_LOG_LEVEL_WARNING,
        GLogLevelFlags::G_LOG_LEVEL_ERROR
    };
    va_list va;

    va_start(va, fmt);
    g_log("CORE", levelflags[level], fmt, va);
    va_end(va);
}

bool Core::RetroCoreEnvironment(unsigned int cmd, void *data)
{
    switch (cmd) {
        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        {
            struct retro_log_callback * cb = (struct retro_log_callback * ) data;
            cb->log = core_log;
            return true;
        }

        case RETRO_ENVIRONMENT_GET_CAN_DUPE:
            *(bool*)data = true;
            return true;
            
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        {
            const enum retro_pixel_format *fmt = (enum retro_pixel_format *)data;

            if (*fmt > RETRO_PIXEL_FORMAT_RGB565)
                return false;

            return true;
        }
        
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
            * (const char * * ) data = ".";
            return true;
            
        default:
            return false;
    }
}

bool Core::LoadGame(std::string filename)
{
    g_message("Loading Game from %s", filename.c_str());
    
    struct retro_game_info info = {
        filename.c_str(),
        0,
        0,
        NULL
    };
    
    if (!need_fullpath) {
        std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
        if (file.is_open())
        {
            info.size = file.tellg();
            info.data = malloc(info.size);
            file.seekg(0, std::ios::beg);
            file.read((char*)info.data, info.size);
            file.close();
        } else {
            g_error("Failed to open file!");
            return false;
        }
    }
    
    bool result = RetroLoadGame(&info);
    if (!result) {
        g_error("Failed to load game!");
        return false;
    }
    
    g_message("Loaded game!");
    
    return true;
}

Core* Core::LoadCore(std::string filepath)
{
    g_message("Loading RetroArch core from %s", filepath.c_str());
    
    void* handle = dlopen(filepath.c_str(), RTLD_LAZY);
    
    return new Core(handle);
}
