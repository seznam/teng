#define BOOST_TEST_MODULE test_filesystem

#include "test_utils.h"

const std::string head = R"__(    <head>
        <title>Example page: ${title}</title>
    </head>
)__";

const std::string paragraph = R"__(Paragraph: ${title})__";

const std::string page = R"__(<html>
<?teng include file="common-head.html" ?>
    <body>
        <p><?teng include file="subdir/paragraph.html" ?></p>
    </body>
</html>)__";

const std::string result_1 = R"__(    <head>
        <title>Example page: Title</title>
    </head>
)__";

const std::string result_2 = R"__(    <head>
        <title>Example page: </title>
    </head>
)__";

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

auto createFS()
{
    auto* fs = new Teng::InMemoryFilesystem_t();
    fs->storage.insert(std::make_pair("common-head.html", head));
    fs->storage.insert(std::make_pair("page.html", page));
    fs->storage.insert(std::make_pair("subdir/paragraph.html", paragraph));
    return fs;
}

BOOST_AUTO_TEST_CASE(test_include_ok)
{
    Teng::Teng_t teng(createFS(), 0);
    Teng::Error_t err;

    BOOST_CHECK_EQUAL(
        generate(teng, "<?teng include file=\"common-head.html\" ?>", *createFragmentTitle(), err),
        result_1);
    BOOST_CHECK_EQUAL(err.count(), 0);
}

BOOST_AUTO_TEST_CASE(test_include_warning)
{
    Teng::Teng_t teng(createFS(), 0);
    Teng::Error_t err;

    BOOST_CHECK_EQUAL(
        generate(teng, "<?teng include file=\"common-head.html\" ?>", *createFragment(), err),
        result_2);
    BOOST_REQUIRE_EQUAL(err.count(), 1);
    BOOST_REQUIRE_EQUAL(
        err.getEntries().front().getLogLine(),
        "common-head.html(2,37) Warning: Runtime: Variable '.title' is undefined\n");
}

BOOST_AUTO_TEST_CASE(test_inner_include_ok)
{
    Teng::Teng_t teng(createFS(), 0);
    Teng::Error_t err;

    BOOST_CHECK_EQUAL(
        generate(teng, "<?teng include file=\"page.html\" ?>", *createFragmentTitle(), err),
        result_3);
    BOOST_CHECK_EQUAL(err.count(), 0);
}

BOOST_AUTO_TEST_CASE(test_inner_include_warning)
{
    Teng::Teng_t teng(createFS(), 0);
    Teng::Error_t err;

    BOOST_CHECK_EQUAL(
        generate(teng, "<?teng include file=\"page.html\" ?>", *createFragment(), err), result_4);
    BOOST_REQUIRE_EQUAL(err.count(), 2);
    BOOST_REQUIRE_EQUAL(
        err.getEntries()[0].getLogLine(),
        "common-head.html(2,37) Warning: Runtime: Variable '.title' is undefined\n");
    BOOST_REQUIRE_EQUAL(
        err.getEntries()[1].getLogLine(),
        "subdir/paragraph.html(1,19) Warning: Runtime: Variable '.title' is undefined\n");
}
