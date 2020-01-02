#include "gui/TASArchApplication.h"
#include "emulator/Core.hpp"
#include <glib.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

#ifdef CUSTOM_LOG

std::string get_string(std::unordered_map<std::string, GLogField>* fields, std::string name, std::string value = "")
{
    if (fields->find(name) != fields->end()) {
        GLogField field = fields->at(name);
        if (field.length > 0)
            return std::string((char*)field.value, field.length);
        return std::string((char*)field.value);
    }
    return value;
}

GLogWriterOutput write_log(GLogLevelFlags log_level, const GLogField * fields, gsize n_fields, gpointer user_data)
{
    std::unordered_map<std::string, GLogField> flds;
    for (int i = 0; i < n_fields; i++) {
        GLogField field = fields[i];
        flds[field.key] = field;
    }
    
    std::unordered_map<GLogLevelFlags, std::string> level_to_str = {
        {G_LOG_LEVEL_DEBUG, "DEBUG"},
        {G_LOG_LEVEL_INFO, "INFO"},
        {G_LOG_LEVEL_MESSAGE, "MESSAGE"},
        {G_LOG_LEVEL_WARNING, "WARNING"},
        {G_LOG_LEVEL_ERROR, "ERROR"}
    };
    
    std::unordered_map<GLogLevelFlags, std::string> level_to_emoji = {
        {G_LOG_LEVEL_DEBUG, "⚫️"},
        {G_LOG_LEVEL_INFO, ""},
        {G_LOG_LEVEL_MESSAGE, "🟢"},
        {G_LOG_LEVEL_WARNING, "🟠"},
        {G_LOG_LEVEL_ERROR, "🔴"}
    };
    
    std::string message = get_string(&flds, "MESSAGE");
    std::string file = get_string(&flds, "CODE_FILE");
    std::string function = get_string(&flds, "CODE_FUNC");
    std::string line = get_string(&flds, "CODE_LINE");
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    
    std::stringstream fmt_file;
    
    std::string source_folder = SOURCE_CODE_ROOT;
    
    std::string short_file = file;
    size_t source_loc = file.find(source_folder);
    if (source_loc != std::string::npos) short_file = file.replace(source_loc, source_folder.length(), "");
    
    fmt_file << "[" << short_file <<  ":" << line;
    fmt_file << " :: " << function << "]: ";
    
    std::stringstream log;
    
    log << std::put_time(&tm, "%Y-%m-%d %H:%M:%S,123");
    log << " " << level_to_emoji[log_level] << " " << std::setw(8) << std::left << level_to_str[log_level] << level_to_emoji[log_level] << " ";
    if (file != "")
        log << std::setw(50) << std::left << fmt_file.str();
    else
        log << ": ";
    log <<  message;
    
    std::cout << log.str() << std::endl;
    
    return GLogWriterOutput::G_LOG_WRITER_HANDLED;
}

#endif // CUSTOM_LOG

int main(int argc, char *argv[])
{
#ifdef CUSTOM_LOG
    g_log_set_writer_func(write_log, nullptr, nullptr);
#endif

    auto application = TASArchApplication::create();
        
    // Start the application, showing the initial window,
    // and opening extra views for any files that it is asked to open,
    // for instance as a command-line parameter.
    // run() will return when the last window has been closed.
    return application->run(argc, argv);
}
