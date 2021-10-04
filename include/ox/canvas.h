//
// Created by alexoxorn on 2021-08-28.
//

#ifndef RANK_VIEWER_CANVAS_H
#define RANK_VIEWER_CANVAS_H

#include <iostream>
#include <functional>
#include <thread>
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <memory>
#include <filesystem>
#include <ox/formatting.h>
#include <ox/colors.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>

#define WRAP_SDL_POINTER(ARG_TYPE, NAME) std::unique_ptr<ARG_TYPE, class ARG_TYPE##_destroyer>;\
struct ARG_TYPE##_destroyer { auto operator()(ARG_TYPE * _p) { return NAME(_p);} };


namespace ox{
    using position = std::pair<int, int>;
    using namespace std::chrono_literals;

    using sdl_window = WRAP_SDL_POINTER(SDL_Window, SDL_DestroyWindow)
    using sdl_surface = WRAP_SDL_POINTER(SDL_Surface, SDL_FreeSurface);
    using sdl_renderer = WRAP_SDL_POINTER(SDL_Renderer, SDL_DestroyRenderer);

    class sdl_instance {
        using callback_signature = bool(sdl_instance&);
        using callback_type = std::function<callback_signature>;

        using handler_signature = bool(sdl_instance&, const SDL_Event&);
        using handler_type = std::function<handler_signature>;
        using event_handler_array = std::array<handler_type, SDL_LASTEVENT>;
        using event_handler_ptr = std::unique_ptr<event_handler_array>;

        sdl_window _window;
        sdl_surface _screen_surface;

        std::unordered_map<std::string, SDL_Surface *> sub_surfaces;
        std::chrono::milliseconds render_sleep = 16ms;

        callback_type init_callback = [](sdl_instance&){return false;};
        callback_type loop_callback = [](sdl_instance&){return false;};
        event_handler_ptr event_callbacks = std::make_unique<event_handler_array>();
        bool _successful_init;

        void _default_quit_handler() {
            event_callbacks->at(SDL_QUIT) = [](sdl_instance&, const SDL_Event&) { return true; };
        }

        void _redraw() {
            SDL_UpdateWindowSurface(window());
        }
    public:
        explicit sdl_instance(const std::string& name, position _size,
                              position _position = {SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED}) {
            _successful_init = true;
            _default_quit_handler();

            if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
                printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
                _successful_init = false;
            } else {
                _window = sdl_window{SDL_CreateWindow(
                        name.c_str(),
                        _position.first,
                        _position.second,
                        _size.first, _size.second, 
                        SDL_WINDOW_SHOWN
                        )};
                if(!_window) {
                    printf( "_window could not be created! SDL_Error: %s\n", SDL_GetError() );
                    _successful_init = false;
                } else {
                    _screen_surface = sdl_surface {SDL_GetWindowSurface( window() )};
                    // auto temp = SDL_CreateRenderer( window(), -1, SDL_RENDERER_ACCELERATED);
                    // printf("%p, %s\n", temp, SDL_GetError());
                }
            }
        }
        sdl_instance(const sdl_instance&) = delete;
        sdl_instance& operator=(const sdl_instance&) = delete;
        sdl_instance& operator=(sdl_instance&&) = delete;
        sdl_instance(sdl_instance && other) = delete;

        bool load_media(const std::string& name, const std::filesystem::path& path) {
            SDL_Surface* image = SDL_LoadBMP(path.c_str());
            if (image == nullptr) {
                printf( "Unable to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
                return false;
            }
            auto result = sub_surfaces.insert({name, image});
            return result.second;
        }

        void register_init_callback(const callback_type& init) {
            init_callback = init;
        }
        void register_loop_callback(const callback_type& loop) {
            loop_callback = loop;
        }
        void register_loop_callback(SDL_EventType type, const handler_type& callback) {
            event_callbacks->at(type) = callback;
        }

        bool successful_init() const {
            return _successful_init;
        }

        SDL_Window *window() const {
            return _window.get();
        }

        SDL_Surface *screen_surface() const {
            return _screen_surface.get();
        }

        SDL_Surface* get_surface(const std::string& name) {
            return sub_surfaces.at(name);
        }

        void operator()() {
            SDL_Event e;
            bool quit = false;

            if(init_callback(*this))
                _redraw();
            while (!quit) {
                bool main_loop_redraw = false;
                bool event_redraw = false;
                auto now = std::chrono::steady_clock::now();
                auto end = now + render_sleep;

                while(SDL_PollEvent( &e )) {
                    if (auto handler = event_callbacks->at(e.type)) {
                        event_redraw = true;
                        quit = handler(*this, e) || quit;
                    }
                }
                main_loop_redraw = loop_callback(*this);
                
                if(main_loop_redraw || event_redraw) {
                    // printf("redraw\n");
                    _redraw();
                }
                std::this_thread::sleep_until(end);
            }
        }
    };

    inline void foo() {
        sdl_instance is{"Test Image", {1920, 1080}};
        // is.load_media("rank_image", "/home/alexoxorn/Pictures/temp/rank.bmp");
        // is.register_init_callback([] (sdl_instance& a) mutable {
        //     SDL_BlitSurface(a.get_surface("rank_image"), nullptr, a.screen_surface(), nullptr);
        //     return true;
        // });

        double y = 50.0;

        is.register_loop_callback([&y, hue = 0.0, width = 0](sdl_instance& win) mutable {
            color rgb = hsl_to_rgb({.hsl = {hue / 360.0, 1.0, 0.5}});

            hue += 1.0;
            if(hue >= 360.0) {
                hue = 0.0;
            }

            SDL_Rect r{50, 50, 50, 50};

            auto& [red, green, blue] = rgb.rgb;
            // Set render color to blue ( rect will be rendered in this color )
            // printf("%d, %d, %d\n", red, green, blue);

            // Render rect
            SDL_FillRect( win.screen_surface(), &r, rgb.rgb.rgba255() );
            // SDL_RenderFillRect( win.renderer(), &r );

            // XSetForeground(win.display, win.gc, win.state.color);
            // XFillRectangle(win.display, win.window, win.gc, 5, 5, static_cast<int>(win.attr.width * y / 100.0), 100);
            return true;
        });

        is();
    }
}

#undef WRAP_SDL_POINTER
#endif //RANK_VIEWER_CANVAS_H
