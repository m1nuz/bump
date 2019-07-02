#pragma once

#include <tuple>
#include <string>
#include <vector>
#include <functional>

#include <regex>

namespace xargs {
    struct args {
        using handler_no_value_type = std::function<void ()>;
        using handler_arg_type = std::function<bool (const std::string& )>;
        //using handler_args_type = std::function<void (const std::vector<std::string>& )>;
        using option_no_value_type = std::tuple<std::string, std::string, handler_no_value_type>;
        using options_type = std::tuple<std::string, std::string, handler_arg_type>;
        using arg_type = std::tuple<std::string, std::string, handler_arg_type> ;
        using args_type = std::tuple<std::string, std::vector<std::string>>;

        args& add_option(const std::string &opt, const std::string &help, handler_arg_type h) {
            options_with_values.emplace_back(std::make_tuple(opt, help, h));
            return *this;
        }

        args& add_option(const std::string &opt, const std::string &help, handler_no_value_type h) {
            options_no_values.emplace_back(std::make_tuple(opt, help, h));
            return *this;
        }

        args& add_arg(const std::string &arg, const std::string &desc, handler_arg_type h) {
            all_args.emplace_back(std::make_tuple(arg, desc, h));
            return *this;
        }

//        args& add_args(const std::string &arg_list, const std::string &desc, handler_args_type h) {
//            return *this;
//        }

        auto count() const {
            return all_args.size() + 1;
        }

        void dispath(int argc, char *argv[]) {
            const auto compare = [] (const auto &a, const auto &b) {
                return std::get<0>(a).compare(std::get<0>(b));
            };

            args_usage.resize( argc );

            std::sort(options_with_values.begin(), options_with_values.end(), compare);
            std::sort(options_no_values.begin(), options_no_values.end(), compare);

            int p = 1;
            for (int i = 0; i < argc; i++) {
                for (const auto &o : options_with_values) {
                    if (std::get<0>(o).compare(argv[i]) == 0 && ((i + 1) < argc) && argv[i + 1][0] != '-') {
                        std::get<2>(o)(argv[i + 1]);
                        args_usage[i] = true;
                        p += 2;
                    }
                }

                for (const auto &o : options_no_values) {
                    if (std::get<0>(o).compare(argv[i]) == 0) {
                        std::get<2>(o)();
                        args_usage[i] = true;
                        p++;
                    }
                }
            }

            for (int i = 1; i < argc; i++) {
                for (const auto& arg : all_args) {
                    if (!args_usage[i]) {
                        args_usage[i] = std::get<2>(arg)(argv[i]);
                        break;
                    }
                }
            }
        }

        auto usage(const std::string &path) {
            const auto pos = path.find_last_of('/');
            const auto leaf = path.substr(pos + 1);

            std::string args_text;
            for (const auto &a : all_args)
                args_text += std::get<0>(a) + ' ';

            const auto have_options = !options_with_values.empty() && !options_no_values.empty();

            std::string text{"Usage: " + (have_options ? leaf + " [options] " : leaf + " ") + args_text};

            for (const auto &a : all_args)
                if (!std::get<1>(a).empty())
                    text += "\n\t" + std::get<0>(a) + " " + std::get<1>(a);

            if (have_options)
                text += "\nOptions:";

            for (const auto &o : options_with_values)
                text += "\n\t" + std::get<0>(o) + " " + std::get<1>(o);

            for (const auto &o : options_no_values)
                text += "\n\t" + std::get<0>(o) + " " + std::get<1>(o);

            return text;
        }

        std::vector<option_no_value_type> options_no_values;
        std::vector<options_type> options_with_values;
        std::vector<arg_type> all_args;
        args_type other_args;
        std::vector<bool> args_usage;
    };
}
