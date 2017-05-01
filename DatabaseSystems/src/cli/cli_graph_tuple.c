#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "cli_logger.h"

char logMessage[LOG_MSG_BUF];

void
cli_graph_tuple(char *cmdline, int *pos)
{
	memset(logMessage, 0, LOG_MSG_BUF);
	char s1[BUFSIZE], s2[BUFSIZE], s3[BUFSIZE], s4[BUFSIZE];
	vertexid_t id1;
	schema_type_t st;
	int i, n;

	memset(s1, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s1);
	memset(s2, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s2);
	memset(s3, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s3);
	memset(s4, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s4);
#if _DEBUG
	printf("s1=[%s] s2=[%s] s3=[%s], s4=[%s]\n", s1, s2, s3, s4);
#endif
	if (strlen(s1) <= 0) {
		graph_print(current, 1); /* with tuples */
		printf("\n");
		return;
	}
	if (strlen(s2) <= 0) {
		printf("Missing name or id\n");
		sprintf(logMessage,"Cannot perform tuple operation. Missing either attribute or attribute value in input [%s].", cmdline);
		updateLog(E, logMessage);
		return;
	}
	/* s1 is always a vertex id */
	id1 = (vertexid_t) atoi(s1);

	/* Figure out if this is for an edge or vertex tuple */
	n = strlen(s2);
	for (i = 0, st = EDGE; i < n; i++)
		if (!isdigit(s2[i])) {
			st = VERTEX;
			break;
		}

	if (st == VERTEX) {
		vertex_t v;
		base_types_t bt;

		/*
		 * Set the value of a vertex tuple
		 */
		if (current == NULL || current->sv == NULL) {
			printf("Missing vertex schema\n");
			sprintf(logMessage, "%s", "Missing vertex schema. Cannot update the tuple.");
			updateLog(E, logMessage);
			return;
		}

		v = graph_find_vertex_by_id(current, id1);
		if (v == NULL) {
			printf("Illegal vertex id\n");
			sprintf(logMessage, "Missing vertex id %llu. Cannot update the tuple.", id1);
			updateLog(E, logMessage);
			return;
		}
		/* s2 is an attribute name from the vertex schema */

		/* Check for a VARCHAR */
		bt = schema_find_type_by_name(v->tuple->s, s2);
		if (bt == VARCHAR) {
			char *first, *second;

			first = strchr(cmdline, '"');
			if (first == NULL) {
				printf("Missing first quote");
				sprintf(logMessage, "Varchar error while updating/creating vertex tuple. Missing first quote in %s", cmdline);
				updateLog(E, logMessage);
				return;
			}
			second = strchr(first + 1, '"');
			if (second == NULL) {
				printf("Missing last quote");
				sprintf(logMessage, "Varchar error while updating/creating vertex tuple. Missing second quote in %s.", cmdline);
				updateLog(E, logMessage);
				return;
			}
			memset(s3, 0, BUFSIZE);
			strncpy(s3, first + 1, second - first - 1);
#if _DEBUG
			printf("s3=[%s]\n", s3);
#endif
		} else if (bt == ENUM) {
			attribute_t attr;

			attr = schema_find_attr_by_name(v->tuple->s, s2);
			if (attr == NULL) {
				printf("Attribute %s not found\n", s2);
				sprintf(logMessage, "Error while updating/creating vertex tuple. Missing attribute %s.", s2);
				updateLog(E, logMessage);
				return;
			}
#if _DEBUG
			printf("set attribute %s with type %s to %s\n",
			       s2, attr->e->name, s3);
#endif
			tuple_set_enum(v->tuple, s2,
				attr->e->name, s3, current->el);

			sprintf(logMessage, "Updating vertex tuple where attribute %s with type %s to %s.", s2, attr->e->name, s3);
			updateLog(V, logMessage);	
			return;
		}


		char *schemaStr = get_schema_str(current->sv);
		if (tuple_set(v->tuple, s2, s3) < 0) {
			printf("Set vertex tuple value failed\n");
			if(schemaStr == NULL){
				sprintf(logMessage, "Error while setting tuple. Tuple entry [%s:%s] doesn't match vertex schema. Current schema is null.", s3, s2);	
			} else {
				sprintf(logMessage, "Error while setting tuple. Tuple entry [%s:%s] doesn't match vertex schema [%s].", s3, s2, schemaStr);		
			}

			updateLog(E, logMessage);	
			return;
		} else {
			if(schemaStr != NULL){
				sprintf(logMessage, "In vertex id %llu of graph %llu, setting attribute %s to %s in the vertex schema %s.", id1, current->id, s2, s3, schemaStr);
				updateLog(V, logMessage);	
			}
		}

	} else if (st == EDGE) {
		edge_t e;
		vertexid_t id2;
		base_types_t bt;

		/*
		 * Set the value of an edge tuple
		 */
		if (current == NULL || current->se == NULL) {
			printf("Missing edge schema\n");
			sprintf(logMessage, "%s", "Missing edge schema. Cannot update the tuple.");
			updateLog(E, logMessage);
			return;
		}
		/* s2 is a vertex id for an edge */
		id2 = (vertexid_t) atoi(s2);

		e = graph_find_edge_by_ids(current, id1, id2);
		if (e == NULL) {
			printf("Illegal vertex id(s)\n");
			sprintf(logMessage, "Cannot update/create edge tuple. Missing vertices %llu and %llu from the graph %llu.", id1, id2, current->id);
			updateLog(E, logMessage);
			return;
		}
		/* Check for a VARCHAR */
		bt = schema_find_type_by_name(e->tuple->s, s3);
		if (bt == VARCHAR) {
			char *first, *second;

			first = strchr(cmdline, '"');
			if (first == NULL) {
				printf("Missing first quote");
				sprintf(logMessage, "Varchar error while updating/creating edge tuple. Missing first quote in %s", cmdline);
				updateLog(E, logMessage);
				return;
			}
			second = strchr(first + 1, '"');
			if (second == NULL) {
				printf("Missing last quote");
				sprintf(logMessage, "Varchar error while updating/creating edge tuple. Missing last quote in %s", cmdline);
				updateLog(E, logMessage);
				return;
			}
			memset(s4, 0, BUFSIZE);
			strncpy(s4, first + 1, second - first - 1);
#if _DEBUG
			printf("s4=[%s]\n", s4);
#endif
		} else if (bt == ENUM) {
			attribute_t attr;

			attr = schema_find_attr_by_name(e->tuple->s, s3);
			if (attr == NULL) {
				printf("Attribute %s not found\n", s3);
				sprintf(logMessage, "Error while updating/creating edge tuple. Missing attribute %s.", s3);
				updateLog(E, logMessage);
				return;
			}
#if _DEBUG
			printf("set attribute %s with type %s to %s\n",
			       s3, attr->e->name, s4);
#endif
			tuple_set_enum(e->tuple, s3,
				attr->e->name, s4, current->el);
			sprintf(logMessage, "Updating edge tuple where attribute %s with type %s to %s.", s3, attr->e->name, s4);
			updateLog(V, logMessage);
			return;
		}

		char *schemaStr = get_schema_str(current->se);
		if (tuple_set(e->tuple, s3, s4) < 0) {
			printf("Set edge tuple value failed\n");
			if(schemaStr == NULL){
				sprintf(logMessage, "Error while setting tuple. Tuple entry [%s:%s] doesn't match vertex schema. Current schema is null.", s4, s3);	
			} else {
				sprintf(logMessage, "Error while setting tuple. Tuple entry [%s:%s] doesn't match vertex schema [%s].", s4, s3, schemaStr);		
			}
			updateLog(E, logMessage);	
			return;
		} else {
			if(schemaStr != NULL){
				sprintf(logMessage, "Between vertices %llu and %llu of graph %llu, setting attribute %s to %s in the edge schema %s.", id1, id2, current->id, s3, s4, schemaStr);
				updateLog(V, logMessage);	
			}
		}
	}
}
