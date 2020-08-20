#include <iostream>
#include <fstream>

#include <experimental/filesystem>
#include <cmath>
#include <cstring>

/*
 * TODO
 * Handle arg passing
 * Handle bad values of VALUE
 */

namespace fs = std::experimental::filesystem;

const char BASE_PATH[] = "/sys/class/backlight/";
const float MIN_PERCENT_FRAC = 0.001;

// TODO: constexpr
//const float exp_factor = 40; // 50;
//const float denom = std::exp(100 / exp_factor) - 1;
const float log_factor = 2;
static const float logged_factor = std::log(1 + log_factor);

const char USAGE[] = \
    "Usage: brightness [OPTIONS] [NEW_VALUE]\n"
    "Used to get_brightness and set the brightness of backlight devices.\n"
    "\n"
    "Mandatory arguments to long options are mandatory for short options too.\n"
    "    -d, --device=DEV	specify the device name,\n"
    "			            if none give, uses the first available device\n"
    "    -h, --help		display this help and exit\n"
    "\n"
    "NEW_VALUE should be of the form:\n"
    "    [+,-]NUMBER[%]\n";


std::string_view get_device(const std::string &path) {
    auto dev = std::string_view(path);
    dev.remove_prefix(path.find_last_of('/') + 1);
    return dev;
}

std::string get_path(const std::string &device_spec) {
    for (const auto &entry : fs::directory_iterator(BASE_PATH)) {
        auto path = entry.path().string();

        if (device_spec.empty() || (get_device(path) == device_spec))
            return path;
    }
    return {};
}

std::pair<int, int> get_brightness(std::string path) {
    int cur, max;
    if ((char) path.back() == '/') {
        std::ifstream(path + "brightness") >> cur;
        std::ifstream(path + "max_brightness") >> max;
    } else {
        std::ifstream(path + "/brightness") >> cur;
        std::ifstream(path + "/max_brightness") >> max;
    }
    return std::make_pair(cur, max);
}

int percent_to_brightness(float percent, int max_brightness) {
    if (percent > 1) percent /= 100; // Dirty hack for xx% and 0.xx

    return static_cast<int>(static_cast<float>(max_brightness) * std::log(1 + log_factor * percent) / logged_factor);
}

float brightness_to_percent(float brightness, int max_brightness) {
    return (std::exp(logged_factor * brightness / static_cast<float>(max_brightness)) - 1) / log_factor;
}

void display(const std::string &path) {
    auto device = get_device(path);
    auto[cur, max] = get_brightness(path);
    std::cout << device << ":\t" << static_cast<int>(std::ceil(100 * brightness_to_percent(cur, max))) << "%"
              << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc == 0) return 1; // Should never happen.

    std::string value;  // brightness value. should be -1 if unset
    std::string device;
    bool help;

    std::string *dummy = &value;
    for (size_t i = 1; i < argc; ++i) {
        if (std::strlen(argv[i]) == 0) continue;
        if (argv[i][0] != '-') {
            *dummy = argv[i];
            dummy = &value;
            continue;
        }

        if (std::strlen(argv[i]) < 1) continue;

        if (argv[i][1] != '-') { // Short form option

            // Special case because we use -10% as a value
            if ('0' <= argv[i][1] && argv[i][1] <= '9') {
                value = argv[i];
                continue;
            }

            // Short form options
            for (size_t j = 1; j < std::strlen(argv[i]); ++j) {
                switch (argv[i][j]) {
                    case 'h':
                        help = true;
                        break;
                    case 'd':
                        dummy = &device;
                        break;
                    default:
                        std::cout << "Invalid option '" << argv[i][j] << "'." << std::endl;
                        return 1;
                }
            }
            continue;
        }

        // Long form options
        std::string_view opt(argv[i]), val(argv[i]);
        auto pos = opt.rfind('=');
        if (pos != std::string::npos) {
            opt.remove_suffix(opt.length() - pos);
            val.remove_prefix(pos + 1);
        } else {
            val.remove_prefix(val.length());
        };
        opt.remove_prefix(2);

        if (opt == "help")
            help = true;
        else if (opt == "device")
            device = val;
        else {
            std::cout << "Invalid option '" << opt << "'." << std::endl;
            return 1;
        }
    }

    if (help) {
        std::cout << USAGE;
        return 0;
    }

    /* if (show_all) {
     *  //do
     *  return 0]
     *  }
     */


    std::string path = get_path(device);

    if (path.empty()) {
        if (device.empty())
            std::cout << "No backlight found." << std::endl;
        else
            std::cout << "Backlight '" << device << "' not found." << std::endl;
        return 1;
    }

    if (value.empty()) {
        display(path);
        return 0; // End of default mode.
    }

    auto[cur, max] = get_brightness(path);

    int new_value = 0;
    if ((char) value.back() == '%') {
        switch (value[0]) {
            case '+' :
            case '-' :
                new_value = percent_to_brightness(100 * brightness_to_percent(cur, max) + std::stof(value), max);
                break;
            default:
                new_value = percent_to_brightness(std::stof(value), max);
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

    if (new_value < MIN_PERCENT_FRAC * max) new_value = MIN_PERCENT_FRAC * max;
    if (new_value > max) new_value = max;

    std::ofstream(path + "/brightness") << new_value;

    display(path);
    return 0;
}
