//
// Created by alexoxorn on 2021-08-28.
//

#ifndef RANK_VIEWER_TESTX11_H
#define RANK_VIEWER_TESTX11_H

#include <iostream>
#include <functional>
#include <thread>
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <ox/formatting.h>
#include <ox/colors.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

namespace ox{
    using position = std::pair<int, int>;
    using namespace std::chrono_literals;

    class EmptyX11StateClass {};

    template <typename StateType = EmptyX11StateClass>
    class X11Window {
        using callback_signature = bool(X11Window&);
        std::array<std::vector<std::function<callback_signature>>, LASTEvent> event_callbacks;
        std::vector<std::function<callback_signature>> continuous_callbacks;
        std::vector<std::function<callback_signature>> draw_callbacks;
        std::chrono::milliseconds render_sleep = 16ms;
    public:
        StateType state{};
        Display* display;
        int screen;
        Window window;
        GC gc;
        XEvent event{};
        XWindowAttributes  attr{};
        bool quit_flag{};

        X11Window(position _position, position _size, int boarder_width = 5) {
            display = XOpenDisplay(nullptr);
            screen = DefaultScreen(display);

            window = XCreateSimpleWindow(
                    display, DefaultRootWindow(display),
                    _position.first, _position.second,
                    _size.first, _size.second, boarder_width,
                    BlackPixel(display, screen), WhitePixel(display, screen)
            );

            XSetStandardProperties(display, window, "Test Window", "Icon Window", None, nullptr, 0, nullptr);
            XSelectInput(
                    display,
                    window,
                    ExposureMask |
                    KeyPressMask |
                    ButtonPressMask |
                    StructureNotifyMask
//                ResizeRedirectMask
            );
            gc = XCreateGC(display, window, 0, nullptr);

            XSetBackground(display,gc,WhitePixel(display, screen));
            XSetForeground(display,gc,BlackPixel(display, screen));

            XClearWindow(display, window);
            XMapRaised(display, window);
        }
        X11Window(const X11Window&) = delete;
        X11Window& operator=(const X11Window&) = delete;
        X11Window& operator=(X11Window&&) = delete;

        X11Window(X11Window && other) noexcept :
                event_callbacks{std::move(other.event_callbacks)},
                continuous_callbacks{std::move(other.continuous_callbacks)},
                draw_callbacks{std::move(other.draw_callbacks)},
                render_sleep{std::move(other.render_sleep)},
                state{std::move(other.state)} {
            display = other.display;
            gc = other.gc;
            screen = other.screen;
            window = other.window;
            event = other.event;

            other.display = nullptr;
            other.gc = nullptr;
        }

        void register_event_callback(int event_type, std::function<callback_signature> callback) {
            event_callbacks.at(event_type).push_back(std::move(callback));
        }

        void register_continuous_callback(std::function<callback_signature> callback) {
            continuous_callbacks.push_back(std::move(callback));
        }

        void register_draw_callback(std::function<callback_signature> callback) {
            draw_callbacks.push_back(std::move(callback));
        }

        void redraw() {
            XClearWindow(display, window);
            for(const auto& draw : draw_callbacks) {
                draw(*this);
            }
        }

        void operator()() {
            while (true) {
                auto now = std::chrono::steady_clock::now();
                auto end = now + render_sleep;
                bool trigger_redraw = false;
                while(XPending(display)) {
                    XNextEvent(display, &event);
                    for(const auto& callback : event_callbacks.at(event.type)) {
                        trigger_redraw = callback(*this) || trigger_redraw;
                    }
                    if (quit_flag) return;
                }
                for(const auto& callback : continuous_callbacks) {
                    trigger_redraw = callback(*this) || trigger_redraw;
                }
                if(trigger_redraw) {
                    redraw();
                    XFlush(display);
                }
                std::this_thread::sleep_until(end);
            }
        }

        ~X11Window() {
            if (display) {
                XFreeGC(display, gc);
                XDestroyWindow(display, window);
                XCloseDisplay(display);
            }
        }
    };

    struct TestStruct {
        unsigned long color{};
    };

    inline void foo() {
        using TestWindow = X11Window<TestStruct>;
        TestWindow x{{100, 100}, {500, 300}};

        x.register_event_callback(ConfigureNotify, [](TestWindow& win) {
            XGetWindowAttributes(win.display, win.window, &win.attr);
            return true;
        });

        x.register_event_callback(KeyPress, [](TestWindow& win) {
            KeySym key;
            XGetWindowAttributes(win.display, win.window, &win.attr);
            char text[255];
            if (XLookupString(&win.event.xkey,text,255,&key,nullptr)==1) {
                if (text[0]=='q') {
                    win.quit_flag = true;
                    return false;
                }
                printf("You pressed the %c key!\n",text[0]);
            }
            return false;
        });

//    x.rgb.register_event_callback([](X11Window& win) {
//        if (win.event.type==ButtonPress) {
//            /* tell where the mouse Button was Pressed */
//            printf("You pressed a button at (%i,%i)\n",
//                   win.event.xbutton.x, win.event.xbutton.y);
//        }
//        return false;
//    });

        double y = 50.0;

        x.register_continuous_callback([&y, hue = 0.0, width = 0](TestWindow& win) mutable {
            color rgb = hsl_to_rgb({.hsl = {hue / 360.0, 1.0, 0.5}});
            win.state.color = rgb255(rgb);

            hue += 1.0;
            if(hue >= 360.0) {
                hue = 0.0;
            }
            return true;
        });

        x.register_draw_callback([&y](TestWindow& win) {
            XSetForeground(win.display, win.gc, win.state.color);
            XFillRectangle(win.display, win.window, win.gc, 5, 5, static_cast<int>(win.attr.width * y / 100.0), 100);
            return false;
        });

        std::thread loop{std::move(x)};
        loop.join();
    }
}


#endif //RANK_VIEWER_TESTX11_H
