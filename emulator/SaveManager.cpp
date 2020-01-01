//
//  SaveManager.cpp
//  tasarch
//
//  Created by Leonardo Galli on 24.12.19.
//

#include "SaveManager.hpp"
#include <glib.h>
#include <gtkmm.h>
#include <fstream>

Core* SaveManager::CurrentCore = nullptr;

void SaveManager::CoreLoaded(Core* core)
{
    CurrentCore = core;
}

void SaveManager::SaveState(int num)
{
    g_message("Saving state %d", num);
    auto filename = SavePath(num);
    auto serialize_size = CurrentCore->RetroSerializeSize();
    char* buffer = (char*)malloc(serialize_size);
    CurrentCore->RetroSerialize(buffer, serialize_size);
    auto file = std::fstream(filename, std::ios::out | std::ios::binary);
    file.write(buffer, serialize_size);
    file.close();
    free(buffer);
}

void SaveManager::LoadState(int num)
{
    g_message("Loading state %d", num);
    auto filename = SavePath(num);
    if (Glib::file_test(filename, Glib::FileTest::FILE_TEST_EXISTS)) {
        std::ifstream input(filename, std::ios::binary);

        // copies all data into buffer
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
        CurrentCore->RetroUnserialize(buffer.data(), buffer.size());
    } else {
        g_message("No save file for state %d found at %s", num, filename.c_str());
    }
}

std::string SaveManager::SavePath()
{
    if (CurrentCore->has_game) {
        auto rom = Glib::path_get_basename(CurrentCore->current_game);
        auto directory = Glib::path_get_dirname(CurrentCore->current_game);
        auto saves_directory = Glib::build_filename(directory, "saves");
        g_mkdir_with_parents(saves_directory.c_str(), 0777);
        return Glib::build_filename(saves_directory, rom);
    }
    
    return "";
}

std::string SaveManager::SavePath(int num)
{
    auto saves_name = SavePath();
    std::stringstream filename;
    filename << saves_name;
    filename << num;
    filename << ".sav";
    return filename.str();
}
