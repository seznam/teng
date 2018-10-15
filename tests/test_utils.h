#include <assert.h>
#include <boost/test/unit_test.hpp>
#include <memory>
#include <stdio.h>
#include <string>
#include <teng.h>
#include <tengfilesystem.h>

inline auto createFragment()
{
    auto root = std::make_unique<Teng::Fragment_t>();
    return root;
}

inline auto createFragmentTitle()
{
    auto root = std::make_unique<Teng::Fragment_t>();
    root->addVariable("title", "Title");
    return root;
}

inline std::string generate(Teng::Teng_t& teng,
                            const std::string& templ,
                            const Teng::Fragment_t& root,
                            Teng::Error_t& err)
{
    std::string result;
    Teng::StringWriter_t writer(result);

    bool res = teng.generatePage(templ,       // Template
                                 "",          // Dictionary (none)
                                 "",          // Language (none)
                                 "",          // Configuration (none)
                                 "text/html", // Content type
                                 "utf-8",     // Encoding
                                 root,        // Root fragment
                                 writer,      // Writer
                                 err          // Error log
                                 )
        == 0;

    if (!res) {
        err.dump(std::cerr);
        std::runtime_error("Failed to generate page.");
    }
    return result;
}

inline std::string generateFromFile(Teng::Teng_t& teng,
                                    const std::string& filename,
                                    const Teng::Fragment_t& root,
                                    Teng::Error_t& err)
{
    std::string result;
    Teng::StringWriter_t writer(result);

    bool res = teng.generatePage(filename,    // Template
                                 "",          // Skin
                                 "",          // Dictionary (none)
                                 "",          // Language (none)
                                 "",          // Configuration (none)
                                 "text/html", // Content type
                                 "utf-8",     // Encoding
                                 root,        // Root fragment
                                 writer,      // Writer
                                 err          // Error log
                                 )
        == 0;

    if (!res) {
        err.dump(std::cerr);
        std::runtime_error("Failed to generate page.");
    }
    return result;
}
