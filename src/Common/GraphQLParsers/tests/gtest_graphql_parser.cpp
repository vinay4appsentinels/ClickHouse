#include <gtest/gtest.h>

#include <Common/GraphQLParsers/GraphQLParser.h>


using namespace DB;

TEST(GraphQLParser, ParseSimpleQuery)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("{ user { name } }"));
    ASSERT_EQ(parser.getOperationType(), "query");
}

TEST(GraphQLParser, ParseExplicitQuery)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("query { user { name } }"));
    ASSERT_EQ(parser.getOperationType(), "query");
}

TEST(GraphQLParser, ParseMutation)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("mutation { createUser { id } }"));
    ASSERT_EQ(parser.getOperationType(), "mutation");
}

TEST(GraphQLParser, ParseSubscription)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("subscription { onUserCreated { id name } }"));
    ASSERT_EQ(parser.getOperationType(), "subscription");
}

TEST(GraphQLParser, ParseNamedQuery)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("query GetUser { user { name email } }"));
    ASSERT_EQ(parser.getOperationType(), "query");
}

TEST(GraphQLParser, ExtractField)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("{ user { name email } }"));

    std::string result = parser.extractField("user.name");
    ASSERT_EQ(result, "name");
}

TEST(GraphQLParser, ExtractNestedField)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("{ user { profile { avatar } } }"));

    std::string result = parser.extractField("user.profile.avatar");
    ASSERT_EQ(result, "avatar");
}

TEST(GraphQLParser, ExtractNonExistentField)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("{ user { name } }"));

    std::string result = parser.extractField("user.nonexistent");
    ASSERT_TRUE(result.empty());
}

TEST(GraphQLParser, ExtractVariableWithDefault)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse(R"(query GetUser($id: ID! = "123") { user(id: $id) { name } })"));

    std::string result = parser.extractVariable("id");
    ASSERT_EQ(result, "123");
}

TEST(GraphQLParser, ExtractVariableType)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("query GetUser($id: ID!) { user(id: $id) { name } }"));

    std::string result = parser.extractVariable("id");
    ASSERT_EQ(result, "ID!");
}

TEST(GraphQLParser, ExtractNonExistentVariable)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse("query { user { name } }"));

    std::string result = parser.extractVariable("nonexistent");
    ASSERT_TRUE(result.empty());
}

TEST(GraphQLParser, ParseInvalidQuery)
{
    GraphQLParser parser;
    ASSERT_FALSE(parser.parse("{ user { name"));
}

TEST(GraphQLParser, ParseComplexQuery)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse(R"(
        query GetUserWithPosts($userId: ID!, $limit: Int = 10) {
            user(id: $userId) {
                name
                email
                posts(limit: $limit) {
                    title
                    body
                }
            }
        }
    )"));

    ASSERT_EQ(parser.getOperationType(), "query");
    ASSERT_EQ(parser.extractField("user.name"), "name");
    ASSERT_EQ(parser.extractField("user.posts.title"), "title");
    ASSERT_EQ(parser.extractVariable("limit"), "10");
}

TEST(GraphQLParser, ParseMutationWithVariables)
{
    GraphQLParser parser;
    ASSERT_TRUE(parser.parse(R"(
        mutation CreateUser($input: CreateUserInput!) {
            createUser(input: $input) {
                id
                name
            }
        }
    )"));

    ASSERT_EQ(parser.getOperationType(), "mutation");
    ASSERT_EQ(parser.extractVariable("input"), "CreateUserInput!");
}
