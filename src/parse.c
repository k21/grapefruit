#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "const.h"
#include "parse.h"

static struct syntree* new_syntree(void) {
	return alloc(sizeof(struct syntree));
}

static void parse_error(char* re, uintptr_t pos, char* errmsg) {
	fprintf(stderr, "The regular expression could not be parsed!\n\n");
	fprintf(stderr, "Error: %s\n", errmsg);
	fprintf(stderr, "  Pos: %"PRIuPTR"\n", pos+1);
	if (strlen(re) <= 70) {
		fprintf(stderr, "Regex: %s\n", re);
		uintptr_t i;
		for (i = 0; i < pos+7; ++i) {
			fputc(' ', stderr);
		}
		fputs("^\n", stderr);
	}
	exit(2);
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
	res->data.alter.option1 = opt1;
	res->data.alter.option2 = opt2;
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
	res->data.concat.part1 = part1;
	res->data.concat.part2 = part2;
	return res;
}

static struct syntree* repetition(struct syntree* repeated,
		int_fast16_t min, int_fast16_t max) {
	if (!repeated) return 0;
	if (repeated->type == EMPTY) return repeated;
	struct syntree* res = new_syntree();
	res->type = REPEAT;
	res->data.repeat.repeated = repeated;
	res->data.repeat.min = min;
	res->data.repeat.max = max;
	return res;
}

static struct syntree* range_syntree(uint_fast8_t min, uint_fast8_t max) {
	struct syntree* res = new_syntree();
	res->type = RANGE;
	res->data.range.min = min;
	res->data.range.max = max;
	return res;
}

static struct syntree* empty_syntree(void) {
	struct syntree* res = new_syntree();
	res->type = EMPTY;
	return res;
}

