#include <teng.h>

#include <stdio.h>
#include <string>

int main(int argc, char * argv[]) {
    static std::string characters[2] = { "A", "B" };

    // Define some template
    std::string templ = "<html>\n\
    <head>\n\
        <title>Example page</title>\n\
    </head>\n\
    <body>\n\
        #{_tld}\n\
        <?teng frag row?><p>${rnum}\n\
            <?teng frag col?>${cnum} <?teng endfrag?>\n\
        </p><?teng endfrag?>\n\
    </body>\n\
</html>\n";

    // Create Teng engine
    Teng::Teng_t teng("", Teng::Teng_t::LM_LOG_TO_OUTPUT);

    // Root data fragment
    Teng::Fragment_t root;

    // Rows fragment list
    Teng::FragmentList_t &rowList = root.addFragmentList("row");

    // Create two rows
    for(int i = 0; i < 2; i++) {
        Teng::Fragment_t &row = rowList.addFragment();
        // Add variable ‘rnum’ (‘A’ or ‘B’)
        row.addVariable("rnum", characters[i]);
        // Create two columns
        for(int j = 1; j <= 2; j++) {
            Teng::Fragment_t &col = row.addFragment("col");
            // Add variable ‘cnum’ into ‘row’ (‘1’ or ‘2’)
            col.addVariable("cnum", (long int)j);
        }
    }

    // Output to standard output
    Teng::FileWriter_t writer(stdout);

    // Simple error log
    Teng::Error_t err;

    // Generate page
    auto res = teng.generatePage(
        templ, // Template
        "", // Dictionary (none)
        "", // Language (none)
        "", // Configuration (none)
        "text/html", // Content type
        "utf-8", // Encoding
        root, // Root fragment
        writer, // Writer
        err // Error log
    );

    for (auto &line: err.getEntries())
        std::cerr << line.getLogLine() << std::endl;

    return res;
}
