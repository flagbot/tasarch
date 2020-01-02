//
//  Signals.h
//  tasarch
//
//  Created by Leonardo Galli on 23.12.19.
//

#pragma once

#include <sigc++/sigc++.h>
#include <unordered_map>
#include <string>
#include "InputManager.hpp"

template <const char* Ident, typename TReturn, typename... TArgs>
class Signal {
public:
    typedef sigc::signal<TReturn, TArgs...> signal_type;
    
    TReturn Emit(TArgs... args) {
        return signal.emit(args...);
    };
    
    typename signal_type::iterator Connect(typename signal_type::slot_type slot) {
        return signal.connect(slot);
    };
    
private:
    signal_type signal;
    const std::string identifier = Ident;
};

#define SIGNAL(enum, ident, ...)    inline constexpr static char str_signal_##enum [] = ident; \
inline static Signal<str_signal_##enum, __VA_ARGS__> enum;

#define RETROSIGNAL(enum, ident, ret, ...) SIGNAL(enum, ident, ret, ##__VA_ARGS__) \
template <typename... TArgs> \
inline static ret Emit##enum(TArgs... args) { return enum.Emit(args...); };

class Signals {
public:
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
#pragma mark Core
    SIGNAL(CoreLoaded, "core.loaded", void, Core*)
    SIGNAL(CoreInitialized, "core.initialized", void, Core*)
    
#pragma mark Retro
    RETROSIGNAL(RetroInputPoll, "retro.input.poll", void)
    RETROSIGNAL(RetroInputState, "retro.input.state", int16_t, unsigned, unsigned, unsigned, unsigned)
    RETROSIGNAL(RetroSetEnvironment, "retro.environment.set", bool, unsigned, void*)
    RETROSIGNAL(RetroAudioSample, "retro.audio.sample", void, int16_t, int16_t);
    RETROSIGNAL(RetroAudioSampleBatch, "retro.audio.sample-batch", size_t, const int16_t*, size_t)
    RETROSIGNAL(RetroVideoRefresh, "retro.video.refresh", void, const void*, size_t, size_t, size_t)

#pragma mark Game
    SIGNAL(GameLoaded, "game.loaded", void, Core*)
    SIGNAL(FrameRendered, "frame.rendered", void)
    SIGNAL(Draw, "draw", void)
    
#pragma mark Input
    SIGNAL(InputGet, "input.get", bitmask<Input>, bitmask<Input>);
    SIGNAL(InputUpdated, "input.updated", void, bitmask<Input>);
    
#pragma mark Movie
    SIGNAL(MovieFrameChanged, "movie.frame.changed", void, long);
    
#pragma clang diagnostic pop
};

/*#define SIGNALS(FN) \
    FN(CoreLoaded, core_loaded, "core.loaded", void)

#define MAKE_ENUM(enum, type_name, str, ...) enum,

#define MAKE_TYPE(enum, type_name, str, ...) typedef sigc::signal<__VA_ARGS__> type_signal_type_name;

#define MAKE_OBJECT(enum, type_name, str, ...) {SignalType::enum, type_signal_type_name()},

#define MAKE_STRS(enum, type_name, str, ...) {SignalType::enum, str},

class Signal {
public:
    
#pragma mark SignalType
    enum SignalType {
        SIGNALS(MAKE_ENUM)
    };
    
#pragma mark Signal Typedefs
    
    SIGNALS(MAKE_TYPE)
    
#pragma mark Signal Map
    
    static std::unordered_map<SignalType, sigc::signal_base> Signals = {
        SIGNALS(MAKE_OBJECT)
    };
    
#pragma mark String Map
    
    static std::unordered_map<SignalType, std::string> SignalStrings = {
        SIGNALS(MAKE_STRS)
    };
    
#pragma mark Class as enum
    
    Signal() = default;
    constexpr Signal(SignalType signal_type) : signal_type(signal_type) {}
    
#if Enable switch(signal) use case:
    operator SignalType() const { return signal_type; }  // Allow switch and comparisons.
                                            // note: Putting constexpr here causes
                                            // clang to stop warning on incomplete
                                            // case handling.
    explicit operator bool() = delete;        // Prevent usage: if(fruit)
#else
    constexpr bool operator==(Signal a) const { return signal_type == a.signal_type; }
    constexpr bool operator!=(Signal a) const { return signal_type != a.signal_type; }
#endif
    
private:
    SignalType signal_type;
};*/
