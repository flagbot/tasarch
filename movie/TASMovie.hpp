//
//  TASMovie.hpp
//  tasarch
//
//  Created by Leonardo Galli on 26.12.19.
//

#pragma once

#include "../emulator/Signals.h"
#include "../emulator/InputManager.hpp"
#include "../emulator/Emulator.h"
#include <unordered_map>
#include <string>

class TASMovie {
public:
    TASMovie(Emulator* emulator);
    std::unordered_map<long, std::vector<bitmask<Input>>> inputs = {};
    bool is_recording = true;
    bool is_playing = false;
    
    std::string filepath = "";
    
    long max_frame = 0;
    
    // Resets current_frame mostly.
    void ResetMovie();
    void ResetToFrame(long frame);
    void Save(std::string filepath = "");
    
    static TASMovie* LoadFromFile(std::string filepath, Emulator* emulator);
    
private:
    bitmask<Input> OnInputGet(bitmask<Input> input);
    void OnInputUpdated(bitmask<Input> input);
    void OnFrameRendered();
    
    Emulator* emulator;
    long current_frame = 0;
    long num_in_frame = 0;
    
    std::unordered_map<long, void*> save_states = {};
};
