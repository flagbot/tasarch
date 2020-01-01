//
//  TASMovie.cpp
//  tasarch
//
//  Created by Leonardo Galli on 26.12.19.
//

#include "TASMovie.hpp"
#include <archive.h>
#include <archive_entry.h>
#include "json.hpp"

using json = nlohmann::json;

TASMovie::TASMovie(Emulator* emulator) : emulator(emulator)
{
    Signals::InputGet.Connect(sigc::mem_fun(this, &TASMovie::OnInputGet));
    Signals::InputUpdated.Connect(sigc::mem_fun(this, &TASMovie::OnInputUpdated));
    Signals::FrameRendered.Connect(sigc::mem_fun(this, &TASMovie::OnFrameRendered));
}

void TASMovie::ResetMovie()
{
    this->emulator->Pause();
    this->emulator->core->FullReset();
    current_frame = 0;
    num_in_frame = 0;
}

void TASMovie::ResetToFrame(long frame)
{
    this->emulator->Pause();
    
    // Find closest save state
    void* save_state = NULL;
    long save_state_frame = 0;
    
    for (auto cur_frame = 0; cur_frame <= std::min(max_frame, frame); cur_frame++) {
        if (save_states[cur_frame]) {
            save_state = save_states[cur_frame];
            save_state_frame = cur_frame;
        }
    }
    
    // Load the save state
    if (save_state) {
        this->emulator->core->RetroUnserialize(save_state, this->emulator->core->RetroSerializeSize());
    } else {
        this->emulator->core->FullReset();
    }
    current_frame = save_state_frame;
    num_in_frame = 0;
    
    long num_frames_to_run = frame - save_state_frame;
    if (num_frames_to_run > 60) {
        g_error("Need to run emulator for %ld frames to be able to jump to %ld. This might take a long time!", num_frames_to_run, frame);
    }
    
    this->emulator->RunFor(num_frames_to_run);
}

void TASMovie::Save(std::string filepath)
{
    std::string act_filepath = filepath;
    if (filepath == "") act_filepath = this->filepath;
    
    std::string moviename = Glib::path_get_basename(filepath);
    auto dot_pos = moviename.find_last_of(".");
    if (dot_pos != std::string::npos) moviename = moviename.substr(0, dot_pos);
    
    struct archive *a;
    
    a = archive_write_new();
    // make a zip file
    archive_write_set_format(a, ARCHIVE_FORMAT_ZIP);
    archive_write_open_filename(a, filepath.c_str());
    
    // add save_states
    // save_states common entry
    size_t size = this->emulator->core->RetroSerializeSize();
    struct archive_entry *entry = archive_entry_new();
    archive_entry_set_size(entry, size);
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    
    char name_buf[256];
    
    for (auto pair : save_states) {
        void* state = pair.second;
        if (state) {
            // We have an actual value here.
            long frame = pair.first;
            
            snprintf(name_buf, 256, "%05ld.sav", frame);
            std::string save_name = Glib::build_filename(moviename, "saves", name_buf);
            archive_entry_set_pathname(entry, save_name.c_str());
            archive_write_header(a, entry);
            
            archive_write_data(a, state, size);
        }
    }
    
    // add inputs
    json recordings;
    for (auto pair : inputs) {
        long frame = pair.first;
        json inps = json::array();
        for (bitmask<Input> inp : pair.second) {
            inps.push_back(inp.bits());
        }
        std::string frame_str = std::to_string(frame);
        recordings[frame_str] = inps;
    }
    
    std::string inputs_name = Glib::build_filename(moviename, "inputs.json");
    archive_entry_set_pathname(entry, inputs_name.c_str());
    std::string inputs_str = recordings.dump();
    archive_entry_set_size(entry, inputs_str.length());
    archive_write_header(a, entry);
    archive_write_data(a, inputs_str.data(), inputs_str.length());
    
    archive_entry_free(entry);
    archive_write_close(a);
    archive_write_free(a);
    
    this->filepath = act_filepath;
}

bitmask<Input> TASMovie::OnInputGet(bitmask<Input> input)
{
    if (!is_recording) {
        auto cur_frame = inputs[current_frame];
        if (num_in_frame < cur_frame.size()) {
            auto rec_input = cur_frame[num_in_frame];
            num_in_frame++;
            return rec_input;
        }
    }
    
    return input;
}

void TASMovie::OnInputUpdated(bitmask<Input> input)
{
    if (is_recording) {
        auto cur_frame = inputs[current_frame];
        if (cur_frame.size() <= num_in_frame) {
            cur_frame.push_back(input);
        } else {
            cur_frame[num_in_frame] = input;
        }
        inputs[current_frame] = cur_frame;
        num_in_frame++;
    }
}

void TASMovie::OnFrameRendered()
{
    // remove any saved inputs that are in the frame but have not been overwritten
    auto cur_frame = inputs[current_frame];
    if (cur_frame.size() > num_in_frame) {
        cur_frame.erase(cur_frame.begin() + num_in_frame, cur_frame.end());
    }
    // Update current_frame and set num_in_frame back to zero
    Signals::MovieFrameChanged.Emit(current_frame);
    current_frame++;
    max_frame = std::max(current_frame, max_frame);
    num_in_frame = 0;
    
    // Save the state every 60 frames (i.e. once per second)
    if (current_frame % 60 == 0) {
        void* existing = save_states[current_frame];
        size_t size = this->emulator->core->RetroSerializeSize();
        // If we did not save before, allocate the space.
        if (!existing) existing = malloc(size);

        this->emulator->core->RetroSerialize(existing, size);
        save_states[current_frame] = existing;
    }
}

TASMovie* TASMovie::LoadFromFile(std::string filepath, Emulator *emulator)
{
#define CHECK(ret, msg, ...) if (ret != ARCHIVE_OK) { \
const char* error_str = archive_error_string(a); \
g_error(msg, __VA_ARGS__, error_str); \
return nullptr; \
}
    
    TASMovie* movie = new TASMovie(emulator);
    
    struct archive* a;
    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    int r = archive_read_open_filename(a, filepath.c_str(), 10240);
    
    CHECK(r, "Had error opening %s for read: %s", filepath.c_str())
    
    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* pathname = archive_entry_pathname(entry);
        std::string filename = Glib::path_get_basename(pathname);
        size_t size = archive_entry_size(entry);
        char* data = (char*)malloc(size);
        archive_read_data(a, data, size);
        
        if (filename == "inputs.json") {
            std::string data_str(data, size);
            auto recordings = json::parse(data_str);
            long max_frame = 0;
            for (auto pair : recordings.items()) {
                std::string frame_str = pair.key();
                long frame = std::stol(frame_str);
                max_frame = std::max(frame, max_frame);
                for (auto inp : pair.value().get<std::vector<int>>()) {
                    movie->inputs[frame].push_back(bitmask<Input>((Input)inp));
                }
            }
            movie->max_frame = max_frame;
        } else {
            auto dot_pos = filename.find(".");
            std::string frame_str = filename.substr(0, dot_pos);
            long frame = std::stol(frame_str);
            movie->save_states[frame] = data;
        }
    }
    
    archive_read_free(a);
    
    movie->is_recording = false;
    
    return movie;
}
