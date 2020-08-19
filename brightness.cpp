#include <iostream>
#include <fstream>

#include <experimental/filesystem>
#include <cmath>

/*
 * TODO
 * Handle arg passing
 * Handle bad values of VALUE
 */

namespace fs = std::experimental::filesystem;

const char *base = "/sys/class/backlight/";
const float min_percent_frac = 0.01;
const float exp_factor = 40; // 50;
// TODO: constexpr
const float denom = std::exp(100 / exp_factor) - 1;

const std::string newline = "\n";
const std::string usage = \
    "Usage: brightness [OPTION] [NEW_VALUE]" + newline +
    "Used to get and set the brightness of backlight devices." + newline +
    "" + newline +
    "Mandatory arguments to long options are mandatory for short options too." + newline +
    "    -d, --device=DEV	specify the device name," + newline +
    "			  if none give, uses the first device" + newline +
    "    -h, --help		display this help and exit" + newline +
    "    -m, --max		display max brightness values too" + newline +
    "" + newline +
    "VALUE should be of the form:" + newline +
    "    [+,-]NUMBER[%]" + newline +
    "" + newline +
    "";


std::tuple<int, int> get(std::string path) {
    if ((char) path.back() != '/')
        path += "/";
    int cur, max;
    std::ifstream(path + "brightness") >> cur;
    std::ifstream(path + "max_brightness") >> max;
    return std::make_tuple(cur, max);
}

int percent_to_brightness(float percent, int max_brightness) {
    return static_cast<int>((std::exp(percent / 100) - 1) / denom * max_brightness);
}

float brightness_to_percent(float brightness, int max_brightness) {
    return std::log((brightness / max_brightness) * denom + 1) * 100;
}

void display(std::string path, const std::string &device, bool display_max = false) {
    // TODO: remove display_max;
    auto[cur, max] = get(path);
    // std::cout << device << ":\t" << 100*cur / max << "%"  << (display_max? " of " + to_std::string(max) : "") << std::endl;
    std::cout << device << ":\t" << (int) brightness_to_percent(cur, max) << "%" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc == 0) return 1; // Should never happen.

    std::string value;  // brightness value. should be -1 if unset
    if (argc == 2) value = argv[1];

    std::string device_spec; // should be set by options
    bool display_max = false;

    if ((!value.empty()) && (device_spec.length() == 0))
        device_spec = "_";

    std::string path;
    std::string device;
    for (const auto &entry : fs::directory_iterator(base)) {
        path = entry.path();
        device = path.substr(path.find_last_of('/') + 1);
        if (value.empty()) display(path, device, display_max);
        if ((device_spec == "_") || (device == device_spec)) break;
        path = "";
    }
    if (value.empty()) return 0; // End of default mode.
    if (path.length() == 0) return 1; // Device not found.

    path += "/";
    auto[cur, max] = get(path);
    int new_value = 0;
    if ((char) value.back() == '%') {
        switch (value[0]) {
            case '+' :
            case '-' :
                new_value = percent_to_brightness(brightness_to_percent(cur, max) + stoi(value), max);
                break;
            default:
                new_value = percent_to_brightness(stoi(value), max);
        }
    } else {
        switch (value[0]) { // value.back() vs value[value.length() - 1]
            case '+' :
            case '-' :
                new_value = cur + stoi(value);
                break;
            default:
                new_value = stoi(value);
        }
    }
    if (new_value < min_percent_frac * max) new_value = min_percent_frac * max;
    if (new_value > max) new_value = max;

    std::ofstream(path + "brightness") << new_value;
    display(path, device);

    return 0;
}
