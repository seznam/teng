#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <teng.h>

void gen_page(
    const Teng::Teng_t &teng,
    const std::string &t,
    const Teng::Fragment_t &root,
    Teng::Writer_t &output,
    Teng::Error_t &err,
    const std::string &params_file,
    const std::string &dict_file
) {
    teng.generatePage(
        t,
        dict_file,
        "",
        params_file,
        "text/html",
        "utf-8",
        root,
        output,
        err
    );

    std::cerr << "ERRORS(" << err.getEntries().size() << ")" << std::endl;
    for (auto &line: err.getEntries())
        std::cerr << line.getLogLine();

}

int main(int argc, const char *argv[]) {
    Teng::Teng_t teng;
    Teng::FileWriter_t writer(stdout);
    Teng::Error_t err;
    std::string t = "${var}";

    if (argc != 3) {
        std::cerr << "Usage: cache config_filename dict_filename" << std::endl;
        return EXIT_FAILURE;
    }

    std::string params_file = argv[1];
    std::string dict_file = argv[2];

    {
        Teng::Fragment_t root;
        root.addVariable("var", "var1");
        gen_page(teng, t, root, writer, err, params_file, dict_file);
    }

    {
        Teng::Fragment_t root;
        root.addVariable("var", "var2");
        gen_page(teng, t, root, writer, err, params_file, dict_file);
    }

    std::fstream a(dict_file);
    a.seekp(0, std::ios::end);
    a << "\n";
    a.close();

    {
        Teng::Fragment_t root;
        root.addVariable("var", "var3");
        gen_page(teng, t, root, writer, err, params_file, dict_file);
    }

    std::fstream b(params_file);
    b.seekp(0, std::ios::end);
    b << "\n";
    b.close();

    {
        Teng::Fragment_t root;
        root.addVariable("var", "var4");
        gen_page(teng, t, root, writer, err, params_file, dict_file);
    }
}

