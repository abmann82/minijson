#include <gtest/gtest.h>
#include <minijson.h>
#include <memory>

// NOTE: in recent version, a json text may consist entirely of a value only.
//       see RFC 7158
//       (previous versions required that a json text must be an object or an array, i.e. a toplevel
//       string was not allowed)
struct MiniJSONStringTestParam
{
    MiniJSONStringTestParam(const char* txt, const std::string& expectedString)
        : m_Txt(txt), m_ExpectedString(expectedString)
    {
    }
    const char* m_Txt = NULL;
    std::string m_ExpectedString;
};
class MiniJSONStringTest : public ::testing::TestWithParam<MiniJSONStringTestParam>
{
};
TEST_P(MiniJSONStringTest, DISABLED_ParseSimpleString) // disabled, because minijson only accepts objects/array as toplevel atm
{
    const char* txt = " \"123\" ";
    minijson::CParser parser;
    minijson::CEntity* e = NULL;
    ASSERT_NO_THROW(e = parser.Parse(txt));
    ASSERT_NE((minijson::CEntity*)NULL, e);
    ASSERT_TRUE(e->IsString());
    EXPECT_EQ(std::string("123"), e->StringValue());
    EXPECT_EQ(std::string("123"), e->String().Value());
    delete e;
}
INSTANTIATE_TEST_CASE_P(
        MiniJSONStringTest, // instantiation name
        MiniJSONStringTest, // class name
        ::testing::Values(
            // simple string
            MiniJSONStringTestParam(" \"123\" ", std::string("123")),

            // simple string, no spaces before/after
            MiniJSONStringTestParam("\"123\"", std::string("123")),

            // simple string, spaces in value
            MiniJSONStringTestParam(" \"123 \" ", std::string("123 ")),
            MiniJSONStringTestParam(" \" 123 \" ", std::string(" 123 ")),
            MiniJSONStringTestParam(" \" 1 2 3 \" ", std::string(" 1 2 3 ")),

            // simple string, umlauts in value
            MiniJSONStringTestParam(" \"öäü\" ", std::string("öäü")),

            // simple string, newline in value (not valid JSON, non-strict mode should accept it)
            MiniJSONStringTestParam(" \"foo\" : \" 1 \n 2 \" ", std::string(" 1 \n 2 ")),

            // simple string, escaped newline in value
            MiniJSONStringTestParam(" \"foo\" : \" 1 \\n 2 \" ", std::string(" 1 \n 2 ")),

            // simple string, unicode representation of single character (see RFC7159)
            MiniJSONStringTestParam(" \"foo\" : \" \\u0041 \" ", std::string(" A ")),
            MiniJSONStringTestParam(" \"foo\" : \" \\u005c \" ", std::string(" \\ ")),
            MiniJSONStringTestParam(" \"foo\" : \" \\u005C \" ", std::string(" \\ ")),
            MiniJSONStringTestParam(" \"foo\" : \" \\u00F6 \" ", std::string(" ö ")),
            MiniJSONStringTestParam(" \"foo\" : \" \\u00f6 \" ", std::string(" ö ")),
            MiniJSONStringTestParam(" \"foo\" : \" \\u0444 \" ", std::string(" ф ")),
            MiniJSONStringTestParam(" \"foo\" : \" \\u13DB \" ", std::string(" Ꮫ ")),
            MiniJSONStringTestParam(" \"foo\" : \" \\u13db \" ", std::string(" Ꮫ "))
        )
);


/**
 * Struct for parameterized "value in an object" tests, providing the key of the value and the
 * string-value of the value.
 **/
