//
//  Core.hpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#ifndef Core_hpp
#define Core_hpp

#include "../libretro.h"
#include <string>

#define DefineFunction(RET_TYPE, NAME, ...) RET_TYPE( * NAME)(__VA_ARGS__) = nullptr

class Core {
public:
    Core(void* core_handle);
    ~Core();
    /**
     Loads a given core.
     */
    static Core* LoadCore(std::string filepath);
    
    bool RetroCoreEnvironment(unsigned cmd, void* data);
    bool LoadGame(std::string filename);
    /**
     Fully resets the core, including clearing any saves stored on the hardware.
     This is done by loading the savestate created when initially loading the game.
     */
    void FullReset();
    
    bool has_game = false;
    std::string current_game;
    
    std::string library_name;
    std::string library_version;
    std::string valid_extensions;
    bool need_fullpath;
    bool block_extract;
    
#pragma mark Core Functions
    // Init + Deinit
    DefineFunction(void, RetroInit);
    DefineFunction(void, RetroDeinit);
    
    // Info
    DefineFunction(unsigned, RetroAPIVersion);
    DefineFunction(void, RetroGetSystemInfo, struct retro_system_info * info);
    DefineFunction(void, RetroGetSystemAVInfo, struct retro_system_av_info * info);
    DefineFunction(void, RetroSetControllerPortDevice, unsigned port, unsigned device);
    
    // Running
    DefineFunction(void, RetroReset);
    DefineFunction(void, RetroRun);
    
    // Serialization
    DefineFunction(size_t, RetroSerializeSize);
    /**
     Serializes the entire state of the emulator into data.
     */
    DefineFunction(bool, RetroSerialize, void* data, size_t size);
    DefineFunction(bool, RetroUnserialize, const void* data, size_t size);
    
    // Game
    DefineFunction(bool, RetroLoadGame, const struct retro_game_info* game);
    DefineFunction(void, RetroUnloadGame);
    
    // Callbacks
    DefineFunction(void, RetroSetEnvironment, retro_environment_t);
    DefineFunction(void, RetroSetVideoRefresh, retro_video_refresh_t);
    DefineFunction(void, RetroSetInputPoll, retro_input_poll_t);
    DefineFunction(void, RetroSetInputState, retro_input_state_t);
    DefineFunction(void, RetroSetAudioSample, retro_audio_sample_t);
    DefineFunction(void, RetroSetAudioSampleBatch, retro_audio_sample_batch_t);
    

private:
    size_t clean_start_size;
    /*
     This buffer holds the data for clean starting the game (i.e. no save data, etc.).
     */
    void* clean_start = NULL;
    
    void* core_handle;
    void LoadSymbol(std::string name, void** var);
};

#endif /* Core_hpp */
