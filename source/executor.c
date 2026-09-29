#include "executor.h"

#include <string.h>
#include <stdlib.h>


static double token_to_number(const Token *token)
{
    char buffer[64];

    /*
     * Token lexemes are stored as a pointer and a length, rather than
     * as null-terminated strings. strtod() expects a null-terminated
     * string, so copy the numeric lexeme into a buffer and add the
     * terminating '\0' before passing it to strtod().
     */
    memcpy(buffer, token->lexeme_start, token->lexeme_length);
    buffer[token->lexeme_length] = '\0';

    return strtod(buffer, NULL);
}


static char *token_to_string(Arena *arena, const Token *token)
{
    size_t length = token->lexeme_length;

    /*
     * STRING tokens include their surrounding single quotes.
     * Remove those quotes when storing the actual string value.
     */
    if (token->category == STRING)
    {
        length -= 2;
    }

    char *string = arena_push(
        arena,
        length + 1,
        _Alignof(char)
    );

    if (token->category == STRING)
    {
        size_t j = 0;

        for (size_t i = 1; i < token->lexeme_length - 1; i++)
        {
            /*
             * Two consecutive single quotes inside a STRING token
             * represent one literal single quote.
             */
            if (token->lexeme_start[i] == '\'' &&
                token->lexeme_start[i + 1] == '\'')
            {
                string[j++] = '\'';
                i++;
            }
            else
            {
                string[j++] = token->lexeme_start[i];
            }
        }

        string[j] = '\0';
    }
    else
    {
        memcpy(string, token->lexeme_start, length);
        string[length] = '\0';
    }

    return string;
}


static Value token_to_value(Executor *executor, const Token *token)
{
    Value value;

    if (token->category == NUMBER)
    {
        value.type = VALUE_NUMBER;
        value.number = token_to_number(token);
    }
    else
    {
        /*
         * WORD and STRING both represent string values in the
         * relation-definition grammar.
         */
        value.type = VALUE_STRING;
        value.string = token_to_string(executor->arena, token);
    }

    return value;
}


static Relation *find_relation(
        Executor *executor,
        const char *name)
{
    for (size_t i = 0; i < executor->relation_count; i++)
    {
        if (strcmp(executor->relations[i].name, name) == 0)
        {
            return &executor->relations[i];
        }
    }

    return NULL;
}


static Relation *execute_relation_definition(
        Executor *executor,
        TreeNode *root)
{
    TreeNode *definition = root->left_child;

    TreeNode *relation_node = definition->left_child;
    TreeNode *attributes_node = definition->right_child;

    Token *relation_token = relation_node->token_arr[0];

    char *relation_name = arena_push(
        executor->arena,
        relation_token->lexeme_length + 1,
        _Alignof(char)
    );

    memcpy(
        relation_name,
        relation_token->lexeme_start,
        relation_token->lexeme_length
    );

    relation_name[relation_token->lexeme_length] = '\0';

    Relation *existing = find_relation(
        executor,
        relation_name
    );

    if (existing != NULL)
    {
        return existing;
    }

    Relation *relation = arena_push(
        executor->arena,
        sizeof(Relation),
        _Alignof(Relation)
    );

    relation->name = relation_name;
    relation->attribute_count = attributes_node->token_count;

    relation->attributes = arena_push(
        executor->arena,
        sizeof(char *) * relation->attribute_count,
        _Alignof(char *)
    );

    for (size_t i = 0; i < relation->attribute_count; i++)
    {
        Token *attribute_token = attributes_node->token_arr[i];

        char *attribute_name = arena_push(
            executor->arena,
            attribute_token->lexeme_length + 1,
            _Alignof(char)
        );

        memcpy(
            attribute_name,
            attribute_token->lexeme_start,
            attribute_token->lexeme_length
        );

        attribute_name[attribute_token->lexeme_length] = '\0';

        relation->attributes[i] = attribute_name;
    }

    relation->tuple_count = 0;

    for (TreeNode *tuple_node = root->right_child;
         tuple_node != NULL;
         tuple_node = tuple_node->left_child)
    {
        relation->tuple_count++;
    }

    if (relation->tuple_count == 0)
    {
        relation->tuples = NULL;
    }
    else
    {
        relation->tuples = arena_push(
            executor->arena,
            sizeof(Tuple) * relation->tuple_count,
            _Alignof(Tuple)
        );
    }

    size_t tuple_index = 0;

    for (TreeNode *tuple_node = root->right_child;
         tuple_node != NULL;
         tuple_node = tuple_node->left_child)
    {
        Tuple *tuple = &relation->tuples[tuple_index++];

        /*
         * token_arr[0] is the synthetic TUPLE token.
         * The actual tuple values begin at token_arr[1].
         */
        tuple->value_count = tuple_node->token_count - 1;

        tuple->values = arena_push(
            executor->arena,
            sizeof(Value) * tuple->value_count,
            _Alignof(Value)
        );

        for (size_t i = 0; i < tuple->value_count; i++)
        {
            Token *value_token = tuple_node->token_arr[i + 1];

            tuple->values[i] = token_to_value(
                executor,
                value_token
            );
        }
    }

    /*
     * Add the newly created relation to the executor's relation list.
     */
    Relation *relations = arena_push(
        executor->arena,
        sizeof(Relation) * (executor->relation_count + 1),
        _Alignof(Relation)
    );

    if (executor->relation_count > 0)
    {
        memcpy(
            relations,
            executor->relations,
            sizeof(Relation) * executor->relation_count
        );
    }

    relations[executor->relation_count] = *relation;

    executor->relations = relations;
    executor->relation_count++;

    return relation;
}


static Relation *execute_query(
        Executor *executor,
        TreeNode *node)
{
    /*
     * Query operators will be implemented here.
     */
    (void)executor;
    (void)node;

    return NULL;
}


static void execute_statement(
        Executor *executor,
        TreeNode *root)
{
    if (root == NULL || root->token_count == 0)
    {
        return;
    }

    switch (root->token_arr[0]->category)
    {
        case EQUAL:
            execute_relation_definition(executor, root);
            break;

        default:
            execute_query(executor, root);
            break;
    }
}


void executor_init(
        Executor *executor,
        Arena *arena)
{
    *executor = (Executor){
        .arena = arena,
        .relations = NULL,
        .relation_count = 0,
        .stats = {
            .join_comparisons = 0,
            .select_tuples_examined = 0
        }
    };
}


void execute(
        Executor *executor,
        Tree *tree)
{
    for (StatementNode *statement = tree->head;
         statement != NULL;
         statement = statement->next)
    {
        execute_statement(
            executor,
            statement->root
        );
    }
}

void print_relations(const Executor *executor)
{
    for (size_t i = 0; i < executor->relation_count; i++)
    {
        const Relation *relation = &executor->relations[i];

        printf("Relation: %s\n", relation->name);

        printf("Attributes: ");

        for (size_t j = 0; j < relation->attribute_count; j++)
        {
            if (j > 0)
                printf(", ");

            printf("%s", relation->attributes[j]);
        }

        printf("\n");

        printf("Tuples:\n");

        for (size_t j = 0; j < relation->tuple_count; j++)
        {
            const Tuple *tuple = &relation->tuples[j];

            printf("  (");

            for (size_t k = 0; k < tuple->value_count; k++)
            {
                if (k > 0)
                    printf(", ");

                const Value *value = &tuple->values[k];

                if (value->type == VALUE_NUMBER)
                    printf("%g", value->number);
                else
                    printf("'%s'", value->string);
            }

            printf(")\n");
        }

        printf("\n");
    }
}
