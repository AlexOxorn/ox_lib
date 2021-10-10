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
#include <SDL2/SDL_ttf.h>

#define WRAP_SDL_POINTER(ARG_TYPE, NAME) std::unique_ptr<ARG_TYPE, struct ARG_TYPE##_destroyer>;\
struct ARG_TYPE##_destroyer { auto operator()(ARG_TYPE * _p) { return NAME(_p);} };


namespace ox{
    using position = std::pair<int, int>;
    using namespace std::chrono_literals;

    using sdl_window = WRAP_SDL_POINTER(SDL_Window, SDL_DestroyWindow)
    using sdl_surface = WRAP_SDL_POINTER(SDL_Surface, SDL_FreeSurface);
    using sdl_renderer = WRAP_SDL_POINTER(SDL_Renderer, SDL_DestroyRenderer);
    using sdl_font = WRAP_SDL_POINTER(TTF_Font, TTF_CloseFont)
    using sdl_texture = WRAP_SDL_POINTER(SDL_Texture, SDL_DestroyTexture)

    class sdl_instance {
    public:
        class texture {
            SDL_Renderer* _renderer;
            
            //The actual hardware texture
            sdl_texture _texture;

            //Image dimensions
            int _width = 0;
            int _height = 0;
            friend class sdl_instance;
        public:
            texture(SDL_Renderer* _r) : _renderer{_r} {};
            texture(SDL_Renderer* _r, const std::filesystem::path& path,
                    SDL_bool key = SDL_FALSE, color key_color = named_colors::black) : _renderer{_r} {
                load_from_file(path, key, key_color);
            };
            texture(SDL_Renderer* _r, TTF_Font* font, const std::string& texture_text, SDL_Color text_color = {0, 0, 0, 0}) : _renderer{_r} {
                load_from_rendered_text(font, texture_text, text_color);
            };
            texture(texture&& other) = default;
            texture& operator=(texture&& other) = default;

            //Loads image at specified path
            bool load_from_file(const std::filesystem::path& path, SDL_bool key = SDL_FALSE, color key_color = named_colors::black);
            
            //Creates image from font string
            bool load_from_rendered_text(TTF_Font* font, const std::string& textureText, SDL_Color textColor );

            //Set color modulation
            void setColor( Uint8 red, Uint8 green, Uint8 blue );

            //Set blending
            void setBlendMode( SDL_BlendMode blending );

            //Set alpha modulation
            void setAlpha( Uint8 alpha );
            
            //Renders texture at given point
            void render(
                int x, int y,
                SDL_Rect* clip = nullptr,
                double angle = 0.0,
                SDL_Point* center = nullptr,
                SDL_RendererFlip flip = SDL_FLIP_NONE
            ) const;

            //Gets image dimensions
            int getWidth() const;
            int getHeight() const;

            SDL_Texture* get() const {
                return _texture.get();
            }

            explicit operator bool() const {
                return bool{_texture};
            }
        };
    private:
        sdl_window _window;
        SDL_Surface* _screen_surface = nullptr;
        sdl_renderer _screen_renderer;

        std::unordered_map<std::string, sdl_surface> sub_surfaces;
        std::unordered_map<std::string, texture> textures;
        bool _successful_init = false;
        bool _ttf_init = false;
        bool _image_init = false;

    public:
        explicit sdl_instance(const std::string& name, bool renderer = false, position _size = {1920, 1080},
                              position _position = {SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED});
        sdl_instance(const sdl_instance&) = delete;
        sdl_instance& operator=(const sdl_instance&) = delete;
        sdl_instance& operator=(sdl_instance&&) = delete;
        sdl_instance(sdl_instance && other) = delete;

        void resurface() {
            _screen_surface = SDL_GetWindowSurface( window() );
        }

        void replace_renderer() {
            _screen_renderer = sdl_renderer{SDL_CreateRenderer( window(), -1, SDL_RENDERER_ACCELERATED )};
        }

        void redraw() {
            if(_screen_surface) {
                SDL_UpdateWindowSurface(window());
            } else if (_screen_renderer) {
                SDL_RenderPresent(screen_renderer());
            }
        }

        bool load_media(const std::string& name, const std::filesystem::path& path);

        bool load_texture(const std::string& name, const std::filesystem::path& path, SDL_bool key = SDL_FALSE, color key_color = named_colors::black);
        
        bool load_text(const std::string& name, const std::filesystem::path& ttf_path, int size, const std::string& s, SDL_Color color = {0, 0, 0, 0});

        bool successful_init() const {
            return _successful_init;
        }

        SDL_Window *window() const {
            return _window.get();
        }

        SDL_Surface *screen_surface() const {
            return _screen_surface;
        }

        SDL_Renderer *screen_renderer() const {
            return _screen_renderer.get();
        }

        SDL_Surface* get_surface(const std::string& name) {
            return sub_surfaces.at(name).get();
        }

        const texture* get_texture(const std::string& name) {
            auto result = textures.find(name);
            if (result == textures.end()) {
                return nullptr;
            }
            return &(result->second);
        }

        void set_renderer_color(color c, int alpha = 0xff) {
            SDL_SetRenderDrawColor(screen_renderer(), c.rgb.r, c.rgb.g, c.rgb.b, alpha);
        }

        void clear_render() {
            SDL_RenderClear( screen_renderer() );
        }
    };

    void foo();
    void foo2();
    unsigned sdl_color(SDL_Surface* s, color c);
    void sdl_check_error();
}

#undef WRAP_SDL_POINTER
#endif //RANK_VIEWER_CANVAS_H
