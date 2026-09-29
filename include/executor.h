#pragma once

#include "common.h"

typedef enum
{
	VALUE_NUMBER,
	VALUE_STRING
} ValueType;

typedef struct
{
	ValueType type;

	union
	{
		double number;
		const char *string;
	};
} Value;

typedef struct
{
	Value *values;
	size_t value_count;
} Tuple;

typedef struct
{
	const char *name;

	const char **attributes;
	size_t attribute_count;

	Tuple *tuples;
	size_t tuple_count;
} Relation;

typedef struct
{
	size_t join_comparisons;
	size_t select_tuples_examined;
} ExecutorStats;

typedef struct
{
    Arena *arena;

    Relation *relations;
    size_t relation_count;

    ExecutorStats stats;
} Executor;

void executor_init(Executor *executor, Arena *arena);
void execute(Executor *executor, Tree *tree);
void print_relations(const Executor *executor);

