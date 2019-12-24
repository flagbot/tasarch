//
//  Emulator.cpp
//  tasarch
//
//  Created by Leonardo Galli on 22.12.19.
//

#include "Emulator.h"

Emulator::Emulator()
{
    
}

void Emulator::LoadCore(std::string filename)
{
    core = Core::LoadCore(filename);
    Signals::CoreLoaded.Emit(core);
}
