#include <stdlib.h>

#include "syntree.h"

void free_tree(struct syntree* tree) {
	switch (tree->type) {
		case CONCAT:
			free_tree(tree->data.concat.part1);
			free_tree(tree->data.concat.part2);
			free(tree);
			break;
		case ALTER:
			free_tree(tree->data.alter.option1);
			free_tree(tree->data.alter.option2);
			free(tree);
			break;
		case REPEAT:
			free_tree(tree->data.repeat.repeated);
			free(tree);
			break;
		case RANGE: case INPUT_BEGIN: case INPUT_END: case EMPTY:
			free(tree);
			break;
	}
}
