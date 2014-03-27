#include <teng.h>
#include <gtest/gtest.h>
#include <iostream>

//g++ -I/usr/src/gtest /usr/src/gtest/src/gtest-all.cc compare.cc -lteng

namespace {
std::string get_teng_output(const std::string& templ, const Teng::Fragment_t& data, 
        std::string content_type="text/html", std::string encoding="utf-8") {
    std::string result;
    Teng::Teng_t teng("", Teng::Teng_t::LM_LOG_TO_OUTPUT);
    Teng::StringWriter_t writer(result);
    Teng::Error_t err; 
    teng.generatePage(templ, "", "", "", content_type, encoding, data, writer, err);
    return result;
}
std::string get_teng_output(const std::string& templ) {
    Teng::Fragment_t data;
    return get_teng_output(templ, data);
}
}

TEST(Teng, EscapeDoubleDolar) {
    // data: fragment.string = "\"
    Teng::Fragment_t data;
    Teng::FragmentList_t &fragment = data.addFragmentList("fragment");
    Teng::Fragment_t &fragment_first = fragment.addFragment();
    fragment_first.addVariable(std::string("string"), "\"");


    EXPECT_EQ(get_teng_output("${$$.fragment.string}", data), 
            get_teng_output("<?teng frag fragment ?>${string}<?teng endfrag ?>", data));
}

TEST(Teng, BasicEscape) {
    EXPECT_EQ(get_teng_output("${escape(\"<div>\")}"), "&lt;div&gt;");
}

TEST(Teng, AlwaysEscape) {
    // wtf teng bug-feature
    EXPECT_EQ(get_teng_output("<?teng set $.__q = '\"kaktus&<>\"'?><?teng set $.__r = $.__q?><?teng set $.__s = $.__r?><?teng set $.__t = $.__s?>${.__t}"), "&amp;amp;amp;quot;kaktus&amp;amp;amp;amp;&amp;amp;amp;lt;&amp;amp;amp;gt;&amp;amp;amp;quot;");
}

TEST(Teng, BasicInt) {
    EXPECT_EQ(get_teng_output("${int(\"12\"++\"3.5\")}"), "123");
    EXPECT_EQ(get_teng_output("${int(22.567)}"), "22");
    EXPECT_EQ(get_teng_output("${int(\"12.6er\")}"), "undefined");
    EXPECT_EQ(get_teng_output("${int(\"128px\",1)}"), "128");
}

TEST(Teng, BasicIsNumber) {
    EXPECT_EQ(get_teng_output("${isnumber(123)}"), "1");
    EXPECT_EQ(get_teng_output("${isnumber(\"123\")}"), "0");
}

TEST(Teng, BasicReplace) {
    EXPECT_EQ(get_teng_output("${replace(\"jede jede šnek\", \"jede\",\"leze\")}"), "leze leze šnek");
}

TEST(Teng, BasicRound) {
    EXPECT_EQ(get_teng_output("${round(123.1245,3)}"), "123.125");
    EXPECT_EQ(get_teng_output("${round(123.1245,2)}"), "123.12");
}

TEST(Teng, BasicSectotime) {
    EXPECT_EQ(get_teng_output("${sectotime(7425)}"), "2:03:45");
}

TEST(Teng, BasicSubstr) {
    EXPECT_EQ(get_teng_output("${substr(\"Dlouhý text\", 7)}"), "text");
    EXPECT_EQ(get_teng_output("${substr(\"Dlouhý text\", 7, \"-> \")}"), "-> text");
    EXPECT_EQ(get_teng_output("${substr(\"Dlouhý text\", 7, \"[\", \"]\")}"), "[text");
    EXPECT_EQ(get_teng_output("${substr(\"Jméno: Pavel Pavlů\", 7, 13)}"), "Pavel ");
    EXPECT_EQ(get_teng_output("${substr(\"Jméno: Pavel Pavlů\", 7, 13, \"ten \", \"je ale šikula...\")}"), "ten Pavel je ale šikula...");
    EXPECT_EQ(get_teng_output("${substr(\"abc\", 0, 3, \"&gt;\", \"&lt;\")}"), "abc");
    EXPECT_EQ(get_teng_output("${substr(\"abc\", 1, 2, \"&gt;\", \"&lt;\")}"), "&gt;b&lt;");
}

TEST(Teng, BasicUnescape) {
    EXPECT_EQ(get_teng_output("${unescape(\"tohle je &lt;b&gt;tučné&lt;/b&gt;\")}"), "tohle je <b>tučné</b>");
}

TEST(Teng, BasicRegexReplace) {
    EXPECT_EQ(get_teng_output("${regex_replace(\"foo bar\", \"\\\\w+\", \"-($0)-\")}"), "-(foo)- -(bar)-");
    EXPECT_EQ(get_teng_output("${regex_replace(\"opravdu <b>tučný</b> text\", \"<[^>]*>\", \"\")}"), "opravdu tučný text");
    EXPECT_EQ(get_teng_output("${regex_replace(\"velmivelkéslovo\", \"([^\\\\s]{5})\", \"$1 \")}"), "velmi velké slovo ");
    EXPECT_EQ(get_teng_output("${regex_replace(\"velmivelkéslovo\", \"([^\\\\s]{6})\", \"$1 \")}"), "velmiv elkésl ovo");
    EXPECT_EQ(get_teng_output("${regex_replace(\"velmivelkéslovo\", \"([^\\\\s]{4})\", \"$1 \")}"), "velm ivel késl ovo");
    EXPECT_EQ(get_teng_output("${regex_replace(\"ééééééé\", \"([^\\\\s]{1})\", \"$1 \")}"), "é é é é é é é ");


}

TEST(Teng, BasicNl2br) {
    EXPECT_EQ(get_teng_output("${nl2br(\"jede\\njede\\nmasina\")}"), "jede\n<br />jede\n<br />masina");
}

TEST(Teng, BasicNumformat) {
    EXPECT_EQ(get_teng_output("${numformat(1230.456666,3,\".\",\",\")}"), "1,230.457");
    EXPECT_EQ(get_teng_output("${numformat(1230456666,0,\".\",\" \")}"), "1 230 456 666");
}

TEST(Teng, BasicReorder) {
    EXPECT_EQ(get_teng_output("${reorder(\"%1 a %2 b %1 \", \"c\", \"d\")}"), "c a d b c ");
}

TEST(Teng, BasicConcat) {
    EXPECT_EQ(get_teng_output("<?teng set .variable = \"a\" ?><?teng set .variable = $.variable ++ \"b\" ?>${.variable}"), "ab");
}

int main(int argc, char** argv)
{
    /*The method is initializes the Google framework and must be called before RUN_ALL_TESTS */
    ::testing::InitGoogleTest(&argc, argv);

    /*RUN_ALL_TESTS automatically detects and runs all the tests defined using the TEST macro. 
     *    It's must be called only once in the code because multiple calls lead to conflicts and, 
     *       therefore, are not supported.
     *          */ 
    return RUN_ALL_TESTS();
}

