#define BOOST_TEST_MODULE base_tests

#include "test_utils.h"

#include <boost/filesystem.hpp>

const std::string result_1 = R"__(    <head>
        <title>Example page: Title</title>
    </head>)__";

const std::string result_2 = R"__(    <head>
        <title>Example page: </title>
    </head>)__";

const std::string result_3 = R"__(<html>
    <head>
        <title>Example page: Title</title>
    </head>
    <body>
        <p>Paragraph: Title</p>
    </body>
</html>)__";

const std::string result_4 = R"__(<html>
    <head>
        <title>Example page: </title>
    </head>
    <body>
        <p>Paragraph: </p>
    </body>
</html>)__";

std::string createAbsPath(const std::string& root, const std::string& filename)
{
    return (!root.empty() && root[0] == '/')
        ? (boost::filesystem::path(root) / filename).string()
        : (boost::filesystem::current_path() / root / filename).string();
}

BOOST_AUTO_TEST_CASE(test_include_ok)
{
    Teng::Teng_t teng("templates", 0);
    Teng::Error_t err;

    BOOST_CHECK_EQUAL(
        generate(teng, "<?teng include file=\"common-head.html\" ?>", *createFragmentTitle(), err),
        result_1);
    BOOST_CHECK_EQUAL(err.count(), 0);
}

BOOST_AUTO_TEST_CASE(test_include_warning)
{
    Teng::Teng_t teng("templates", 0);
    Teng::Error_t err;

    BOOST_CHECK_EQUAL(
        generate(teng, "<?teng include file=\"common-head.html\" ?>", *createFragment(), err),
        result_2);
    BOOST_REQUIRE_EQUAL(err.count(), 1);
    BOOST_CHECK_EQUAL(err.getEntries().front().getLogLine(),
                      "common-head.html(2,37) Warning: Runtime: Variable '.title' is undefined\n");
}

BOOST_AUTO_TEST_CASE(test_include_missing)
{
    Teng::Teng_t teng("no-directory", 0);
    Teng::Error_t err;

    const std::string absPath = createAbsPath("no-directory", "common-head.html");

    BOOST_CHECK_EQUAL(
        generate(teng, "<?teng include file=\"common-head.html\" ?>", *createFragmentTitle(), err),
        "");
    BOOST_REQUIRE_EQUAL(err.count(), 2);
    BOOST_CHECK_EQUAL(err.getEntries()[0].getLogLine(),
                      "(no file)(1,41) Error: Cannot open input file '" + absPath
                          + "' (No such file or directory)\n");
    BOOST_CHECK_EQUAL(err.getEntries()[1].getLogLine(),
                      "common-head.html(1,0) Error: Cannot stat file '" + absPath
                          + "' (No such file or directory)\n");
}

BOOST_AUTO_TEST_CASE(test_file_ok)
{
    Teng::Teng_t teng("templates", 0);
    Teng::Error_t err;

    BOOST_CHECK_EQUAL(generateFromFile(teng, "common-head.html", *createFragmentTitle(), err),
                      result_1);
    BOOST_REQUIRE_EQUAL(err.count(), 0);
}

BOOST_AUTO_TEST_CASE(test_file_missing)
{
    Teng::Teng_t teng("no-directory", 0);
    Teng::Error_t err;

    const std::string absPath = createAbsPath("no-directory", "common-head.html");

    BOOST_CHECK_EQUAL(generateFromFile(teng, "common-head.html", *createFragmentTitle(), err), "");
    BOOST_REQUIRE_EQUAL(err.count(), 2);
    BOOST_CHECK_EQUAL(err.getEntries()[0].getLogLine(),
                      "(no file) Error: Cannot stat file '" + absPath
                          + "' (No such file or directory)\n");
    BOOST_CHECK_EQUAL(err.getEntries()[1].getLogLine(),
                      "(no file) Error: Cannot open input file '" + absPath
                          + "' (No such file or directory)\n");
}

BOOST_AUTO_TEST_CASE(test_inner_include_ok)
{
    Teng::Teng_t teng("templates", 0);
    Teng::Error_t err;

    BOOST_CHECK_EQUAL(
        generate(teng, "<?teng include file=\"page.html\" ?>", *createFragmentTitle(), err),
        result_3);
    BOOST_CHECK_EQUAL(err.count(), 0);
}

BOOST_AUTO_TEST_CASE(test_inner_include_warning)
{
    Teng::Teng_t teng("templates", 0);
    Teng::Error_t err;

    const std::string filename1 = "common-head.html";
    const std::string filename2 = "subdir/paragraph.html";

    BOOST_CHECK_EQUAL(
        generate(teng, "<?teng include file=\"page.html\" ?>", *createFragment(), err), result_4);
    BOOST_REQUIRE_EQUAL(err.count(), 2);
    BOOST_CHECK_EQUAL(err.getEntries()[0].getLogLine(),
                      filename1 + "(2,37) Warning: Runtime: Variable '.title' is undefined\n");
    BOOST_CHECK_EQUAL(err.getEntries()[1].getLogLine(),
                      filename2 + "(1,19) Warning: Runtime: Variable '.title' is undefined\n");
}

BOOST_AUTO_TEST_CASE(test_include_abs_ok)
{
    Teng::Teng_t teng("templates", 0);
    Teng::Error_t err;

    const std::string path = createAbsPath("templates", "subdir/paragraph.html");
    BOOST_REQUIRE(boost::filesystem::exists(path));

    const std::string templ = "<?teng include file=\"" + path + "\" ?>";

    BOOST_CHECK_EQUAL(generate(teng, templ, *createFragmentTitle(), err), "Paragraph: Title");
    BOOST_CHECK_EQUAL(err.count(), 0);
}
