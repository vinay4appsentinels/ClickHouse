# XML and GraphQL Functions

ClickHouse provides functions for parsing and extracting data from XML documents and GraphQL queries.

## XML Functions

### XMLExtractString

Extracts text content from an XML document using an XPath expression.

**Syntax**

```sql
XMLExtractString(xml, xpath)
```

**Arguments**

- `xml` — XML document as a String.
- `xpath` — XPath expression to evaluate.

**Returned value**

- The text content of the first matching node. Type: `Nullable(String)`.
- Returns `NULL` if the XPath doesn't match any node.

**Features**

- Namespace prefixes are stripped during element matching, so `'Body'` matches `'soap:Body'`. This makes querying SOAP messages easy without worrying about namespace prefixes.
- Supports XPath 1.0 subset:
  - Location paths: `/`, `//`, `.`, `..`
  - Node tests: element names, `*`, `text()`, `node()`
  - Predicates: `[n]`, `[@attr]`, `[@attr='value']`
  - Axes: `child::`, `descendant::`, `parent::`, `attribute::`, `self::`

**Examples**

```sql
-- Simple extraction
SELECT XMLExtractString('<root><item>Hello</item></root>', '/root/item');
-- Result: 'Hello'

-- With namespaces (prefixes are ignored)
SELECT XMLExtractString(
    '<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/"><soap:Body>Content</soap:Body></soap:Envelope>',
    '//Body'
);
-- Result: 'Content'

-- Using attribute predicates
SELECT XMLExtractString(
    '<root><item id="1">First</item><item id="2">Second</item></root>',
    '/root/item[@id="2"]'
);
-- Result: 'Second'

-- Extract from a table column
SELECT XMLExtractString(xml_column, '//price') AS price
FROM soap_messages
WHERE XMLExtractString(xml_column, '//status') = 'success';
```

---

### XMLExtractRaw

Extracts a raw XML subtree from an XML document using an XPath expression.

**Syntax**

```sql
XMLExtractRaw(xml, xpath)
```

**Arguments**

- `xml` — XML document as a String.
- `xpath` — XPath expression to evaluate.

**Returned value**

- The serialized XML of the first matching node including tags. Type: `Nullable(String)`.
- Returns `NULL` if the XPath doesn't match any node.

**Examples**

```sql
-- Extract raw XML fragment
SELECT XMLExtractRaw('<root><item id="1">Hello</item></root>', '/root/item');
-- Result: '<item id="1">Hello</item>'

-- Extract nested structure
SELECT XMLExtractRaw(
    '<root><user><name>John</name><email>john@example.com</email></user></root>',
    '/root/user'
);
-- Result: '<user><name>John</name><email>john@example.com</email></user>'
```

---

## GraphQL Functions

### GraphQLExtractString

Extracts a field name from a GraphQL query by its dot-separated path.

**Syntax**

```sql
GraphQLExtractString(query, field_path)
```

**Arguments**

- `query` — GraphQL query string.
- `field_path` — Dot-separated path to the field.

**Returned value**

- The field name at the specified path. Type: `Nullable(String)`.
- Returns `NULL` if the path doesn't match or parsing fails.

**Path notation**

- Use dots to navigate through the query structure
- Start with the operation type (`query`, `mutation`, `subscription`) or directly with field names
- Examples:
  - `'user.name'` - extracts 'name' from the user field
  - `'query.user.email'` - explicitly starts from query operation
  - `'mutation.createUser.id'` - navigates mutation operation

**Examples**

```sql
-- Simple field extraction
SELECT GraphQLExtractString('{ user { name email } }', 'user.name');
-- Result: 'name'

-- From mutation
SELECT GraphQLExtractString(
    'mutation { createUser(input: {name: "John"}) { id } }',
    'mutation.createUser.id'
);
-- Result: 'id'

-- Analyze GraphQL logs
SELECT
    GraphQLExtractString(query, 'user.id') AS requested_user_field
FROM graphql_logs
WHERE GraphQLGetOperationType(query) = 'query';
```

---

### GraphQLExtractVariable

Extracts a variable definition from a GraphQL query.

**Syntax**

```sql
GraphQLExtractVariable(query, variable_name)
```

**Arguments**

- `query` — GraphQL query string with variable definitions.
- `variable_name` — Name of the variable (without the `$` prefix).

**Returned value**

- The variable's default value if specified, otherwise the variable type. Type: `Nullable(String)`.
- Returns `NULL` if the variable is not found or parsing fails.

**Examples**

```sql
-- Extract variable with default value
SELECT GraphQLExtractVariable(
    'query GetUser($id: ID! = "123") { user(id: $id) { name } }',
    'id'
);
-- Result: '123'

-- Extract variable type (no default)
SELECT GraphQLExtractVariable(
    'query GetUser($id: ID!) { user(id: $id) { name } }',
    'id'
);
-- Result: 'ID!'

-- Extract from query with multiple variables
SELECT GraphQLExtractVariable(
    'query Search($term: String!, $limit: Int = 10) { search(term: $term, limit: $limit) { results } }',
    'limit'
);
-- Result: '10'
```

---

### GraphQLGetOperationType

Returns the operation type of a GraphQL query.

**Syntax**

```sql
GraphQLGetOperationType(query)
```

**Arguments**

- `query` — GraphQL query string.

**Returned value**

- The operation type: `'query'`, `'mutation'`, or `'subscription'`. Type: `Nullable(String)`.
- For anonymous queries (starting with `{`), returns `'query'`.
- Returns `NULL` if parsing fails.

**Examples**

```sql
-- Explicit query
SELECT GraphQLGetOperationType('query { user { name } }');
-- Result: 'query'

-- Mutation
SELECT GraphQLGetOperationType('mutation { createUser { id } }');
-- Result: 'mutation'

-- Subscription
SELECT GraphQLGetOperationType('subscription { onUserCreated { id name } }');
-- Result: 'subscription'

-- Anonymous query
SELECT GraphQLGetOperationType('{ user { name } }');
-- Result: 'query'

-- Filter logs by operation type
SELECT count(*)
FROM graphql_logs
GROUP BY GraphQLGetOperationType(query);
```

---

## Use Cases

### Parsing SOAP/XML API Responses

```sql
-- Extract data from SOAP responses stored in ClickHouse
SELECT
    XMLExtractString(response, '//OrderId') AS order_id,
    XMLExtractString(response, '//Status') AS status,
    XMLExtractString(response, '//Amount') AS amount
FROM api_responses
WHERE XMLExtractString(response, '//Status') = 'Completed';
```

### Analyzing GraphQL Query Patterns

```sql
-- Analyze which operations are most common
SELECT
    GraphQLGetOperationType(query) AS op_type,
    count(*) AS cnt
FROM graphql_logs
GROUP BY op_type
ORDER BY cnt DESC;

-- Find queries requesting specific fields
SELECT query, timestamp
FROM graphql_logs
WHERE GraphQLExtractString(query, 'user.email') IS NOT NULL;
```

### Combining XML and GraphQL Analysis

```sql
-- If your system bridges GraphQL to SOAP
SELECT
    GraphQLGetOperationType(request) AS graphql_op,
    XMLExtractString(response, '//StatusCode') AS soap_status
FROM api_bridge_logs
WHERE response IS NOT NULL;
```
