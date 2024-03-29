#ifndef OXLIB_COLORS_H
#define OXLIB_COLORS_H

#include <cstddef>
#include <string>
#include <array>
#include <concepts>
#include <ox/types.h>
#include <climits>
#include <type_traits>
#include <bit>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <ox/formatting.h>

namespace ox {
    struct color {
        int r, g, b;

        constexpr color() = default;
        constexpr color(int r, int g, int b) : r{r}, g{g}, b{b} {};
        constexpr explicit color(uint32_t rgb) {
            b = int(rgb & 0xff);
            rgb >>= 8;
            g = int(rgb & 0xff);
            rgb >>= 8;
            r = int(rgb & 0xff);
        }

        [[nodiscard]] unsigned long rgb255() const {
            return b + (g << 8) + (r << 16);
        }
        [[nodiscard]] unsigned long rgba255(int a = 255) const {
            return a + (b << 8) + (g << 16) + (r << 24);
        }

        void lighten(double d) {
            r += 255 * d;
            g += 255 * d;
            b += 255 * d;
            redistribute_rgb();
        }

    private:
        void redistribute_rgb() {
            int threshold = 255;
            int m = std::max({r, g, b});
            if (m <= threshold)
                return;
            int total = r + g + b;
            if (total >= 3 * threshold) {
                r = b = g = 255;
                return;
            };
            double x = (3.0 * threshold - total) / (3.0 * m - total);
            double grey = threshold - x * m;
            r = static_cast<int>(grey + x * r);
            g = static_cast<int>(grey + x * g);
            b = static_cast<int>(grey + x * b);
        }
    };

    struct hsl {
        double h, s, l;
    };

    inline std::ostream& operator<<(std::ostream& out, const color& c) {
        return out << ox::format{ox::escape::direct_color, 2, c.r, c.g, c.b};
    }

    inline unsigned long rgb255(color c) {
        return c.rgb255();
    }

    inline color hsl_to_rgb(hsl hsl) {
        if (hsl.s == 0) {
            int rgb_l = static_cast<int>(hsl.l * 255);
            return {rgb_l, rgb_l, rgb_l};
        }

        double tmp1 = hsl.l < 0.5 ? hsl.l * (1.0 + hsl.s) : hsl.l + hsl.s - hsl.l * hsl.s;
        double tmp2 = 2 * hsl.l - tmp1;

        double tmpColours[3];
        for (int i = 0; i < 3; i++) {
            tmpColours[i] = hsl.h - (i - 1) * 1.0/3;
            while (tmpColours[i] < 0) tmpColours[i] += 1;
            while (tmpColours[i] > 1) tmpColours[i] -= 1;
        }

        for (auto& color: tmpColours) {
            if (color * 6 < 1) {
                color = tmp2 + (tmp1 - tmp2) * 6 * color;
            } else if (2 * color < 1) {
                color = tmp1;
            } else if (3 * color < 2) {
                color = tmp2 + (tmp1 - tmp2) * (2.0 / 3 - color) * 6;
            } else {
                color = tmp2;
            }
        }

        int colors255[3];
        for (int i = 0; i < 3; i++) {
            colors255[i] = static_cast<int>(255 * tmpColours[i]);
        }

        auto& [r, g, b] = colors255;
        return {r, g, b};
    }

    inline void printX11Colours(const std::filesystem::path& color_file = "/usr/share/X11/rgb.txt") {
        std::ifstream color_stream{color_file};
        std::string temp;
        std::getline(color_stream, temp);
        while(color_stream) {
            int r, g, b;
            std::string name;
            color_stream >> r >> g >> b;
            std::getline(color_stream, name);
            std::cout
                << ox::format{ox::escape::background_direct, 2, r, g, b}
                << "        "
                << ox::format{ox::escape::reset}
                << ox::format{ox::escape::direct_color, 2, r, g, b}
                << (name.data() + 2)
                << std::endl;
        }
    }

