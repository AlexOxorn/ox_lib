//
// Created by alexoxorn on 2021-08-28.
//
#include <iostream>
#include <functional>
#include <string_view>
#include <ox/canvas.h>
#include <ox/algorithms.h>

namespace ox{
    unsigned sdl_color(SDL_Surface* s, color c) {
        auto [red, green, blue] = c;
        return SDL_MapRGB(s->format, red, green, blue);
    };

    void sdl_check_error() {
        std::string_view error = SDL_GetError();
        if (error.size() > 0) {
            std::cerr << "SDL ERROR: " << SDL_GetError() << std::endl;
            SDL_ClearError();
        }
    }

    sdl_instance::sdl_instance(const std::string& name, bool renderer,
                                position _size, position _position
    ) {
        int image_flags = IMG_INIT_PNG;
        if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
            printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
            return;
        }
        _successful_init = true;
        if(!( IMG_Init( image_flags ) & image_flags ) ) {
            printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        }
        _image_init = true;
        if(TTF_Init() == -1) {
            printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        }
        _ttf_init = true;
        _window = sdl_window{SDL_CreateWindow(
                name.c_str(),
                _position.first,
                _position.second,
                _size.first, _size.second, 
                SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
                )};
        if(!_window) {
            printf( "_window could not be created! SDL_Error: %s\n", SDL_GetError() );
            return;
        } else {
            if(renderer) {
                replace_renderer();
            } else {
                resurface();
            }
        }
    }

    bool sdl_instance::load_media(const std::string& name, const std::filesystem::path& path) {
        sdl_surface image{SDL_LoadBMP(path.c_str())};
        if (!image) {
            return false;
        }
        insert_or_overwrite(sub_surfaces, name, std::move(image));
        return true;
    }

    bool sdl_instance::load_texture(const std::string& name, const std::filesystem::path& path, SDL_bool key, color key_color) {
        texture new_texture{screen_renderer(), path, key, key_color};
        if (!new_texture) {
            return false;
        }
        insert_or_overwrite(textures, name, std::move(new_texture));
        return true;
    }

    bool sdl_instance::load_text(const std::string& name, const std::filesystem::path& ttf_path, int size, const std::string& s, SDL_Color color) {
        sdl_font font{TTF_OpenFont(ttf_path.c_str(), size)};
        texture new_texture{screen_renderer(), font.get(), s, color};
        if (!new_texture) {
            return false;
        }
        insert_or_overwrite(textures, name, std::move(new_texture));
        return true;
    }

    bool sdl_instance::texture::load_from_file(const std::filesystem::path& path, SDL_bool key, color key_color) {
        sdl_surface loaded_surface{IMG_Load(path.c_str())};
        if( loaded_surface == nullptr ) {
            printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
            return false;
        }
        //Color key image
        SDL_SetColorKey( loaded_surface.get(), key, sdl_color(loaded_surface.get(), key_color) );
        _texture = sdl_texture{SDL_CreateTextureFromSurface(_renderer, loaded_surface.get() )};
        if (!_texture) {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
            return false;
        }
        _width = loaded_surface->w;
        _height = loaded_surface->h;
        return true;
    }
    
    bool sdl_instance::texture::load_from_rendered_text(TTF_Font* font, const std::string& texture_text, SDL_Color text_color) {
        sdl_surface text_surface{TTF_RenderText_Solid(font, texture_text.c_str(), text_color)};
        if( text_surface == nullptr ) {
            printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
            return false;
        }
        //Color key image
        _texture = sdl_texture{SDL_CreateTextureFromSurface(_renderer, text_surface.get())};
        if (!_texture) {
            printf( "Unable to create texture from rendered text: \"%s\"! SDL Error: %s\n", texture_text.c_str(), SDL_GetError() );
            return false;
        }
        _width = text_surface->w;
        _height = text_surface->h;
        return true;
    }

    void sdl_instance::texture::render(
        int x, int y,
        SDL_Rect* clip,
        double angle,
        SDL_Point* center,
        SDL_RendererFlip flip 
    ) const {
        //Set rendering space and render to screen
        SDL_Rect render_quad = { x, y, _width, _height };

        //Set clip rendering dimensions
        if( clip != nullptr ) {
            render_quad.w = clip->w;
            render_quad.h = clip->h;
        }

        //Render to screen
        SDL_RenderCopyEx( _renderer, _texture.get(), nullptr, &render_quad, angle, center, flip );
    }

    void foo() {
        sdl_instance win{"Test Image", false, {1920, 1080}};
        SDL_Event e;
        double y = 50.0;
        double hue = 0.0;
        const std::chrono::milliseconds render_sleep = 16ms;

        while(true) {
            auto now = std::chrono::steady_clock::now();
            auto end = now + render_sleep;
            while(SDL_PollEvent( &e )) {
                switch (e.type) {
                case SDL_QUIT:
                    goto end;
                case SDL_WINDOWEVENT:
                    switch(e.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        win.resurface();    
                    }
                }
            }

            color rgb = hsl_to_rgb({hue / 360.0, 1.0, 0.5});

            hue += 1.0;
            if(hue >= 360.0) {
                hue = 0.0;
            }

            int resized_width, resized_height;
            SDL_GetWindowSize(win.window(), &resized_width, &resized_height);

            SDL_Rect r{50, 50, static_cast<int>(resized_width * y / 100.0), 50};

            auto [red, green, blue] = rgb;

            SDL_FillRect(win.screen_surface(), &r, SDL_MapRGB(win.screen_surface()->format, red, green, blue) );
            win.redraw();
            std::this_thread::sleep_until(end);
        }

        end:
        return;
    }

    void foo2() {
        sdl_instance win{"Test Image", true, {1920, 1080}};
        SDL_Event e;
        double y = 50.0;
        double hue = 0.0;
        const std::chrono::milliseconds render_sleep = 16ms;

        while(true) {
            ox::sdl_check_error();
            auto now = std::chrono::steady_clock::now();
            auto end = now + render_sleep;
            while(SDL_PollEvent( &e )) {
                switch (e.type) {
                case SDL_QUIT:
                    goto end;
                case SDL_WINDOWEVENT:
                    switch(e.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        // win.resurface(); 
                        break;
                    }
                }
            }

            color rgb = hsl_to_rgb({hue / 360.0, 1.0, 0.5});

            hue += 1.0;
            if(hue >= 360.0) {
                hue = 0.0;
            }

            int resized_width, resized_height;
            SDL_GetWindowSize(win.window(), &resized_width, &resized_height);

            SDL_Rect r{50, 50, static_cast<int>(resized_width * y / 100.0), 50};
            
            win.set_renderer_color(ox::named_colors::black);
            win.clear_render();
            win.set_renderer_color(rgb);
            SDL_RenderFillRect(win.screen_renderer(), &r);
            win.redraw();
            std::this_thread::sleep_until(end);
        }

        end:
        return;
    }
}
