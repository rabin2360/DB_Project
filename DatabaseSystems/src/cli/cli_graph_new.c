#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cli.h"
#include "cli_logger.h"

int counter = 0;

void
cli_graph_new(char *cmdline, int *pos)
{
	graph_t g;
	vertex_t v;

	/* Create first vertex in graph */
	v = (vertex_t) malloc(sizeof(struct vertex));
	assert (v != NULL);
	vertex_init(v);
	v->id = 1;
	
	/* Create new graph */
	g = (graph_t) malloc(sizeof(struct graph));
	assert (g != NULL);
	graph_init(g, counter++);
	graph_insert_vertex(g, v);

	if (graphs == NULL)
		current = g;
	cli_graphs_insert(g);
}
