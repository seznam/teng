#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <teng.h>

void gen_page(
    const Teng::Teng_t &teng,
    const Teng::Teng_t::GenPageArgs_t &args,
    const Teng::Fragment_t &root,
    Teng::Writer_t &output,
    Teng::Error_t &err
) {
    teng.generatePage(args, root, output, err);
    std::cerr << "ERRORS(" << err.getEntries().size() << ")" << std::endl;
    for (auto &line: err.getEntries())
        std::cerr << line.getLogLine();

}

int main(int argc, const char *argv[]) {
    Teng::Teng_t teng;
    Teng::FileWriter_t writer(stdout);
    Teng::Error_t err;
    Teng::Teng_t::GenPageArgs_t args;
    args.templateString = "${var}";

    if (argc != 3) {
        std::cerr << "Usage: cache config_filename dict_filename" << std::endl;
        return EXIT_FAILURE;
    }

    args.paramsFilename = argv[1];
    args.dictFilename = argv[2];

    {
        Teng::Fragment_t root;
        root.addVariable("var", "var1");
        gen_page(teng, args, root, writer, err);
    }

    {
        Teng::Fragment_t root;
        root.addVariable("var", "var2");
        gen_page(teng, args, root, writer, err);
    }

    std::fstream a(args.dictFilename);
    a.seekp(0, std::ios::end);
    a << "\n";
    a.close();

    {
        Teng::Fragment_t root;
        root.addVariable("var", "var3");
        gen_page(teng, args, root, writer, err);
    }

    std::fstream b(args.paramsFilename);
    b.seekp(0, std::ios::end);
    b << "\n";
    b.close();

    {
        Teng::Fragment_t root;
        root.addVariable("var", "var4");
        gen_page(teng, args, root, writer, err);
    }
}