struct MiniJSONObjectValueTestParam
{
    MiniJSONObjectValueTestParam(const char* txt, const char* key, const std::string& expectedValue)
        : m_Txt(txt), m_Key(key), m_ExpectedValue(expectedValue)
    {
    }
    const char* m_Txt = NULL;
    const char* m_Key = NULL;
    std::string m_ExpectedValue;
};
class MiniJSONObjectStringTest : public ::testing::TestWithParam<MiniJSONObjectValueTestParam>
{
};
TEST_P(MiniJSONObjectStringTest, ParseSimpleObjectString)
{
    const MiniJSONObjectValueTestParam& p = GetParam();
    minijson::CParser parser;
    minijson::CEntity* e = NULL;
    ASSERT_NO_THROW(e = parser.Parse(p.m_Txt));
    ASSERT_NE((minijson::CEntity*)NULL, e);
    ASSERT_TRUE(e->IsObject());
    minijson::CEntity* s = e->Object().GetEntity(p.m_Key);
    ASSERT_NE(s, (minijson::CEntity*)NULL);
    ASSERT_TRUE(s->IsString());
    EXPECT_EQ(p.m_ExpectedValue, s->StringValue());
    EXPECT_EQ(p.m_ExpectedValue, s->String().Value());
    EXPECT_EQ(p.m_ExpectedValue, e->Object().GetString(p.m_Key));
    delete e;
}
INSTANTIATE_TEST_CASE_P(
        MiniJSONObjectStringTest, // instantiation name
        MiniJSONObjectStringTest, // class name
        ::testing::Values(
            // simple object/string
            MiniJSONObjectValueTestParam(" { \"foo\": \"123\" } ", "foo", std::string("123")),

            // simple object/string, no spaces
            MiniJSONObjectValueTestParam("{\"foo\":\"123\"}", "foo", std::string("123")),

            // simple object/string, spaces in *key*
            MiniJSONObjectValueTestParam(" { \"foo \" : \"123\" } ", "foo ", std::string("123")),
            MiniJSONObjectValueTestParam(" { \" foo \" : \"123\" } ", " foo ", std::string("123")),
            MiniJSONObjectValueTestParam(" { \" f o o \" : \"123\" } ", " f o o ", std::string("123")),

            // simple object/string, umlauts in *key*
            MiniJSONObjectValueTestParam(" { \"öäü\" : \"123\" } ", "öäü", std::string("123")),

            // simple object/string, spaces in value
            MiniJSONObjectValueTestParam(" { \"foo\" : \"123 \" } ", "foo", std::string("123 ")),
            MiniJSONObjectValueTestParam(" { \"foo\" : \" 123 \" } ", "foo", std::string(" 123 ")),
            MiniJSONObjectValueTestParam(" { \"foo\" : \" 1 2 3 \" } ", "foo", std::string(" 1 2 3 ")),

            // simple object/string, umlauts in value
            MiniJSONObjectValueTestParam(" { \"foo\" : \"öäü\" } ", "foo", std::string("öäü")),

            // simple object/string, newline in value (not valid JSON, non-strict mode should accept it)
            MiniJSONObjectValueTestParam(" { \"foo\" : \" 1 \n 2 \" } ", "foo", std::string(" 1 \n 2 ")),

            // simple object/string, escaped newline in value
            MiniJSONObjectValueTestParam(" { \"foo\" : \" 1 \\n 2 \" } ", "foo", std::string(" 1 \n 2 ")),

            // simple object/string, unicode representation of single character (see RFC7159)
            MiniJSONObjectValueTestParam(" { \"foo\" : \" \\u0041 \" } ", "foo", std::string(" A ")),
            MiniJSONObjectValueTestParam(" { \"foo\" : \" \\u005c \" } ", "foo", std::string(" \\ ")),
            MiniJSONObjectValueTestParam(" { \"foo\" : \" \\u005C \" } ", "foo", std::string(" \\ ")),
            MiniJSONObjectValueTestParam(" { \"foo\" : \" \\u00F6 \" } ", "foo", std::string(" ö ")),
            MiniJSONObjectValueTestParam(" { \"foo\" : \" \\u00f6 \" } ", "foo", std::string(" ö ")),
            MiniJSONObjectValueTestParam(" { \"foo\" : \" \\u0444 \" } ", "foo", std::string(" ф ")),
            MiniJSONObjectValueTestParam(" { \"foo\" : \" \\u13DB \" } ", "foo", std::string(" Ꮫ ")),
            MiniJSONObjectValueTestParam(" { \"foo\" : \" \\u13db \" } ", "foo", std::string(" Ꮫ "))
        )
);

