#include <gtest/gtest.h>

#include <Common/XMLParsers/PocoXMLParser.h>
#include <Common/XMLParsers/XPathEvaluator.h>


using namespace DB;

TEST(XMLParser, ParseSimpleXML)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse("<root><item>Hello</item></root>"));

    auto root = parser.getRootNode();
    ASSERT_FALSE(root.isNull());
    ASSERT_EQ(root.localName(), "root");
}

TEST(XMLParser, ParseInvalidXML)
{
    PocoXMLParser parser;
    ASSERT_FALSE(parser.parse("<root><item>unclosed"));
}

TEST(XMLParser, ExtractTextContent)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse("<root><item>Hello World</item></root>"));

    auto root = parser.getRootNode();
    auto children = root.childElements();
    ASSERT_EQ(children.size(), 1);
    ASSERT_EQ(children[0].innerText(), "Hello World");
}

TEST(XMLParser, ExtractAttribute)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse("<root><item id=\"123\" name=\"test\">Content</item></root>"));

    auto root = parser.getRootNode();
    auto children = root.childElements();
    ASSERT_EQ(children.size(), 1);
    ASSERT_TRUE(children[0].hasAttribute("id"));
    ASSERT_EQ(children[0].getAttribute("id"), "123");
    ASSERT_EQ(children[0].getAttribute("name"), "test");
}

TEST(XMLParser, NamespaceStripping)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse(R"(<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/"><soap:Body>Content</soap:Body></soap:Envelope>)"));

    auto root = parser.getRootNode();
    ASSERT_EQ(root.localName(), "Envelope");

    auto children = root.childElements();
    ASSERT_EQ(children.size(), 1);
    ASSERT_EQ(children[0].localName(), "Body");
}

TEST(XPathEvaluator, SimplePathEvaluation)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse("<root><item>Hello</item></root>"));

    XPathEvaluator evaluator;
    auto result = evaluator.evaluateToString(parser.getRootNode(), "/root/item");
    ASSERT_EQ(result, "Hello");
}

TEST(XPathEvaluator, DescendantSearch)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse("<root><level1><level2><target>Found</target></level2></level1></root>"));

    XPathEvaluator evaluator;
    auto result = evaluator.evaluateToString(parser.getRootNode(), "//target");
    ASSERT_EQ(result, "Found");
}

TEST(XPathEvaluator, AttributePredicate)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse(R"(<root><item id="1">First</item><item id="2">Second</item></root>)"));

    XPathEvaluator evaluator;
    auto result = evaluator.evaluateToString(parser.getRootNode(), "/root/item[@id='2']");
    ASSERT_EQ(result, "Second");
}

TEST(XPathEvaluator, NamespaceIgnoring)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse(R"(<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/"><soap:Body>Content</soap:Body></soap:Envelope>)"));

    XPathEvaluator evaluator;
    auto result = evaluator.evaluateToString(parser.getRootNode(), "//Body");
    ASSERT_EQ(result, "Content");
}

TEST(XPathEvaluator, NonExistentPath)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse("<root><item>Hello</item></root>"));

    XPathEvaluator evaluator;
    auto result = evaluator.evaluateToString(parser.getRootNode(), "/root/nonexistent");
    ASSERT_TRUE(result.empty());
}

TEST(XMLParser, ToXMLSerialization)
{
    PocoXMLParser parser;
    ASSERT_TRUE(parser.parse("<root><item id=\"1\">Hello</item></root>"));

    auto root = parser.getRootNode();
    auto children = root.childElements();
    ASSERT_EQ(children.size(), 1);

    std::string xml = children[0].toXML();
    ASSERT_TRUE(xml.find("item") != std::string::npos);
    ASSERT_TRUE(xml.find("Hello") != std::string::npos);
}