static uintptr_t class_syntree(char* re, uintptr_t len, uintptr_t i,
		struct syntree** result) {
	if (i+1 >= len) {
		parse_error(re, i, "Unmatched \"[\"");
	}
	++i;
	uintptr_t begin = i;
	while (i == begin || re[i] != ']' || (re[begin] == '^' && i == begin+1)) {
		++i;
		if (i >= len) {
			parse_error(re, begin-1, "Unmatched \"[\"");
		}
	}
	uintptr_t end = i;
	bool invert = false;
	if (re[begin] == '^') {
		invert = true;
		++begin;
	}
	uintptr_t j;
	bool in_class[ALPHABET_SIZE+1] = {0};
	for (j = begin; j < end; ++j) {
		uint_fast8_t ch = re[j];
		if (j+2 < end && re[j+1] == '-') {
			j += 2;
			uint_fast8_t ch2 = re[j];
			if (ch2 < ch) parse_error(re, j-1, "Invalid range");
			while (ch <= ch2) {
				in_class[(uintptr_t)ch] = true;
				++ch;
			}
		} else {
			in_class[(uintptr_t)ch] = true;
		}
	}
	in_class[ALPHABET_SIZE] = invert;
	*result = 0;
	uintptr_t range_start = 0;
	bool prev_in_class = false;
	for(j = 0; j < ALPHABET_SIZE+1; ++j) {
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

static uintptr_t escaped_syntree(char* re, uintptr_t len, uintptr_t begin,
		struct syntree** result) {
	if (begin >= len) {
		parse_error(re, begin-1, "Incomplete escape sequence");
	}
	uint_fast8_t ch;
	uintptr_t end = begin;
	switch (re[begin]) {
		case '(': case ')': case '|': case '?': case '*': case '+':
		case '{': case '}': case '[': case ']': case '.': case '\\':
		case '\'': case '"': case '^': case '$':
			ch = re[begin];
			break;
		case 'a': ch = 7; break;
		case 'b': ch = 8; break;
		case 'f': ch = 12; break;
		case 'n': ch = 10; break;
		case 'r': ch = 13; break;
		case 't': ch = 9; break;
		case 'v': ch = 11; break;
		case 'x':
			end = begin+2;
			if (end >= len) {
				parse_error(re, begin, "Incomplete \\x escape sequence");
			}
			{
				uint_fast32_t code = 0;
				uintptr_t i;
				for (i = 1; i < 3; ++i) {
					uint_fast8_t c = re[begin+i];
					if (c >= '0' && c <= '9') {
						c -= '0';
					} else if (c >= 'a' && c <= 'f') {
						c -= 'a'-10;
					} else if (c >= 'A' && c <= 'F') {
						c -= 'A'-10;
					} else {
						parse_error(re, begin+i, "Invalid hexadecimal character in \\x "
								"escape sequence");
					}
					code *= 16;
					code += c;
				}
				if (code > MAX_CHAR) {
					parse_error(re, begin, "Escaped non-ASCII character");
				}
				ch = code;
			}
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			end = begin+2;
			if (end >= len) {
				parse_error(re, begin, "Incomplete \\000 escape sequence");
			}
			{
				uint_fast32_t code = 0;
				uintptr_t i;
				for (i = 0; i < 3; ++i) {
					uint_fast8_t c = re[begin+i];
					if (c < '0' || c > '7') {
						parse_error(re, begin+i, "Invalid octal character in \\000 "
								"escape sequence");
					}
					code *= 8;
					code += c-'0';
				}
				if (code > MAX_CHAR) {
					parse_error(re, begin, "Escaped non-ASCII character");
				}
				ch = code;
			}
			break;
		default:
			parse_error(re, begin, "Unknown escape sequence");
	}
	*result = range_syntree(ch, ch);
	return end;
}

static int_fast32_t parse_number(char* data, uintptr_t begin, uintptr_t end,
		int_fast32_t limit) {
	uintptr_t i;
	int_fast32_t res = 0;
	for (i = begin; i < end; ++i) {
		if (data[i] < '0' || data[i] > '9') {
			parse_error(data, i, "Invalid decimal character");
		}
		res *= 10;
		res += data[i]-'0';
		if (res > limit) return -1;
	}
	return res;
}

static uintptr_t custom_repetition(char* re, uintptr_t len, uintptr_t begin,
		struct syntree** last) {
	if (begin >= len) {
		parse_error(re, begin-1, "Unmatched \"{\"");
	}
	if (re[begin] == '}') {
		parse_error(re, begin-1, "Empty repetition range");
	}
	uintptr_t i = begin;
	while (re[i] != '}') {
		++i;
		if (i >= len) {
			parse_error(re, begin-1, "Unmatched \"{\"");
		}
	}
	uintptr_t end = i;
	uintptr_t middle = end;
	for (i = begin; i < end; ++i) {
		if (re[i] == ',') {
			middle = i;
			break;
		}
	}
	int_fast32_t limit = 10000;
	char err_over_limit[100];
	sprintf(err_over_limit, "Too many repetitions (> %"PRIdFAST32")", limit);
	if (middle == begin) {
		int_fast32_t max = parse_number(re, middle+1, end, limit);
		if (max == -1) parse_error(re, middle+1, err_over_limit);
		*last = repetition(*last, 0, max);
		return end;
	} else {
		int_fast32_t min = parse_number(re, begin, middle, limit);
		if (min == -1) parse_error(re, begin, err_over_limit);
		if (middle == end) {
			*last = repetition(*last, min, min);
			return end;
		}
		if (middle == end-1) {
			*last = repetition(*last, min, -1);
			return end;
		} else {
			int_fast32_t max = parse_number(re, middle+1, end, limit);
			if (max == -1) parse_error(re, middle+1, err_over_limit);
			if (max < min) parse_error(re, middle, "Invalid repetition range");
			*last = repetition(*last, min, max);
			return end;
		}
	}
}

static uintptr_t parse_impl(char* re, uintptr_t len, uintptr_t begin,
		struct syntree** result) {
	struct syntree* last = empty_syntree();
	struct syntree* option = empty_syntree();
	*result = 0;
	uintptr_t i;
	bool done = false;
	for (i = begin; i < len; ++i) {
		uint_fast8_t ch = re[i];
		switch (ch) {
			case '(':
				option = concatenation(option, last);
				++i;
				i = parse_impl(re, len, i, &last);
				//TODO check for error
				break;
			case ')':
				if (begin == 0) {
					parse_error(re, i, "Unmatched \")\"");
				}
				done = true;
				break;
			case '|':
				option = concatenation(option, last);
				last = empty_syntree();
				*result = alternation(*result, option);
				option = empty_syntree();
				break;
			case '?':
				last = alternation(last, empty_syntree());
				break;
			case '*':
				last = repetition(last, 0, UNLIMITED);
				break;
			case '+':
				last = repetition(last, 1, UNLIMITED);
				break;
			case '{':
				i = custom_repetition(re, len, i+1, &last);
				break;
			case '[':
				option = concatenation(option, last);
				i = class_syntree(re, len, i, &last);
				break;
			case '\\':
				option = concatenation(option, last);
				i = escaped_syntree(re, len, i+1, &last);
				break;
			case '.':
				option = concatenation(option, last);
				last = range_syntree(0, MAX_CHAR);
				break;
			default:
				option = concatenation(option, last);
				last = range_syntree(ch, ch);
				break;
		}
		if (done) break;
	}
	if (i == len && begin != 0) {
		parse_error(re, begin-1, "Unmatched \"(\"");
	}
	option = concatenation(option, last);
	*result = alternation(*result, option);
	return i;
}

struct syntree* parse(char* re, uintptr_t len) {
	uintptr_t i;
	for (i = 0; i < len; ++i) {
		if ((uint_fast8_t)re[i] > MAX_CHAR) {
			parse_error(re, i, "Non-ASCII character");
		}
	}
	struct syntree* res;
	parse_impl(re, len, 0, &res);
	return res;
}