class MiniJSONObjectNumberTest : public ::testing::TestWithParam<MiniJSONObjectValueTestParam>
{
};
TEST_P(MiniJSONObjectNumberTest, ParseSimpleObjectNumber)
{
    const MiniJSONObjectValueTestParam& p = GetParam();
    minijson::CParser parser;
    minijson::CEntity* e = NULL;
    ASSERT_NO_THROW(e = parser.Parse(p.m_Txt));
    ASSERT_NE((minijson::CEntity*)NULL, e);
    ASSERT_TRUE(e->IsObject());
    minijson::CEntity* n = e->Object().GetEntity(p.m_Key);
    ASSERT_NE(n, (minijson::CEntity*)NULL);
    ASSERT_TRUE(n->IsNumber());
    EXPECT_EQ(p.m_ExpectedValue, n->ToString());
    EXPECT_EQ(p.m_ExpectedValue, n->Number().Value());
    delete e;
}
INSTANTIATE_TEST_CASE_P(
        MiniJSONObjectNumberTest, // instantiation name
        MiniJSONObjectNumberTest, // class name
        ::testing::Values(
            // simple object/number
            MiniJSONObjectValueTestParam(" { \"foo\": 123 } ", "foo", std::string("123")),
            MiniJSONObjectValueTestParam(" { \"foo\": -123 } ", "foo", std::string("-123")),
            MiniJSONObjectValueTestParam(" { \"foo\": 123.4 } ", "foo", std::string("123.4")),
            MiniJSONObjectValueTestParam(" { \"foo\": -123.4 } ", "foo", std::string("-123.4")),

            // simple object/number, no spaces
            MiniJSONObjectValueTestParam("{\"foo\":123}", "foo", std::string("123")),
            MiniJSONObjectValueTestParam("{\"foo\":-123}", "foo", std::string("-123")),
            MiniJSONObjectValueTestParam("{\"foo\":123.4}", "foo", std::string("123.4")),
            MiniJSONObjectValueTestParam("{\"foo\":-123.4}", "foo", std::string("-123.4"))

            // numbers with exponential parts (e.g. 10e3 == 10*10^3 == 10000)
            // see RFC 7158 section 6 for details
#if 0 // currently not supported by minijson
            MiniJSONObjectValueTestParam("{\"foo\":1e+4}", "foo", std::string("1e+4")),
            MiniJSONObjectValueTestParam("{\"foo\":1E+4}", "foo", std::string("1E+4")),
            MiniJSONObjectValueTestParam("{\"foo\":1e-4}", "foo", std::string("1e-4")),
            MiniJSONObjectValueTestParam("{\"foo\":1E-4}", "foo", std::string("1E-4")),
            MiniJSONObjectValueTestParam("{\"foo\":1e4}", "foo", std::string("1e4")),
            MiniJSONObjectValueTestParam("{\"foo\":1E4}", "foo", std::string("1E4")),
            MiniJSONObjectValueTestParam("{\"foo\":-1e4}", "foo", std::string("-1e4")),
            MiniJSONObjectValueTestParam("{\"foo\":-1E4}", "foo", std::string("-1E4")),
            MiniJSONObjectValueTestParam("{\"foo\":2.1e+4}", "foo", std::string("2.1e+4")),
            MiniJSONObjectValueTestParam("{\"foo\":2.1E+4}", "foo", std::string("2.1E+4")),
            MiniJSONObjectValueTestParam("{\"foo\":2.1e-4}", "foo", std::string("2.1e-4")),
            MiniJSONObjectValueTestParam("{\"foo\":2.1E-4}", "foo", std::string("2.1E-4")),
            MiniJSONObjectValueTestParam("{\"foo\":2.1e4}", "foo", std::string("2.1e4")),
            MiniJSONObjectValueTestParam("{\"foo\":2.1E4}", "foo", std::string("2.1E4")),
            MiniJSONObjectValueTestParam("{\"foo\":-2.1e4}", "foo", std::string("-2.1e4")),
            MiniJSONObjectValueTestParam("{\"foo\":-2.1E4}", "foo", std::string("-2.1E4"))
#endif
        )
);

