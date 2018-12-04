/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Naskove 1, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:teng@firma.seznam.cz
 *
 *
 * $Id: $
 *
 * DESCRIPTION
 * Teng engine -- fuzzer main.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-11-30  (burlog)
 *             Created.
 */

#include <fstream>
#include <iostream>

#include <teng/teng.h>

extern "C" {

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string string(reinterpret_cast<const char *>(data), size);

    std::string result;
    Teng::Teng_t teng;
    Teng::Error_t err;
    Teng::StringWriter_t writer(result);

    Teng::Fragment_t root;
    auto &data_frag = root.addFragment("data");
    data_frag.addFragment("current_section");
    for (int i = 0; i < 10; ++i) {
        auto &sub_frag = data_frag.addFragment("subsections");
        sub_frag.addVariable("thread_count", i);
        sub_frag.addVariable("icon_url", "http:://icon.url");
        for (int j = 0; j < 3; ++j) {
            auto &n_frag = sub_frag.addFragment("topNThreads");
            n_frag.addVariable("answer_count", 10);
            if (j > 1) {
                auto &profile_frag = n_frag.addFragment("profile");
                profile_frag.addVariable("image", "aaa");
            }
        }
    }

    Teng::Teng_t::GenPageArgs_t args;
    args.contentType = "text/html";
    args.encoding = "utf-8";
    args.templateString = std::move(string);

    teng.generatePage(args, root, writer, err);
    return 0;
}

} // extern "C"

#ifdef FUZZ_MAIN

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cerr << "Usage: ./fuzz artifact-file" << std::endl;
        return -1;
    }
    std::ifstream artifact(argv[1]);
    if (!artifact) {
        std::cerr << "can't open file: " << argv[1] << std::endl;
        return -2;
    }

    std::string data;
    artifact.seekg(0, std::ios::end);
    data.resize(artifact.tellg());
    artifact.seekg(0, std::ios::beg);
    artifact.read(&data[0], data.size());


    auto ptr = reinterpret_cast<const uint8_t *>(data.data());
    LLVMFuzzerTestOneInput(ptr, data.size());
    return EXIT_SUCCESS;
}

#endif /* FUZZ_MAIN */

