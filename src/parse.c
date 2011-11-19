#include <stdbool.h>
#include <stdlib.h>

#include "common.h"
#include "parse.h"

static struct syntree* new_syntree(void) {
	return alloc(sizeof(struct syntree));
}

static struct syntree* alternation(struct syntree* opt1, struct syntree* opt2) {
	if (!opt1) return opt2;
	if (!opt2) return opt1;
	if (opt1->type == EMPTY && opt2->type == EMPTY) {
		free_tree(opt2);
		return opt1;
	}
	struct syntree* res = new_syntree();
	res->type = ALTER;
	res->alter.option1 = opt1;
	res->alter.option2 = opt2;
	return res;
}

static struct syntree* concatenation(struct syntree* part1, struct syntree* part2) {
	if (!part1) return part2;
	if (!part2) return part1;
	if (part1->type == EMPTY) {
		free_tree(part1);
		return part2;
	}
	if (part2->type == EMPTY) {
		free_tree(part2);
		return part1;
	}
	struct syntree* res = new_syntree();
	res->type = CONCAT;
	res->concat.part1 = part1;
	res->concat.part2 = part2;
	return res;
}

static struct syntree* repetition(struct syntree* repeated, int min, int max) {
	if (!repeated) return 0;
	if (repeated->type == EMPTY) return repeated;
	struct syntree* res = new_syntree();
	res->type = REPEAT;
	res->repeat.repeated = repeated;
	res->repeat.min = min;
	res->repeat.max = max;
	return res;
}

static struct syntree* range_syntree(char min, char max) {
	struct syntree* res = new_syntree();
	res->type = RANGE;
	res->range.min = min;
	res->range.max = max;
	return res;
}

static struct syntree* empty_syntree(void) {
	struct syntree* res = new_syntree();
	res->type = EMPTY;
	return res;
}

static int parse_impl(char* re, int len, struct syntree** result) {
	struct syntree* last = 0;
	struct syntree* option = empty_syntree();
	*result = 0;
	int i;
	bool done = false;
	for (i = 0; i < len; ++i) {
		char ch = re[i];
		switch (ch) {
			case '(':
				option = concatenation(option, last);
				++i;
				i += parse_impl(re+i, len-i, &last);
				if (!last) {
					//TODO error
				}
				break;
			case ')':
				done = true;
				break;
			case '|':
				option = concatenation(option, last);
				last = 0;
				*result = alternation(*result, option);
				option = empty_syntree();
				break;
			case '?':
				last = alternation(last, empty_syntree());
				break;
			case '*':
				if (!last) {
					//TODO error
				}
				last = repetition(last, 0, -1);
				break;
			case '+':
				if (!last) {
					//TODO error
				}
				last = repetition(last, 1, -1);
				break;
			case '{':
				//TODO custom repetitions
				break;
			case '[':
				//TODO range
				break;
			case '\\':
				//TODO escaped chars
				break;
			case '.':
				option = concatenation(option, last);
				last = range_syntree(0, 127);
				break;
			default:
				option = concatenation(option, last);
				last = range_syntree(ch, ch);
				break;
		}
		if (done) break;
	}
	option = concatenation(option, last);
	*result = alternation(*result, option);
	return i;
}

struct syntree* parse(char* re, int len) {
	struct syntree* res;
	int end = parse_impl(re, len, &res);
	if (!res) {
		//TODO
		return 0;
	}
	if (end != len) {
		//TODO
		return 0;
	}
	return res;
}
