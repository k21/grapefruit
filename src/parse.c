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

static int class_syntree(char* re, int len, int i, struct syntree** result) {
	if (i+1 >= len) {
		die(1, 0, "Missing \"]\" for \"[\" at position %d", i+1);
	}
	++i;
	int begin = i;
	while (i == begin || re[i] != ']') {
		++i;
		if (i >= len) {
			die(1, 0, "Missing \"]\" for \"[\" at position %d", begin+1);
		}
	}
	int end = i;
	bool invert = false;
	if (re[begin] == '^') {
		invert = true;
		++begin;
	}
	int j;
	bool in_class[129] = {0};
	for (j = begin; j < end; ++j) {
		char ch = re[j];
		if (j+2 < end && re[j+1] == '-') {
			j += 2;
			char ch2 = re[j];
			if (ch2 < ch) die(1, 0, "Invalid range end at position %d", j+1);
			while (ch <= ch2) {
				in_class[(int)ch] = true;
				++ch;
			}
		} else {
			in_class[(int)ch] = true;
		}
	}
	in_class[129] = invert;
	*result = 0;
	int range_start = 0;
	bool prev_in_class = false;
	for(j = 0; j < 129; ++j) {
		if (in_class[j] != invert) {
			if (!prev_in_class) {
				prev_in_class = true;
				range_start = j;
			}
		} else {
			if (prev_in_class) {
				struct syntree* part = range_syntree(range_start, j-1);
				if (*result) {
					*result = alternation(*result, part);
				} else {
					*result = part;
				}
				prev_in_class = false;
			}
		}
	}
	return end;
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
				option = concatenation(option, last);
				i = class_syntree(re, len, i, &last);
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