class MiniJSONObjectBooleanTest : public ::testing::TestWithParam<MiniJSONObjectValueTestParam>
{
};
TEST_P(MiniJSONObjectBooleanTest, ParseSimpleObjectBoolean)
{
    const MiniJSONObjectValueTestParam& p = GetParam();
    minijson::CParser parser;
    minijson::CEntity* e = NULL;
    ASSERT_NO_THROW(e = parser.Parse(p.m_Txt));
    ASSERT_NE((minijson::CEntity*)NULL, e);
    ASSERT_TRUE(e->IsObject());
    minijson::CEntity* n = e->Object().GetEntity(p.m_Key);
    ASSERT_NE(n, (minijson::CEntity*)NULL);
    ASSERT_TRUE(n->IsBoolean());
    EXPECT_EQ(p.m_ExpectedValue, n->ToString());
    if (p.m_ExpectedValue == "true")
    {
        EXPECT_EQ(true, n->Boolean().Value());
    }
    else if (p.m_ExpectedValue == "false")
    {
        EXPECT_EQ(false, n->Boolean().Value());
    }
    else
    {
        ASSERT_TRUE(false);
    }
    delete e;
}
INSTANTIATE_TEST_CASE_P(
        MiniJSONObjectBooleanTest, // instantiation name
        MiniJSONObjectBooleanTest, // class name
        // NOTE: json allows lowercase true/false only.
        ::testing::Values(
            // simple object/boolean
            MiniJSONObjectValueTestParam(" { \"foo\": true} ", "foo", std::string("true")),
            MiniJSONObjectValueTestParam(" { \"foo\": false} ", "foo", std::string("false")),

            // simple object/boolean, no spaces
            MiniJSONObjectValueTestParam("{\"foo\":true}", "foo", std::string("true")),
            MiniJSONObjectValueTestParam("{\"foo\":false}", "foo", std::string("false"))
        )
);

class MiniJSONObjectNullTest : public ::testing::TestWithParam<MiniJSONObjectValueTestParam>
{
};
TEST_P(MiniJSONObjectNullTest, ParseSimpleObjectNull)
{
    const MiniJSONObjectValueTestParam& p = GetParam();
    minijson::CParser parser;
    minijson::CEntity* e = NULL;
    ASSERT_NO_THROW(e = parser.Parse(p.m_Txt));
    ASSERT_NE((minijson::CEntity*)NULL, e);
    ASSERT_TRUE(e->IsObject());
    minijson::CEntity* n = e->Object().GetEntity(p.m_Key);
    ASSERT_NE(n, (minijson::CEntity*)NULL);
    ASSERT_TRUE(n->IsNull());
    delete e;
}
INSTANTIATE_TEST_CASE_P(
        MiniJSONObjectNullTest, // instantiation name
        MiniJSONObjectNullTest, // class name
        // NOTE: json allows lowercase true/false only.
        ::testing::Values(
            // simple object/null
            MiniJSONObjectValueTestParam(" { \"foo\": null} ", "foo", std::string()),

            // simple object/null, no spaces
            MiniJSONObjectValueTestParam("{\"foo\":null}", "foo", std::string())
        )
);



// TODO: arrays