    namespace named_colors {
        [[maybe_unused]] inline constexpr color snow{255, 250, 250};
        [[maybe_unused]] inline constexpr color GhostWhite{248, 248, 255};
        [[maybe_unused]] inline constexpr color WhiteSmoke{245, 245, 245};
        [[maybe_unused]] inline constexpr color gainsboro{220, 220, 220};
        [[maybe_unused]] inline constexpr color FloralWhite{255, 250, 240};
        [[maybe_unused]] inline constexpr color OldLace{253, 245, 230};
        [[maybe_unused]] inline constexpr color linen{250, 240, 230};
        [[maybe_unused]] inline constexpr color AntiqueWhite{250, 235, 215};
        [[maybe_unused]] inline constexpr color PapayaWhip{255, 239, 213};
        [[maybe_unused]] inline constexpr color BlanchedAlmond{255, 235, 205};
        [[maybe_unused]] inline constexpr color bisque{255, 228, 196};
        [[maybe_unused]] inline constexpr color PeachPuff{255, 218, 185};
        [[maybe_unused]] inline constexpr color NavajoWhite{255, 222, 173};
        [[maybe_unused]] inline constexpr color moccasin{255, 228, 181};
        [[maybe_unused]] inline constexpr color cornsilk{255, 248, 220};
        [[maybe_unused]] inline constexpr color ivory{255, 255, 240};
        [[maybe_unused]] inline constexpr color LemonChiffon{255, 250, 205};
        [[maybe_unused]] inline constexpr color seashell{255, 245, 238};
        [[maybe_unused]] inline constexpr color honeydew{240, 255, 240};
        [[maybe_unused]] inline constexpr color MintCream{245, 255, 250};
        [[maybe_unused]] inline constexpr color azure{240, 255, 255};
        [[maybe_unused]] inline constexpr color AliceBlue{240, 248, 255};
        [[maybe_unused]] inline constexpr color lavender{230, 230, 250};
        [[maybe_unused]] inline constexpr color LavenderBlush{255, 240, 245};
        [[maybe_unused]] inline constexpr color MistyRose{255, 228, 225};
        [[maybe_unused]] inline constexpr color white{255, 255, 255};
        [[maybe_unused]] inline constexpr color black{0, 0, 0};
        [[maybe_unused]] inline constexpr color DarkSlateGray{47, 79, 79};
        [[maybe_unused]] inline constexpr const color& DarkSlateGrey = DarkSlateGray;
        [[maybe_unused]] inline constexpr color DimGray{105, 105, 105};
        [[maybe_unused]] inline constexpr const color& DimGrey = DimGray;
        [[maybe_unused]] inline constexpr color SlateGray{112, 128, 144};
        [[maybe_unused]] inline constexpr const color& SlateGrey = SlateGray;
        [[maybe_unused]] inline constexpr color LightSlateGray{119, 136, 153};
        [[maybe_unused]] inline constexpr const color& LightSlateGrey = LightSlateGray;
        [[maybe_unused]] inline constexpr color gray{190, 190, 190};
        [[maybe_unused]] inline constexpr const color& grey = gray;
        [[maybe_unused]] inline constexpr color LightGray{211, 211, 211};
        [[maybe_unused]] inline constexpr const color& LightGrey = LightGray;
        [[maybe_unused]] inline constexpr color MidnightBlue{25, 25, 112};
        [[maybe_unused]] inline constexpr color navy{0, 0, 128};
        [[maybe_unused]] inline constexpr color NavyBlue{0, 0, 128};
        [[maybe_unused]] inline constexpr color CornflowerBlue{100, 149, 237};
        [[maybe_unused]] inline constexpr color DarkSlateBlue{72, 61, 139};
        [[maybe_unused]] inline constexpr color SlateBlue{106, 90, 205};
        [[maybe_unused]] inline constexpr color MediumSlateBlue{123, 104, 238};
        [[maybe_unused]] inline constexpr color LightSlateBlue{132, 112, 255};
        [[maybe_unused]] inline constexpr color MediumBlue{0, 0, 205};
        [[maybe_unused]] inline constexpr color RoyalBlue{65, 105, 225};
        [[maybe_unused]] inline constexpr color blue{0, 0, 255};
        [[maybe_unused]] inline constexpr color DodgerBlue{30, 144, 255};
        [[maybe_unused]] inline constexpr color DeepSkyBlue{0, 191, 255};
        [[maybe_unused]] inline constexpr color SkyBlue{135, 206, 235};
        [[maybe_unused]] inline constexpr color LightSkyBlue{135, 206, 250};
        [[maybe_unused]] inline constexpr color SteelBlue{70, 130, 180};
        [[maybe_unused]] inline constexpr color LightSteelBlue{176, 196, 222};
        [[maybe_unused]] inline constexpr color LightBlue{173, 216, 230};
        [[maybe_unused]] inline constexpr color PowderBlue{176, 224, 230};
        [[maybe_unused]] inline constexpr color PaleTurquoise{175, 238, 238};
        [[maybe_unused]] inline constexpr color DarkTurquoise{0, 206, 209};
        [[maybe_unused]] inline constexpr color MediumTurquoise{72, 209, 204};
        [[maybe_unused]] inline constexpr color turquoise{64, 224, 208};
        [[maybe_unused]] inline constexpr color cyan{0, 255, 255};
        [[maybe_unused]] inline constexpr color LightCyan{224, 255, 255};
        [[maybe_unused]] inline constexpr color CadetBlue{95, 158, 160};
        [[maybe_unused]] inline constexpr color MediumAquamarine{102, 205, 170};
        [[maybe_unused]] inline constexpr color aquamarine{127, 255, 212};
        [[maybe_unused]] inline constexpr color DarkGreen{0, 100, 0};
        [[maybe_unused]] inline constexpr color DarkOliveGreen{85, 107, 47};
        [[maybe_unused]] inline constexpr color DarkSeaGreen{143, 188, 143};
        [[maybe_unused]] inline constexpr color SeaGreen{46, 139, 87};
        [[maybe_unused]] inline constexpr color MediumSeaGreen{60, 179, 113};
        [[maybe_unused]] inline constexpr color LightSeaGreen{32, 178, 170};
        [[maybe_unused]] inline constexpr color PaleGreen{152, 251, 152};
        [[maybe_unused]] inline constexpr color SpringGreen{0, 255, 127};
        [[maybe_unused]] inline constexpr color LawnGreen{124, 252, 0};
        [[maybe_unused]] inline constexpr color green{0, 255, 0};
        [[maybe_unused]] inline constexpr color chartreuse{127, 255, 0};
        [[maybe_unused]] inline constexpr color MediumSpringGreen{0, 250, 154};
        [[maybe_unused]] inline constexpr color GreenYellow{173, 255, 47};
        [[maybe_unused]] inline constexpr color LimeGreen{50, 205, 50};
        [[maybe_unused]] inline constexpr color YellowGreen{154, 205, 50};
        [[maybe_unused]] inline constexpr color ForestGreen{34, 139, 34};
        [[maybe_unused]] inline constexpr color OliveDrab{107, 142, 35};
        [[maybe_unused]] inline constexpr color DarkKhaki{189, 183, 107};
        [[maybe_unused]] inline constexpr color khaki{240, 230, 140};
        [[maybe_unused]] inline constexpr color PaleGoldenrod{238, 232, 170};
        [[maybe_unused]] inline constexpr color LightGoldenrodYellow{250, 250, 210};
        [[maybe_unused]] inline constexpr color LightYellow{255, 255, 224};
        [[maybe_unused]] inline constexpr color yellow{255, 255, 0};
        [[maybe_unused]] inline constexpr color gold{255, 215, 0};
        [[maybe_unused]] inline constexpr color LightGoldenrod{238, 221, 130};
        [[maybe_unused]] inline constexpr color goldenrod{218, 165, 32};
        [[maybe_unused]] inline constexpr color DarkGoldenrod{184, 134, 11};
        [[maybe_unused]] inline constexpr color RosyBrown{188, 143, 143};
        [[maybe_unused]] inline constexpr color IndianRed{205, 92, 92};
        [[maybe_unused]] inline constexpr color SaddleBrown{139, 69, 19};
        [[maybe_unused]] inline constexpr color sienna{160, 82, 45};
        [[maybe_unused]] inline constexpr color peru{205, 133, 63};
        [[maybe_unused]] inline constexpr color burlywood{222, 184, 135};
        [[maybe_unused]] inline constexpr color beige{245, 245, 220};
        [[maybe_unused]] inline constexpr color wheat{245, 222, 179};
        [[maybe_unused]] inline constexpr color SandyBrown{244, 164, 96};
        [[maybe_unused]] inline constexpr color tan{210, 180, 140};
        [[maybe_unused]] inline constexpr color chocolate{210, 105, 30};
        [[maybe_unused]] inline constexpr color firebrick{178, 34, 34};
        [[maybe_unused]] inline constexpr color brown{165, 42, 42};
        [[maybe_unused]] inline constexpr color DarkSalmon{233, 150, 122};
        [[maybe_unused]] inline constexpr color salmon{250, 128, 114};
        [[maybe_unused]] inline constexpr color LightSalmon{255, 160, 122};
        [[maybe_unused]] inline constexpr color orange{255, 165, 0};
        [[maybe_unused]] inline constexpr color DarkOrange{255, 140, 0};
        [[maybe_unused]] inline constexpr color coral{255, 127, 80};
        [[maybe_unused]] inline constexpr color LightCoral{240, 128, 128};
        [[maybe_unused]] inline constexpr color tomato{255, 99, 71};
        [[maybe_unused]] inline constexpr color OrangeRed{255, 69, 0};
        [[maybe_unused]] inline constexpr color red{255, 0, 0};
        [[maybe_unused]] inline constexpr color HotPink{255, 105, 180};
        [[maybe_unused]] inline constexpr color DeepPink{255, 20, 147};
        [[maybe_unused]] inline constexpr color pink{255, 192, 203};
        [[maybe_unused]] inline constexpr color LightPink{255, 182, 193};
        [[maybe_unused]] inline constexpr color PaleVioletRed{219, 112, 147};
        [[maybe_unused]] inline constexpr color maroon{176, 48, 96};
        [[maybe_unused]] inline constexpr color MediumVioletRed{199, 21, 133};
        [[maybe_unused]] inline constexpr color VioletRed{208, 32, 144};
        [[maybe_unused]] inline constexpr color magenta{255, 0, 255};
        [[maybe_unused]] inline constexpr color violet{238, 130, 238};
        [[maybe_unused]] inline constexpr color plum{221, 160, 221};
        [[maybe_unused]] inline constexpr color orchid{218, 112, 214};
        [[maybe_unused]] inline constexpr color MediumOrchid{186, 85, 211};
        [[maybe_unused]] inline constexpr color DarkOrchid{153, 50, 204};
        [[maybe_unused]] inline constexpr color DarkViolet{148, 0, 211};
        [[maybe_unused]] inline constexpr color BlueViolet{138, 43, 226};
        [[maybe_unused]] inline constexpr color purple{160, 32, 240};
        [[maybe_unused]] inline constexpr color MediumPurple{147, 112, 219};
        [[maybe_unused]] inline constexpr color thistle{216, 191, 216};
        [[maybe_unused]] inline constexpr color snow1{255, 250, 250};
        [[maybe_unused]] inline constexpr color snow2{238, 233, 233};
        [[maybe_unused]] inline constexpr color snow3{205, 201, 201};
        [[maybe_unused]] inline constexpr color snow4{139, 137, 137};
        [[maybe_unused]] inline constexpr color seashell1{255, 245, 238};
        [[maybe_unused]] inline constexpr color seashell2{238, 229, 222};
        [[maybe_unused]] inline constexpr color seashell3{205, 197, 191};
        [[maybe_unused]] inline constexpr color seashell4{139, 134, 130};
        [[maybe_unused]] inline constexpr color AntiqueWhite1{255, 239, 219};
        [[maybe_unused]] inline constexpr color AntiqueWhite2{238, 223, 204};
        [[maybe_unused]] inline constexpr color AntiqueWhite3{205, 192, 176};
        [[maybe_unused]] inline constexpr color AntiqueWhite4{139, 131, 120};
        [[maybe_unused]] inline constexpr color bisque1{255, 228, 196};
        [[maybe_unused]] inline constexpr color bisque2{238, 213, 183};
        [[maybe_unused]] inline constexpr color bisque3{205, 183, 158};
        [[maybe_unused]] inline constexpr color bisque4{139, 125, 107};
        [[maybe_unused]] inline constexpr color PeachPuff1{255, 218, 185};
        [[maybe_unused]] inline constexpr color PeachPuff2{238, 203, 173};
        [[maybe_unused]] inline constexpr color PeachPuff3{205, 175, 149};
        [[maybe_unused]] inline constexpr color PeachPuff4{139, 119, 101};
        [[maybe_unused]] inline constexpr color NavajoWhite1{255, 222, 173};
        [[maybe_unused]] inline constexpr color NavajoWhite2{238, 207, 161};
        [[maybe_unused]] inline constexpr color NavajoWhite3{205, 179, 139};
        [[maybe_unused]] inline constexpr color NavajoWhite4{139, 121, 94};
        [[maybe_unused]] inline constexpr color LemonChiffon1{255, 250, 205};
        [[maybe_unused]] inline constexpr color LemonChiffon2{238, 233, 191};
        [[maybe_unused]] inline constexpr color LemonChiffon3{205, 201, 165};
        [[maybe_unused]] inline constexpr color LemonChiffon4{139, 137, 112};
        [[maybe_unused]] inline constexpr color cornsilk1{255, 248, 220};
        [[maybe_unused]] inline constexpr color cornsilk2{238, 232, 205};
        [[maybe_unused]] inline constexpr color cornsilk3{205, 200, 177};
        [[maybe_unused]] inline constexpr color cornsilk4{139, 136, 120};
        [[maybe_unused]] inline constexpr color ivory1{255, 255, 240};
        [[maybe_unused]] inline constexpr color ivory2{238, 238, 224};
        [[maybe_unused]] inline constexpr color ivory3{205, 205, 193};
        [[maybe_unused]] inline constexpr color ivory4{139, 139, 131};
        [[maybe_unused]] inline constexpr color honeydew1{240, 255, 240};
        [[maybe_unused]] inline constexpr color honeydew2{224, 238, 224};
        [[maybe_unused]] inline constexpr color honeydew3{193, 205, 193};
        [[maybe_unused]] inline constexpr color honeydew4{131, 139, 131};
        [[maybe_unused]] inline constexpr color LavenderBlush1{255, 240, 245};
        [[maybe_unused]] inline constexpr color LavenderBlush2{238, 224, 229};
        [[maybe_unused]] inline constexpr color LavenderBlush3{205, 193, 197};
        [[maybe_unused]] inline constexpr color LavenderBlush4{139, 131, 134};
        [[maybe_unused]] inline constexpr color MistyRose1{255, 228, 225};
        [[maybe_unused]] inline constexpr color MistyRose2{238, 213, 210};
        [[maybe_unused]] inline constexpr color MistyRose3{205, 183, 181};
        [[maybe_unused]] inline constexpr color MistyRose4{139, 125, 123};
        [[maybe_unused]] inline constexpr color azure1{240, 255, 255};
        [[maybe_unused]] inline constexpr color azure2{224, 238, 238};
        [[maybe_unused]] inline constexpr color azure3{193, 205, 205};
        [[maybe_unused]] inline constexpr color azure4{131, 139, 139};
        [[maybe_unused]] inline constexpr color SlateBlue1{131, 111, 255};
        [[maybe_unused]] inline constexpr color SlateBlue2{122, 103, 238};
        [[maybe_unused]] inline constexpr color SlateBlue3{105, 89, 205};
        [[maybe_unused]] inline constexpr color SlateBlue4{71, 60, 139};
        [[maybe_unused]] inline constexpr color RoyalBlue1{72, 118, 255};
        [[maybe_unused]] inline constexpr color RoyalBlue2{67, 110, 238};
        [[maybe_unused]] inline constexpr color RoyalBlue3{58, 95, 205};
        [[maybe_unused]] inline constexpr color RoyalBlue4{39, 64, 139};
        [[maybe_unused]] inline constexpr color blue1{0, 0, 255};
        [[maybe_unused]] inline constexpr color blue2{0, 0, 238};
        [[maybe_unused]] inline constexpr color blue3{0, 0, 205};
        [[maybe_unused]] inline constexpr color blue4{0, 0, 139};
        [[maybe_unused]] inline constexpr color DodgerBlue1{30, 144, 255};
        [[maybe_unused]] inline constexpr color DodgerBlue2{28, 134, 238};
        [[maybe_unused]] inline constexpr color DodgerBlue3{24, 116, 205};
        [[maybe_unused]] inline constexpr color DodgerBlue4{16, 78, 139};
        [[maybe_unused]] inline constexpr color SteelBlue1{99, 184, 255};
        [[maybe_unused]] inline constexpr color SteelBlue2{92, 172, 238};
        [[maybe_unused]] inline constexpr color SteelBlue3{79, 148, 205};
        [[maybe_unused]] inline constexpr color SteelBlue4{54, 100, 139};
        [[maybe_unused]] inline constexpr color DeepSkyBlue1{0, 191, 255};
        [[maybe_unused]] inline constexpr color DeepSkyBlue2{0, 178, 238};
        [[maybe_unused]] inline constexpr color DeepSkyBlue3{0, 154, 205};
        [[maybe_unused]] inline constexpr color DeepSkyBlue4{0, 104, 139};
        [[maybe_unused]] inline constexpr color SkyBlue1{135, 206, 255};
        [[maybe_unused]] inline constexpr color SkyBlue2{126, 192, 238};
        [[maybe_unused]] inline constexpr color SkyBlue3{108, 166, 205};
        [[maybe_unused]] inline constexpr color SkyBlue4{74, 112, 139};
        [[maybe_unused]] inline constexpr color LightSkyBlue1{176, 226, 255};
        [[maybe_unused]] inline constexpr color LightSkyBlue2{164, 211, 238};
        [[maybe_unused]] inline constexpr color LightSkyBlue3{141, 182, 205};
        [[maybe_unused]] inline constexpr color LightSkyBlue4{96, 123, 139};
        [[maybe_unused]] inline constexpr color SlateGray1{198, 226, 255};
        [[maybe_unused]] inline constexpr color SlateGray2{185, 211, 238};
        [[maybe_unused]] inline constexpr color SlateGray3{159, 182, 205};
        [[maybe_unused]] inline constexpr color SlateGray4{108, 123, 139};
        [[maybe_unused]] inline constexpr color LightSteelBlue1{202, 225, 255};
        [[maybe_unused]] inline constexpr color LightSteelBlue2{188, 210, 238};
        [[maybe_unused]] inline constexpr color LightSteelBlue3{162, 181, 205};
        [[maybe_unused]] inline constexpr color LightSteelBlue4{110, 123, 139};
        [[maybe_unused]] inline constexpr color LightBlue1{191, 239, 255};
        [[maybe_unused]] inline constexpr color LightBlue2{178, 223, 238};
        [[maybe_unused]] inline constexpr color LightBlue3{154, 192, 205};
        [[maybe_unused]] inline constexpr color LightBlue4{104, 131, 139};
        [[maybe_unused]] inline constexpr color LightCyan1{224, 255, 255};
        [[maybe_unused]] inline constexpr color LightCyan2{209, 238, 238};
        [[maybe_unused]] inline constexpr color LightCyan3{180, 205, 205};
        [[maybe_unused]] inline constexpr color LightCyan4{122, 139, 139};
        [[maybe_unused]] inline constexpr color PaleTurquoise1{187, 255, 255};
        [[maybe_unused]] inline constexpr color PaleTurquoise2{174, 238, 238};
        [[maybe_unused]] inline constexpr color PaleTurquoise3{150, 205, 205};
        [[maybe_unused]] inline constexpr color PaleTurquoise4{102, 139, 139};
        [[maybe_unused]] inline constexpr color CadetBlue1{152, 245, 255};
        [[maybe_unused]] inline constexpr color CadetBlue2{142, 229, 238};
        [[maybe_unused]] inline constexpr color CadetBlue3{122, 197, 205};
        [[maybe_unused]] inline constexpr color CadetBlue4{83, 134, 139};
        [[maybe_unused]] inline constexpr color turquoise1{0, 245, 255};
        [[maybe_unused]] inline constexpr color turquoise2{0, 229, 238};
        [[maybe_unused]] inline constexpr color turquoise3{0, 197, 205};
        [[maybe_unused]] inline constexpr color turquoise4{0, 134, 139};
        [[maybe_unused]] inline constexpr color cyan1{0, 255, 255};
        [[maybe_unused]] inline constexpr color cyan2{0, 238, 238};
        [[maybe_unused]] inline constexpr color cyan3{0, 205, 205};
        [[maybe_unused]] inline constexpr color cyan4{0, 139, 139};
        [[maybe_unused]] inline constexpr color DarkSlateGray1{151, 255, 255};
        [[maybe_unused]] inline constexpr color DarkSlateGray2{141, 238, 238};
        [[maybe_unused]] inline constexpr color DarkSlateGray3{121, 205, 205};
        [[maybe_unused]] inline constexpr color DarkSlateGray4{82, 139, 139};
        [[maybe_unused]] inline constexpr color aquamarine1{127, 255, 212};
        [[maybe_unused]] inline constexpr color aquamarine2{118, 238, 198};
        [[maybe_unused]] inline constexpr color aquamarine3{102, 205, 170};
        [[maybe_unused]] inline constexpr color aquamarine4{69, 139, 116};
        [[maybe_unused]] inline constexpr color DarkSeaGreen1{193, 255, 193};
        [[maybe_unused]] inline constexpr color DarkSeaGreen2{180, 238, 180};
        [[maybe_unused]] inline constexpr color DarkSeaGreen3{155, 205, 155};
        [[maybe_unused]] inline constexpr color DarkSeaGreen4{105, 139, 105};
        [[maybe_unused]] inline constexpr color SeaGreen1{84, 255, 159};
        [[maybe_unused]] inline constexpr color SeaGreen2{78, 238, 148};
        [[maybe_unused]] inline constexpr color SeaGreen3{67, 205, 128};
        [[maybe_unused]] inline constexpr color SeaGreen4{46, 139, 87};
        [[maybe_unused]] inline constexpr color PaleGreen1{154, 255, 154};
        [[maybe_unused]] inline constexpr color PaleGreen2{144, 238, 144};
        [[maybe_unused]] inline constexpr color PaleGreen3{124, 205, 124};
        [[maybe_unused]] inline constexpr color PaleGreen4{84, 139, 84};
        [[maybe_unused]] inline constexpr color SpringGreen1{0, 255, 127};
        [[maybe_unused]] inline constexpr color SpringGreen2{0, 238, 118};
        [[maybe_unused]] inline constexpr color SpringGreen3{0, 205, 102};
        [[maybe_unused]] inline constexpr color SpringGreen4{0, 139, 69};
        [[maybe_unused]] inline constexpr color green1{0, 255, 0};
        [[maybe_unused]] inline constexpr color green2{0, 238, 0};
        [[maybe_unused]] inline constexpr color green3{0, 205, 0};
        [[maybe_unused]] inline constexpr color green4{0, 139, 0};
        [[maybe_unused]] inline constexpr color chartreuse1{127, 255, 0};
        [[maybe_unused]] inline constexpr color chartreuse2{118, 238, 0};
        [[maybe_unused]] inline constexpr color chartreuse3{102, 205, 0};
        [[maybe_unused]] inline constexpr color chartreuse4{69, 139, 0};
        [[maybe_unused]] inline constexpr color OliveDrab1{192, 255, 62};
        [[maybe_unused]] inline constexpr color OliveDrab2{179, 238, 58};
        [[maybe_unused]] inline constexpr color OliveDrab3{154, 205, 50};
        [[maybe_unused]] inline constexpr color OliveDrab4{105, 139, 34};
        [[maybe_unused]] inline constexpr color DarkOliveGreen1{202, 255, 112};
        [[maybe_unused]] inline constexpr color DarkOliveGreen2{188, 238, 104};
        [[maybe_unused]] inline constexpr color DarkOliveGreen3{162, 205, 90};
        [[maybe_unused]] inline constexpr color DarkOliveGreen4{110, 139, 61};
        [[maybe_unused]] inline constexpr color khaki1{255, 246, 143};
        [[maybe_unused]] inline constexpr color khaki2{238, 230, 133};
        [[maybe_unused]] inline constexpr color khaki3{205, 198, 115};
        [[maybe_unused]] inline constexpr color khaki4{139, 134, 78};
        [[maybe_unused]] inline constexpr color LightGoldenrod1{255, 236, 139};
        [[maybe_unused]] inline constexpr color LightGoldenrod2{238, 220, 130};
        [[maybe_unused]] inline constexpr color LightGoldenrod3{205, 190, 112};
        [[maybe_unused]] inline constexpr color LightGoldenrod4{139, 129, 76};
        [[maybe_unused]] inline constexpr color LightYellow1{255, 255, 224};
        [[maybe_unused]] inline constexpr color LightYellow2{238, 238, 209};
        [[maybe_unused]] inline constexpr color LightYellow3{205, 205, 180};
        [[maybe_unused]] inline constexpr color LightYellow4{139, 139, 122};
        [[maybe_unused]] inline constexpr color yellow1{255, 255, 0};
        [[maybe_unused]] inline constexpr color yellow2{238, 238, 0};
        [[maybe_unused]] inline constexpr color yellow3{205, 205, 0};
        [[maybe_unused]] inline constexpr color yellow4{139, 139, 0};
        [[maybe_unused]] inline constexpr color gold1{255, 215, 0};
        [[maybe_unused]] inline constexpr color gold2{238, 201, 0};
        [[maybe_unused]] inline constexpr color gold3{205, 173, 0};
        [[maybe_unused]] inline constexpr color gold4{139, 117, 0};
        [[maybe_unused]] inline constexpr color goldenrod1{255, 193, 37};
        [[maybe_unused]] inline constexpr color goldenrod2{238, 180, 34};
        [[maybe_unused]] inline constexpr color goldenrod3{205, 155, 29};
        [[maybe_unused]] inline constexpr color goldenrod4{139, 105, 20};
        [[maybe_unused]] inline constexpr color DarkGoldenrod1{255, 185, 15};
        [[maybe_unused]] inline constexpr color DarkGoldenrod2{238, 173, 14};
        [[maybe_unused]] inline constexpr color DarkGoldenrod3{205, 149, 12};
        [[maybe_unused]] inline constexpr color DarkGoldenrod4{139, 101, 8};
        [[maybe_unused]] inline constexpr color RosyBrown1{255, 193, 193};
        [[maybe_unused]] inline constexpr color RosyBrown2{238, 180, 180};
        [[maybe_unused]] inline constexpr color RosyBrown3{205, 155, 155};
        [[maybe_unused]] inline constexpr color RosyBrown4{139, 105, 105};
        [[maybe_unused]] inline constexpr color IndianRed1{255, 106, 106};
        [[maybe_unused]] inline constexpr color IndianRed2{238, 99, 99};
        [[maybe_unused]] inline constexpr color IndianRed3{205, 85, 85};
        [[maybe_unused]] inline constexpr color IndianRed4{139, 58, 58};
        [[maybe_unused]] inline constexpr color sienna1{255, 130, 71};
        [[maybe_unused]] inline constexpr color sienna2{238, 121, 66};
        [[maybe_unused]] inline constexpr color sienna3{205, 104, 57};
        [[maybe_unused]] inline constexpr color sienna4{139, 71, 38};
        [[maybe_unused]] inline constexpr color burlywood1{255, 211, 155};
        [[maybe_unused]] inline constexpr color burlywood2{238, 197, 145};
        [[maybe_unused]] inline constexpr color burlywood3{205, 170, 125};
        [[maybe_unused]] inline constexpr color burlywood4{139, 115, 85};
        [[maybe_unused]] inline constexpr color wheat1{255, 231, 186};
        [[maybe_unused]] inline constexpr color wheat2{238, 216, 174};
        [[maybe_unused]] inline constexpr color wheat3{205, 186, 150};
        [[maybe_unused]] inline constexpr color wheat4{139, 126, 102};
        [[maybe_unused]] inline constexpr color tan1{255, 165, 79};
        [[maybe_unused]] inline constexpr color tan2{238, 154, 73};
        [[maybe_unused]] inline constexpr color tan3{205, 133, 63};
        [[maybe_unused]] inline constexpr color tan4{139, 90, 43};
        [[maybe_unused]] inline constexpr color chocolate1{255, 127, 36};
        [[maybe_unused]] inline constexpr color chocolate2{238, 118, 33};
        [[maybe_unused]] inline constexpr color chocolate3{205, 102, 29};
        [[maybe_unused]] inline constexpr color chocolate4{139, 69, 19};
        [[maybe_unused]] inline constexpr color firebrick1{255, 48, 48};
        [[maybe_unused]] inline constexpr color firebrick2{238, 44, 44};
        [[maybe_unused]] inline constexpr color firebrick3{205, 38, 38};
        [[maybe_unused]] inline constexpr color firebrick4{139, 26, 26};
        [[maybe_unused]] inline constexpr color brown1{255, 64, 64};
        [[maybe_unused]] inline constexpr color brown2{238, 59, 59};
        [[maybe_unused]] inline constexpr color brown3{205, 51, 51};
        [[maybe_unused]] inline constexpr color brown4{139, 35, 35};
        [[maybe_unused]] inline constexpr color salmon1{255, 140, 105};
        [[maybe_unused]] inline constexpr color salmon2{238, 130, 98};
        [[maybe_unused]] inline constexpr color salmon3{205, 112, 84};
        [[maybe_unused]] inline constexpr color salmon4{139, 76, 57};
        [[maybe_unused]] inline constexpr color LightSalmon1{255, 160, 122};
        [[maybe_unused]] inline constexpr color LightSalmon2{238, 149, 114};
        [[maybe_unused]] inline constexpr color LightSalmon3{205, 129, 98};
        [[maybe_unused]] inline constexpr color LightSalmon4{139, 87, 66};
        [[maybe_unused]] inline constexpr color orange1{255, 165, 0};
        [[maybe_unused]] inline constexpr color orange2{238, 154, 0};
        [[maybe_unused]] inline constexpr color orange3{205, 133, 0};
        [[maybe_unused]] inline constexpr color orange4{139, 90, 0};
        [[maybe_unused]] inline constexpr color DarkOrange1{255, 127, 0};
        [[maybe_unused]] inline constexpr color DarkOrange2{238, 118, 0};
        [[maybe_unused]] inline constexpr color DarkOrange3{205, 102, 0};
        [[maybe_unused]] inline constexpr color DarkOrange4{139, 69, 0};
        [[maybe_unused]] inline constexpr color coral1{255, 114, 86};
        [[maybe_unused]] inline constexpr color coral2{238, 106, 80};
        [[maybe_unused]] inline constexpr color coral3{205, 91, 69};
        [[maybe_unused]] inline constexpr color coral4{139, 62, 47};
        [[maybe_unused]] inline constexpr color tomato1{255, 99, 71};
        [[maybe_unused]] inline constexpr color tomato2{238, 92, 66};
        [[maybe_unused]] inline constexpr color tomato3{205, 79, 57};
        [[maybe_unused]] inline constexpr color tomato4{139, 54, 38};
        [[maybe_unused]] inline constexpr color OrangeRed1{255, 69, 0};
        [[maybe_unused]] inline constexpr color OrangeRed2{238, 64, 0};
        [[maybe_unused]] inline constexpr color OrangeRed3{205, 55, 0};
        [[maybe_unused]] inline constexpr color OrangeRed4{139, 37, 0};
        [[maybe_unused]] inline constexpr color red1{255, 0, 0};
        [[maybe_unused]] inline constexpr color red2{238, 0, 0};
        [[maybe_unused]] inline constexpr color red3{205, 0, 0};
        [[maybe_unused]] inline constexpr color red4{139, 0, 0};
        [[maybe_unused]] inline constexpr color DebianRed{215, 7, 81};
        [[maybe_unused]] inline constexpr color DeepPink1{255, 20, 147};
        [[maybe_unused]] inline constexpr color DeepPink2{238, 18, 137};
        [[maybe_unused]] inline constexpr color DeepPink3{205, 16, 118};
        [[maybe_unused]] inline constexpr color DeepPink4{139, 10, 80};
        [[maybe_unused]] inline constexpr color HotPink1{255, 110, 180};
        [[maybe_unused]] inline constexpr color HotPink2{238, 106, 167};
        [[maybe_unused]] inline constexpr color HotPink3{205, 96, 144};
        [[maybe_unused]] inline constexpr color HotPink4{139, 58, 98};
        [[maybe_unused]] inline constexpr color pink1{255, 181, 197};
        [[maybe_unused]] inline constexpr color pink2{238, 169, 184};
        [[maybe_unused]] inline constexpr color pink3{205, 145, 158};
        [[maybe_unused]] inline constexpr color pink4{139, 99, 108};
        [[maybe_unused]] inline constexpr color LightPink1{255, 174, 185};
        [[maybe_unused]] inline constexpr color LightPink2{238, 162, 173};
        [[maybe_unused]] inline constexpr color LightPink3{205, 140, 149};
        [[maybe_unused]] inline constexpr color LightPink4{139, 95, 101};
        [[maybe_unused]] inline constexpr color PaleVioletRed1{255, 130, 171};
        [[maybe_unused]] inline constexpr color PaleVioletRed2{238, 121, 159};
        [[maybe_unused]] inline constexpr color PaleVioletRed3{205, 104, 137};
        [[maybe_unused]] inline constexpr color PaleVioletRed4{139, 71, 93};
        [[maybe_unused]] inline constexpr color maroon1{255, 52, 179};
        [[maybe_unused]] inline constexpr color maroon2{238, 48, 167};
        [[maybe_unused]] inline constexpr color maroon3{205, 41, 144};
        [[maybe_unused]] inline constexpr color maroon4{139, 28, 98};
        [[maybe_unused]] inline constexpr color VioletRed1{255, 62, 150};
        [[maybe_unused]] inline constexpr color VioletRed2{238, 58, 140};
        [[maybe_unused]] inline constexpr color VioletRed3{205, 50, 120};
        [[maybe_unused]] inline constexpr color VioletRed4{139, 34, 82};
        [[maybe_unused]] inline constexpr color magenta1{255, 0, 255};
        [[maybe_unused]] inline constexpr color magenta2{238, 0, 238};
        [[maybe_unused]] inline constexpr color magenta3{205, 0, 205};
        [[maybe_unused]] inline constexpr color magenta4{139, 0, 139};
        [[maybe_unused]] inline constexpr color orchid1{255, 131, 250};
        [[maybe_unused]] inline constexpr color orchid2{238, 122, 233};
        [[maybe_unused]] inline constexpr color orchid3{205, 105, 201};
        [[maybe_unused]] inline constexpr color orchid4{139, 71, 137};
        [[maybe_unused]] inline constexpr color plum1{255, 187, 255};
        [[maybe_unused]] inline constexpr color plum2{238, 174, 238};
        [[maybe_unused]] inline constexpr color plum3{205, 150, 205};
        [[maybe_unused]] inline constexpr color plum4{139, 102, 139};
        [[maybe_unused]] inline constexpr color MediumOrchid1{224, 102, 255};
        [[maybe_unused]] inline constexpr color MediumOrchid2{209, 95, 238};
        [[maybe_unused]] inline constexpr color MediumOrchid3{180, 82, 205};
        [[maybe_unused]] inline constexpr color MediumOrchid4{122, 55, 139};
        [[maybe_unused]] inline constexpr color DarkOrchid1{191, 62, 255};
        [[maybe_unused]] inline constexpr color DarkOrchid2{178, 58, 238};
        [[maybe_unused]] inline constexpr color DarkOrchid3{154, 50, 205};
        [[maybe_unused]] inline constexpr color DarkOrchid4{104, 34, 139};
        [[maybe_unused]] inline constexpr color purple1{155, 48, 255};
        [[maybe_unused]] inline constexpr color purple2{145, 44, 238};
        [[maybe_unused]] inline constexpr color purple3{125, 38, 205};
        [[maybe_unused]] inline constexpr color purple4{85, 26, 139};
        [[maybe_unused]] inline constexpr color MediumPurple1{171, 130, 255};
        [[maybe_unused]] inline constexpr color MediumPurple2{159, 121, 238};
        [[maybe_unused]] inline constexpr color MediumPurple3{137, 104, 205};
        [[maybe_unused]] inline constexpr color MediumPurple4{93, 71, 139};
        [[maybe_unused]] inline constexpr color thistle1{255, 225, 255};
        [[maybe_unused]] inline constexpr color thistle2{238, 210, 238};
        [[maybe_unused]] inline constexpr color thistle3{205, 181, 205};
        [[maybe_unused]] inline constexpr color thistle4{139, 123, 139};
        [[maybe_unused]] inline constexpr color gray0{0, 0, 0};
        [[maybe_unused]] inline constexpr const color& grey0 = gray0;
        [[maybe_unused]] inline constexpr color gray1{3, 3, 3};
        [[maybe_unused]] inline constexpr const color& grey1 = gray1;
        [[maybe_unused]] inline constexpr color gray2{5, 5, 5};
        [[maybe_unused]] inline constexpr const color& grey2 = gray2;
        [[maybe_unused]] inline constexpr color gray3{8, 8, 8};
        [[maybe_unused]] inline constexpr const color& grey3 = gray3;
        [[maybe_unused]] inline constexpr color gray4{10, 10, 10};
        [[maybe_unused]] inline constexpr const color& grey4 = gray4;
        [[maybe_unused]] inline constexpr color gray5{13, 13, 13};
        [[maybe_unused]] inline constexpr const color& grey5 = gray5;
        [[maybe_unused]] inline constexpr color gray6{15, 15, 15};
        [[maybe_unused]] inline constexpr const color& grey6 = gray6;
        [[maybe_unused]] inline constexpr color gray7{18, 18, 18};
        [[maybe_unused]] inline constexpr const color& grey7 = gray7;
        [[maybe_unused]] inline constexpr color gray8{20, 20, 20};
        [[maybe_unused]] inline constexpr const color& grey8 = gray8;
        [[maybe_unused]] inline constexpr color gray9{23, 23, 23};
        [[maybe_unused]] inline constexpr const color& grey9 = gray9;
        [[maybe_unused]] inline constexpr color gray10{26, 26, 26};
        [[maybe_unused]] inline constexpr const color& grey10 = gray10;
        [[maybe_unused]] inline constexpr color gray11{28, 28, 28};
        [[maybe_unused]] inline constexpr const color& grey11 = gray11;
        [[maybe_unused]] inline constexpr color gray12{31, 31, 31};
        [[maybe_unused]] inline constexpr const color& grey12 = gray12;
        [[maybe_unused]] inline constexpr color gray13{33, 33, 33};
        [[maybe_unused]] inline constexpr const color& grey13 = gray13;
        [[maybe_unused]] inline constexpr color gray14{36, 36, 36};
        [[maybe_unused]] inline constexpr const color& grey14 = gray14;
        [[maybe_unused]] inline constexpr color gray15{38, 38, 38};
        [[maybe_unused]] inline constexpr const color& grey15 = gray15;
        [[maybe_unused]] inline constexpr color gray16{41, 41, 41};
        [[maybe_unused]] inline constexpr const color& grey16 = gray16;
        [[maybe_unused]] inline constexpr color gray17{43, 43, 43};
        [[maybe_unused]] inline constexpr const color& grey17 = gray17;
        [[maybe_unused]] inline constexpr color gray18{46, 46, 46};
        [[maybe_unused]] inline constexpr const color& grey18 = gray18;
        [[maybe_unused]] inline constexpr color gray19{48, 48, 48};
        [[maybe_unused]] inline constexpr const color& grey19 = gray19;
        [[maybe_unused]] inline constexpr color gray20{51, 51, 51};
        [[maybe_unused]] inline constexpr const color& grey20 = gray20;
        [[maybe_unused]] inline constexpr color gray21{54, 54, 54};
        [[maybe_unused]] inline constexpr const color& grey21 = gray21;
        [[maybe_unused]] inline constexpr color gray22{56, 56, 56};
        [[maybe_unused]] inline constexpr const color& grey22 = gray22;
        [[maybe_unused]] inline constexpr color gray23{59, 59, 59};
        [[maybe_unused]] inline constexpr const color& grey23 = gray23;
        [[maybe_unused]] inline constexpr color gray24{61, 61, 61};
        [[maybe_unused]] inline constexpr const color& grey24 = gray24;
        [[maybe_unused]] inline constexpr color gray25{64, 64, 64};
        [[maybe_unused]] inline constexpr const color& grey25 = gray25;
        [[maybe_unused]] inline constexpr color gray26{66, 66, 66};
        [[maybe_unused]] inline constexpr const color& grey26 = gray26;
        [[maybe_unused]] inline constexpr color gray27{69, 69, 69};
        [[maybe_unused]] inline constexpr const color& grey27 = gray27;
        [[maybe_unused]] inline constexpr color gray28{71, 71, 71};
        [[maybe_unused]] inline constexpr const color& grey28 = gray28;
        [[maybe_unused]] inline constexpr color gray29{74, 74, 74};
        [[maybe_unused]] inline constexpr const color& grey29 = gray29;
        [[maybe_unused]] inline constexpr color gray30{77, 77, 77};
        [[maybe_unused]] inline constexpr const color& grey30 = gray30;
        [[maybe_unused]] inline constexpr color gray31{79, 79, 79};
        [[maybe_unused]] inline constexpr const color& grey31 = gray31;
        [[maybe_unused]] inline constexpr color gray32{82, 82, 82};
        [[maybe_unused]] inline constexpr const color& grey32 = gray32;
        [[maybe_unused]] inline constexpr color gray33{84, 84, 84};
        [[maybe_unused]] inline constexpr const color& grey33 = gray33;
        [[maybe_unused]] inline constexpr color gray34{87, 87, 87};
        [[maybe_unused]] inline constexpr const color& grey34 = gray34;
        [[maybe_unused]] inline constexpr color gray35{89, 89, 89};
        [[maybe_unused]] inline constexpr const color& grey35 = gray35;
        [[maybe_unused]] inline constexpr color gray36{92, 92, 92};
        [[maybe_unused]] inline constexpr const color& grey36 = gray36;
        [[maybe_unused]] inline constexpr color gray37{94, 94, 94};
        [[maybe_unused]] inline constexpr const color& grey37 = gray37;
        [[maybe_unused]] inline constexpr color gray38{97, 97, 97};
        [[maybe_unused]] inline constexpr const color& grey38 = gray38;
        [[maybe_unused]] inline constexpr color gray39{99, 99, 99};
        [[maybe_unused]] inline constexpr const color& grey39 = gray39;
        [[maybe_unused]] inline constexpr color gray40{102, 102, 102};
        [[maybe_unused]] inline constexpr const color& grey40 = gray40;
        [[maybe_unused]] inline constexpr color gray41{105, 105, 105};
        [[maybe_unused]] inline constexpr const color& grey41 = gray41;
        [[maybe_unused]] inline constexpr color gray42{107, 107, 107};
        [[maybe_unused]] inline constexpr const color& grey42 = gray42;
        [[maybe_unused]] inline constexpr color gray43{110, 110, 110};
        [[maybe_unused]] inline constexpr const color& grey43 = gray43;
        [[maybe_unused]] inline constexpr color gray44{112, 112, 112};
        [[maybe_unused]] inline constexpr const color& grey44 = gray44;
        [[maybe_unused]] inline constexpr color gray45{115, 115, 115};
        [[maybe_unused]] inline constexpr const color& grey45 = gray45;
        [[maybe_unused]] inline constexpr color gray46{117, 117, 117};
        [[maybe_unused]] inline constexpr const color& grey46 = gray46;
        [[maybe_unused]] inline constexpr color gray47{120, 120, 120};
        [[maybe_unused]] inline constexpr const color& grey47 = gray47;
        [[maybe_unused]] inline constexpr color gray48{122, 122, 122};
        [[maybe_unused]] inline constexpr const color& grey48 = gray48;
        [[maybe_unused]] inline constexpr color gray49{125, 125, 125};
        [[maybe_unused]] inline constexpr const color& grey49 = gray49;
        [[maybe_unused]] inline constexpr color gray50{127, 127, 127};
        [[maybe_unused]] inline constexpr const color& grey50 = gray50;
        [[maybe_unused]] inline constexpr color gray51{130, 130, 130};
        [[maybe_unused]] inline constexpr const color& grey51 = gray51;
        [[maybe_unused]] inline constexpr color gray52{133, 133, 133};
        [[maybe_unused]] inline constexpr const color& grey52 = gray52;
        [[maybe_unused]] inline constexpr color gray53{135, 135, 135};
        [[maybe_unused]] inline constexpr const color& grey53 = gray53;
        [[maybe_unused]] inline constexpr color gray54{138, 138, 138};
        [[maybe_unused]] inline constexpr const color& grey54 = gray54;
        [[maybe_unused]] inline constexpr color gray55{140, 140, 140};
        [[maybe_unused]] inline constexpr const color& grey55 = gray55;
        [[maybe_unused]] inline constexpr color gray56{143, 143, 143};
        [[maybe_unused]] inline constexpr const color& grey56 = gray56;
        [[maybe_unused]] inline constexpr color gray57{145, 145, 145};
        [[maybe_unused]] inline constexpr const color& grey57 = gray57;
        [[maybe_unused]] inline constexpr color gray58{148, 148, 148};
        [[maybe_unused]] inline constexpr const color& grey58 = gray58;
        [[maybe_unused]] inline constexpr color gray59{150, 150, 150};
        [[maybe_unused]] inline constexpr const color& grey59 = gray59;
        [[maybe_unused]] inline constexpr color gray60{153, 153, 153};
        [[maybe_unused]] inline constexpr const color& grey60 = gray60;
        [[maybe_unused]] inline constexpr color gray61{156, 156, 156};
        [[maybe_unused]] inline constexpr const color& grey61 = gray61;
        [[maybe_unused]] inline constexpr color gray62{158, 158, 158};
        [[maybe_unused]] inline constexpr const color& grey62 = gray62;
        [[maybe_unused]] inline constexpr color gray63{161, 161, 161};
        [[maybe_unused]] inline constexpr const color& grey63 = gray63;
        [[maybe_unused]] inline constexpr color gray64{163, 163, 163};
        [[maybe_unused]] inline constexpr const color& grey64 = gray64;
        [[maybe_unused]] inline constexpr color gray65{166, 166, 166};
        [[maybe_unused]] inline constexpr const color& grey65 = gray65;
        [[maybe_unused]] inline constexpr color gray66{168, 168, 168};
        [[maybe_unused]] inline constexpr const color& grey66 = gray66;
        [[maybe_unused]] inline constexpr color gray67{171, 171, 171};
        [[maybe_unused]] inline constexpr const color& grey67 = gray67;
        [[maybe_unused]] inline constexpr color gray68{173, 173, 173};
        [[maybe_unused]] inline constexpr const color& grey68 = gray68;
        [[maybe_unused]] inline constexpr color gray69{176, 176, 176};
        [[maybe_unused]] inline constexpr const color& grey69 = gray69;
        [[maybe_unused]] inline constexpr color gray70{179, 179, 179};
        [[maybe_unused]] inline constexpr const color& grey70 = gray70;
        [[maybe_unused]] inline constexpr color gray71{181, 181, 181};
        [[maybe_unused]] inline constexpr const color& grey71 = gray71;
        [[maybe_unused]] inline constexpr color gray72{184, 184, 184};
        [[maybe_unused]] inline constexpr const color& grey72 = gray72;
        [[maybe_unused]] inline constexpr color gray73{186, 186, 186};
        [[maybe_unused]] inline constexpr const color& grey73 = gray73;
        [[maybe_unused]] inline constexpr color gray74{189, 189, 189};
        [[maybe_unused]] inline constexpr const color& grey74 = gray74;
        [[maybe_unused]] inline constexpr color gray75{191, 191, 191};
        [[maybe_unused]] inline constexpr const color& grey75 = gray75;
        [[maybe_unused]] inline constexpr color gray76{194, 194, 194};
        [[maybe_unused]] inline constexpr const color& grey76 = gray76;
        [[maybe_unused]] inline constexpr color gray77{196, 196, 196};
        [[maybe_unused]] inline constexpr const color& grey77 = gray77;
        [[maybe_unused]] inline constexpr color gray78{199, 199, 199};
        [[maybe_unused]] inline constexpr const color& grey78 = gray78;
        [[maybe_unused]] inline constexpr color gray79{201, 201, 201};
        [[maybe_unused]] inline constexpr const color& grey79 = gray79;
        [[maybe_unused]] inline constexpr color gray80{204, 204, 204};
        [[maybe_unused]] inline constexpr const color& grey80 = gray80;
        [[maybe_unused]] inline constexpr color gray81{207, 207, 207};
        [[maybe_unused]] inline constexpr const color& grey81 = gray81;
        [[maybe_unused]] inline constexpr color gray82{209, 209, 209};
        [[maybe_unused]] inline constexpr const color& grey82 = gray82;
        [[maybe_unused]] inline constexpr color gray83{212, 212, 212};
        [[maybe_unused]] inline constexpr const color& grey83 = gray83;
        [[maybe_unused]] inline constexpr color gray84{214, 214, 214};
        [[maybe_unused]] inline constexpr const color& grey84 = gray84;
        [[maybe_unused]] inline constexpr color gray85{217, 217, 217};
        [[maybe_unused]] inline constexpr const color& grey85 = gray85;
        [[maybe_unused]] inline constexpr color gray86{219, 219, 219};
        [[maybe_unused]] inline constexpr const color& grey86 = gray86;
        [[maybe_unused]] inline constexpr color gray87{222, 222, 222};
        [[maybe_unused]] inline constexpr const color& grey87 = gray87;
        [[maybe_unused]] inline constexpr color gray88{224, 224, 224};
        [[maybe_unused]] inline constexpr const color& grey88 = gray88;
        [[maybe_unused]] inline constexpr color gray89{227, 227, 227};
        [[maybe_unused]] inline constexpr const color& grey89 = gray89;
        [[maybe_unused]] inline constexpr color gray90{229, 229, 229};
        [[maybe_unused]] inline constexpr const color& grey90 = gray90;
        [[maybe_unused]] inline constexpr color gray91{232, 232, 232};
        [[maybe_unused]] inline constexpr const color& grey91 = gray91;
        [[maybe_unused]] inline constexpr color gray92{235, 235, 235};
        [[maybe_unused]] inline constexpr const color& grey92 = gray92;
        [[maybe_unused]] inline constexpr color gray93{237, 237, 237};
        [[maybe_unused]] inline constexpr const color& grey93 = gray93;
        [[maybe_unused]] inline constexpr color gray94{240, 240, 240};
        [[maybe_unused]] inline constexpr const color& grey94 = gray94;
        [[maybe_unused]] inline constexpr color gray95{242, 242, 242};
        [[maybe_unused]] inline constexpr const color& grey95 = gray95;
        [[maybe_unused]] inline constexpr color gray96{245, 245, 245};
        [[maybe_unused]] inline constexpr const color& grey96 = gray96;
        [[maybe_unused]] inline constexpr color gray97{247, 247, 247};
        [[maybe_unused]] inline constexpr const color& grey97 = gray97;
        [[maybe_unused]] inline constexpr color gray98{250, 250, 250};
        [[maybe_unused]] inline constexpr const color& grey98 = gray98;
        [[maybe_unused]] inline constexpr color gray99{252, 252, 252};
        [[maybe_unused]] inline constexpr const color& grey99 = gray99;
        [[maybe_unused]] inline constexpr color gray100{255, 255, 255};
        [[maybe_unused]] inline constexpr const color& grey100 = gray100;
        [[maybe_unused]] inline constexpr color DarkGray{169, 169, 169};
        [[maybe_unused]] inline constexpr const color& DarkGrey = DarkGray;
        [[maybe_unused]] inline constexpr color DarkBlue{0, 0, 139};
        [[maybe_unused]] inline constexpr color DarkCyan{0, 139, 139};
        [[maybe_unused]] inline constexpr color DarkMagenta{139, 0, 139};
        [[maybe_unused]] inline constexpr color DarkRed{139, 0, 0};
        [[maybe_unused]] inline constexpr color LightGreen{144, 238, 144};

        // Colour Pallet: Red/Blue Candy
        [[maybe_unused]] inline constexpr color dark_teal{13, 71, 81};
        [[maybe_unused]] inline constexpr color bright_teal{66, 179, 201};
        [[maybe_unused]] inline constexpr color pale_teal{171, 220, 215};
        [[maybe_unused]] inline constexpr color light_peach{236, 234, 220};
        [[maybe_unused]] inline constexpr color bright_red{227, 58, 55};

        // Ubuntu
        [[maybe_unused]] inline constexpr color terminal_background{56, 12, 42};
    }
}

#endif
