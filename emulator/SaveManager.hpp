//
//  SaveManager.hpp
//  tasarch
//
//  Created by Leonardo Galli on 24.12.19.
//

#pragma once

#include "Core.hpp"

class SaveManager {
public:
    constexpr static int NumStates = 10;
    
    static void SaveState(int num);
    static void LoadState(int num);
    
    static void CoreLoaded(Core* core);
    
private:
    static Core* CurrentCore;
    
    static std::string SavePath();
    static std::string SavePath(int num);
};
