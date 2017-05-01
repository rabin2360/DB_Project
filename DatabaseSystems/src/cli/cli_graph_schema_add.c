#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "graph.h"
#include "cli_logger.h"

void cli_graph_update_tuples(schema_type_t st, int old_schema_size);

static void
cli_graph_schema_add_enum(
	int old_schema_size,
	schema_type_t st,
	enum_t e,
	char *name)
{
	attribute_t attr;
#if _DEBUG
	printf("add attribute type enum %s name %s to schema\n",
	       e->name, name);
#endif
	attr = (attribute_t)
		malloc(sizeof(struct attribute));
	assert(attr != NULL);
	schema_attribute_init(attr, name, ENUM, e);
	strncpy(attr->name, name, ATTR_NAME_MAXLEN - 1);
	if (st == EDGE) {
		if (current->se == NULL)
			schema_init(&(current->se));
		schema_attribute_insert(current->se, attr);
	} else if (st == VERTEX) {
		if (current->sv == NULL)
			schema_init(&(current->sv));
		schema_attribute_insert(current->sv, attr);
	}
#if _DEBUG
	printf("update tuples with enum values\n");
#endif
	cli_graph_update_tuples(st, old_schema_size);

}

static void
cli_graph_schema_add_base(
	int old_schema_size,
	schema_type_t st,
	char *type,
	char *name)
{
	int i;

	for (i = 0; i < BASE_TYPES_MAX; i++) {
		if (strcasecmp(type, base_types_str[i]) == 0) {
#if _DEBUG
			printf("add %s to schema\n", type);
#endif
			attribute_t attr;
	
			attr = (attribute_t)
				malloc(sizeof(struct attribute));
			assert(attr != NULL);
			schema_attribute_init(attr, name, i, NULL);
			if (st == EDGE) {
				if (current->se == NULL)
					schema_init(&(current->se));
				schema_attribute_insert(
					current->se, attr);
			} else if (st == VERTEX) {
				if (current->sv == NULL)
					schema_init(&(current->sv));
				schema_attribute_insert(
					current->sv, attr);
			}
#if _DEBUG
			printf("update tuples\n");
#endif
			cli_graph_update_tuples(st, old_schema_size);

			break;
		}
	}
}

void
cli_graph_schema_add(schema_type_t st, char *cmdline, int *pos)
{
	char logMessage[LOG_BUF];
	char currentNodeId[LOG_MSG_BUF];
	
	if(current != NULL) {
		strcpy(logMessage, "In graph ");
		sprintf(currentNodeId, "%llu", current->id);
		strcat(logMessage, currentNodeId);
		strcat(logMessage, ", ");
	}else {
		sprintf(logMessage, "%s", "Error adding schema. Either the vertex or the edge is missing.");
		updateLog(E, logMessage);
		return;
	}

	char *schemaStr;	
	if(st == EDGE && current != NULL) {
		strcat(logMessage, "change edge schema from ");
		schemaStr = get_schema_str(current->se);
	} else if(st == VERTEX && current != NULL){
		strcat(logMessage, "change vertex schema from ");
		schemaStr = get_schema_str(current->sv);
	}else {
		strcat(logMessage, "change schema from ");
		schemaStr = NULL;
	}

	if(schemaStr == NULL) {
		strcat(logMessage, "null to ");
	} else {
		strcat(logMessage, schemaStr);
		strcat(logMessage, " to ");
	}


	char type[BUFSIZE];
	char name[BUFSIZE];
	int old_schema_size = 0;
	enum_t e;

	if (st == EDGE && current->se != NULL)
		old_schema_size = schema_size(current->se);
	else if (st == VERTEX && current->sv != NULL)
		old_schema_size = schema_size(current->sv);

	/* Attribute type */
        memset(type, 0, BUFSIZE);
        nextarg(cmdline, pos, " ", type);

	/* Attribute name */
        memset(name, 0, BUFSIZE);
        nextarg(cmdline, pos, " ", name);

	/* Search enums for type name */
	e = enum_list_find_by_name(current->el, type);
	if (e != NULL) {
#if _DEBUG
		printf("add enum %s\n", name);
#endif
		cli_graph_schema_add_enum(old_schema_size, st, e, name);
		if(st == VERTEX) {
			schemaStr = get_schema_str(current->sv);
		} else if(st == EDGE) {
			schemaStr = get_schema_str(current->se);
		} else {
			schemaStr = NULL;
		}

		if(schemaStr == NULL) {
			strcat(logMessage, "null. ");
		} else {
			strcat(logMessage, schemaStr);
		}
		printf("%s\n", logMessage);
		return;
	}
			
	cli_graph_schema_add_base(old_schema_size, st, type, name);
		
	if(st == VERTEX) {
		schemaStr = get_schema_str(current->sv);
	} else if(st == EDGE) {
		schemaStr = get_schema_str(current->se);
	} else {
		schemaStr = NULL;
	}

	if(schemaStr == NULL) {
		strcat(logMessage, "null. ");
	} else {
		strcat(logMessage, schemaStr);
	}
	updateLog(V, logMessage);
}