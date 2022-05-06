#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <libgen.h>
#include <wordexp.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "header.h"

typedef enum keyword keyword;

enum keyword {
	NIL = -1,
	INHERITS
};

static const char *keywords[] = {
	[INHERITS] = "inherits",
	NULL
};

static int get_line(char **result, size_t *result_len, FILE *f, int *line) {
	size_t pos;

	do {
		pos = 0;

		do {
			if (*result_len - pos < 10) {
				char *newstr = realloc(*result, *result_len * 2 + 80);
				if (newstr == NULL) {
					return 0;
				}

				*result = newstr;
				*result_len = *result_len * 2 + 80;
			}

			if (!fgets(*result + pos, *result_len - pos, f)) {
				if (ferror(f) == 0 && pos == 0)
					return 0;

				if (ferror(f) != 0)
					return 0;
			}

			pos += strlen(*result + pos);
		} while (pos == *result_len - 1 && (*result)[pos - 1] != '\n');

		if (pos != 0 && (*result)[pos - 1] == '\n')
			(*result)[--pos] = '\0';

		(*line)++;

		assert((*result)[pos] == '\0');

		int first = 0;

		while (isspace((*result)[first]) && (*result)[first])
			first++;

		memmove(*result, *result + first, pos - first + 1);
		pos -= first;
	} while ((*result)[0] == '#');

	while (pos && (*result)[pos - 1] == '\\') {
		(*result)[--pos] = '\0';

		do {
			if (*result_len - pos < 10) {
				char *newstr = realloc(*result, *result_len * 2 + 80);
				if (newstr == NULL) {
					return 0;
				}

				*result = newstr;
				*result_len = *result_len * 2 + 80;
			}
			if (!fgets(*result + pos, *result_len - pos, f)) {
				if (ferror(f) == 0 && pos == 0)
					return 0;

				if (ferror(f) != 0)
					return 0;
			}

			pos += strlen(*result + pos);
		} while (pos == *result_len - 1 && (*result)[pos - 1] != '\n');

		if (pos != 0 && (*result)[pos - 1] == '\n')
			(*result)[--pos] = '\0';

		(*line)++;

		assert((*result)[pos] == '\0');
	}

	while (pos && isspace((*result)[pos - 1]))	/* remove trailing whitespace */
		pos--;

	(*result)[pos] = '\0';

	return 1;
}

static char *next_word(char *buf, char *word, int maxlen) {
	if (!buf)
		return NULL;

	if (!*buf)
		return NULL;

	while (!isspace(*buf) && *buf) {
		if (maxlen-- > 1)
			*word++ = *buf;

		buf++;
	}

	if (maxlen > 0)
		*word = '\0';

	while (isspace(*buf) && *buf)
		buf++;

	return buf;
}

static address_family *get_address_family(address_family *af[], const char *name) {
	for (int i = 0; af[i]; i++)
		if (strcmp(af[i]->name, name) == 0)
			return af[i];

	return NULL;
}

static method *get_method(address_family *af, const char *name) {
	for (int i = 0; i < af->n_methods; i++)
		if (strcmp(af->method[i].name, name) == 0)
			return &af->method[i];

	return NULL;
}

static keyword get_keyword(const char *word) {
	for (int i = 0; keywords[i]; i++) {
		if (strcmp(keywords[i], word) == 0) {
			return i;
		}
	}

	return -1;
}

static allowup_defn *get_allowup(allowup_defn **allowups, const char *name) {
	for (; *allowups; allowups = &(*allowups)->next)
		if (strcmp((*allowups)->when, name) == 0)
			break;

	if (*allowups == NULL) {
		*allowups = calloc(1, sizeof **allowups);
		if (*allowups == NULL)
			return NULL;

		(*allowups)->when = strdup(name);
	}

	return *allowups;
}

static allowup_defn *add_allow_up(const char *filename, int line, allowup_defn *allow_up, const char *iface_name) {
	for (int i = 0; i < allow_up->n_interfaces; i++)
		if (strcmp(iface_name, allow_up->interfaces[i]) == 0)
			return allow_up;

	if (allow_up->n_interfaces == allow_up->max_interfaces) {
		char **tmp;

		allow_up->max_interfaces *= 2;
		allow_up->max_interfaces++;

		tmp = realloc(allow_up->interfaces, sizeof(*tmp) * allow_up->max_interfaces);
		if (tmp == NULL) {
			perror(filename);
			return NULL;
		}

		allow_up->interfaces = tmp;
	}

	allow_up->interfaces[allow_up->n_interfaces] = strdup(iface_name);
	allow_up->n_interfaces++;

	return allow_up;
}

variable *set_variable(const char *name, const char *value, variable **var, int *n_vars, int *max_vars) {
	/*
	 * if name ends with '?', don't update
	 * the variable if it already exists
	 */
	bool dont_update = false;

	size_t len = strlen(name);

	if (name[len - 1] == '?') {
		dont_update = true;
		len--;
	}

	if (strcmp(name, "pre-up") != 0 && strcmp(name, "up") != 0 && strcmp(name, "down") != 0 && strcmp(name, "post-down") != 0) {
		for (int j = 0; j < *n_vars; j++) {
			if (strncmpz(name, (*var)[j].name, len) == 0) {
				if (dont_update)
					return NULL;

				if ((*var)[j].value == value)
					return &(*var)[j];

				free((*var)[j].value);
				(*var)[j].value = strdup(value);

				if (!(*var)[j].value) {
					perror(argv0);
					return NULL;
				}

				return &((*var)[j]);
			}
		}
	}

	if (*n_vars >= *max_vars) {
		variable *new_var;

		*max_vars += 10;
		new_var = realloc(*var, sizeof *new_var * *max_vars);

		if (new_var == NULL) {
			perror(argv0);
			return NULL;
		}

		*var = new_var;
	}

	(*var)[*n_vars].name = strndup(name, len);
	(*var)[*n_vars].value = strdup(value);

	if (!(*var)[*n_vars].name) {
		perror(argv0);
		return NULL;
	}

	if (!(*var)[*n_vars].value) {
		perror(argv0);
		return NULL;
	}

	(*n_vars)++;
	return &((*var)[(*n_vars) - 1]);
}

void convert_variables(conversion *conversions, interface_defn *ifd) {
	for (conversion *c = conversions; c && c->option && c->fn; c++) {
		if (strcmp(c->option, "iface") == 0) {
			if (c->newoption) {
				variable *o = set_variable(c->newoption, ifd->real_iface, &ifd->option, &ifd->n_options, &ifd->max_options);
				if (o)
					c->fn(ifd, &o->value, c->argc, c->argv);
				continue;
			}
		}

		for (int j = 0; j < ifd->n_options; j++) {
			if (strcmp(ifd->option[j].name, c->option) == 0) {
				if (c->newoption) {
					variable *o = set_variable(c->newoption, ifd->option[j].value, &ifd->option, &ifd->n_options, &ifd->max_options);
					if (o)
						c->fn(ifd, &o->value, c->argc, c->argv);
				} else {
					variable *o = &(ifd->option[j]);
					c->fn(ifd, &o->value, c->argc, c->argv);
				}
			}
		}
	}
}

static int directory_filter(const struct dirent *d) {
	if (d == NULL || d->d_name[0] == 0)
		return 0;

	for (const char *p = d->d_name; *p; p++)
		if (!(((*p >= 'a') && (*p <= 'z')) || ((*p >= 'A') && (*p <= 'Z')) || ((*p >= '0') && (*p <= '9')) || (*p == '_') || (*p == '-')))
			return 0;

	return 1;
}

struct seen_file {
	struct seen_file *next;
	char *filename;
} *seen_files = NULL;

static bool already_seen(const char *filename) {
	if(seen_files)
		for(struct seen_file *seen = seen_files; seen; seen = seen->next)
			if(strcmp(seen->filename, filename) == 0)
				return true;

	struct seen_file *seen = malloc(sizeof *seen);
	if(!seen) {
		perror("malloc");
		return false;
	}

	seen->filename = strdup(filename);
	if(!seen->filename) {
		free(seen);
		perror("strdup");
		return false;
	}

	seen->next = seen_files;
	seen_files = seen;

	return false;
}

static void clear_seen(void) {
	while(seen_files) {
		struct seen_file *seen = seen_files;
		seen_files = seen->next;
		free(seen->filename);
		free(seen);
	}
}

static interface_defn *get_interface(interfaces_file *defn, const char *iface, const char *addr_fam) {
	for (interface_defn *currif = defn->ifaces; currif; currif = currif->next) {
		if (strcmp(iface, currif->logical_iface) == 0) {
			/* addr_fam == NULL matches any address family */
			if ((addr_fam == NULL) || (strcmp(addr_fam, currif->address_family->name) == 0))
				return currif;
		}
	}
	return NULL;
}

static interface_defn *copy_variables(interface_defn *destif, interface_defn *srcif) {
	if (srcif->n_options > destif->max_options) {
		variable *new_option;
		new_option = realloc(destif->option, sizeof *new_option * srcif->n_options);

		if (new_option == NULL) {
			perror(argv0);
			return NULL;
		}

		destif->option = new_option;
		destif->max_options = srcif->n_options;
	}

	for (int i = 0; i < srcif->n_options; i++) {
		destif->option[i].name = strdup(srcif->option[i].name);
		if (!destif->option[i].name) {
			perror(argv0);
			return NULL;
		}

		destif->option[i].value = strdup(srcif->option[i].value);
		if (!destif->option[i].value) {
			perror(argv0);
			return NULL;
		}
	}
	destif->n_options = srcif->n_options;

	return destif;
}

static void add_to_list(char ***list, int *count, const char *item) {
	(*count)++;
	*list = realloc(*list, sizeof **list * *count);
	if (!*list)
		perror(argv0);
	(*list)[*count - 1] = strdup(item);
	if (!(*list)[*count - 1])
		perror(argv0);
}

static interfaces_file *read_interfaces_defn(interfaces_file *defn, const char *filename) {
	FILE *f;
	int line;
	char *buf = NULL;
	size_t buf_len = 0;
	interface_defn *currif = NULL;
	mapping_defn *currmap = NULL;
	enum { NONE, IFACE, MAPPING } currently_processing = NONE;
	char firstword[80];
	char *rest;

	if(already_seen(filename))
		return NULL;

	f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "warning: couldn't open interfaces file \"%s\"\n", filename);
		return defn;
	}

	line = 0;

	while (get_line(&buf, &buf_len, f, &line)) {
		rest = next_word(buf, firstword, 80);
		if (rest == NULL)
			continue;	/* blank line */

		if (strcmp(firstword, "mapping") == 0) {
			currmap = calloc(1, sizeof *currmap);
			if (currmap == NULL) {
				perror(filename);
				return NULL;
			}

			while ((rest = next_word(rest, firstword, 80))) {
				if (currmap->max_matches == currmap->n_matches) {
					char **tmp;

					currmap->max_matches = currmap->max_matches * 2 + 1;
					tmp = realloc(currmap->match, sizeof(*tmp) * currmap->max_matches);
					if (tmp == NULL) {
						currmap->max_matches = (currmap->max_matches - 1) / 2;
						perror(filename);
						return NULL;
					}
					currmap->match = tmp;
				}

				currmap->match[currmap->n_matches++] = strdup(firstword);
			}

			mapping_defn **where = &defn->mappings;

			while (*where != NULL)
				where = &(*where)->next;

			*where = currmap;
			currmap->next = NULL;

			currently_processing = MAPPING;
		} else if (strcmp(firstword, "source") == 0) {
			char *filename_dup = strdup(filename);
			if (filename_dup == NULL) {
				perror(filename);
				return NULL;
			}

			char *dir = strdup(dirname(filename_dup));
			if (dir == NULL) {
				perror(filename);
				return NULL;
			}

			free(filename_dup);

			size_t l = strlen(dir);
			char *pattern;

			if (rest[0] == '/') {
				size_t s = strlen(rest) + 1;	/* + NUL */

				pattern = malloc(s);
				if (pattern == NULL) {
					free(dir);
					perror(filename);
					return NULL;
				}

				pattern[0] = '\0';
			} else {
				size_t s = l + strlen(rest) + 2;	/* + slash + NUL */

				pattern = malloc(s);
				if (pattern == NULL) {
					free(dir);
					perror(filename);
					return NULL;
				}

				pattern[0] = '\0';
				strcat(pattern, dir);
				strcat(pattern, "/");
			}

			strcat(pattern, rest);

			wordexp_t p;
			int fail = wordexp(pattern, &p, WRDE_NOCMD);

			if (!fail) {
				char **w = p.we_wordv;

				for (size_t i = 0; i < p.we_wordc; i++) {
					struct stat sb;

					if (stat(w[i], &sb) == -1)
						/* wordexp can't expand * in an empty dir */
						if (errno == ENOENT)
							continue;

					if (verbose)
						fprintf(stderr, "Parsing file %s\n", w[i]);

					read_interfaces_defn(defn, w[i]);
				}

				wordfree(&p);
			}

			free(pattern);
			free(dir);
			currently_processing = NONE;
		} else if (strlmatch(firstword, "source-dir") == 0) {
			char *filename_dup = strdup(filename);
			if (filename_dup == NULL) {
				perror(filename);
				return NULL;
			}

			char *dir = strdup(dirname(filename_dup));
			if (dir == NULL) {
				perror(filename);
				return NULL;
			}

			free(filename_dup);

			size_t l = strlen(dir);
			char *pattern;

			if (rest[0] == '/') {
				size_t s = strlen(rest) + 1;	/* + NUL */

				pattern = malloc(s);
				if (pattern == NULL) {
					free(dir);
					perror(filename);
					return NULL;
				}

				pattern[0] = '\0';
			} else {
				size_t s = l + strlen(rest) + 2;	/* + slash + NUL */

				pattern = malloc(s);
				if (pattern == NULL) {
					free(dir);
					perror(filename);
					return NULL;
				}

				pattern[0] = '\0';
				strcat(pattern, dir);
				strcat(pattern, "/");
			}

			strcat(pattern, rest);

			wordexp_t p;
			int fail = wordexp(pattern, &p, WRDE_NOCMD);

			if (!fail) {
				char **w = p.we_wordv;
				for (size_t i = 0; i < p.we_wordc; i++) {
					struct dirent **namelist;
					int n = scandir(w[i], &namelist, directory_filter, alphasort);

					if (n >= 0) {
						if (verbose)
							fprintf(stderr, "Reading directory %s\n", w[i]);

						size_t ll = strlen(w[i]);

						for (int j = 0; j < n; j++) {
							size_t s = ll + strlen(namelist[j]->d_name) + 2;	/* + slash + NUL */

							char *name = malloc(s);
							if (name == NULL) {
								perror(filename);
								return NULL;
							}

							name[0] = '\0';
							strcat(name, w[i]);
							strcat(name, "/");
							strcat(name, namelist[j]->d_name);

							if (verbose)
								fprintf(stderr, "Parsing file %s\n", name);

							read_interfaces_defn(defn, name);
							free(name);
						}

						free(namelist);
					}
				}
				wordfree(&p);
			}

			free(pattern);
			free(dir);
			currently_processing = NONE;
		} else if (strcmp(firstword, "iface") == 0) {
			char iface_name[80];
			char address_family_name[80];
			char method_name[80];
			char inherits[80];
			keyword kw = NIL;

			currif = malloc(sizeof *currif);
			if (!currif) {
				perror(filename);
				return NULL;
			}

			*currif = (interface_defn) {
				.max_options = 0,
				.n_options = 0,
				.option = NULL,
			};

			rest = next_word(rest, iface_name, 80);
			if (rest == NULL) {
				fprintf(stderr, "%s:%d: too few parameters for iface line\n", filename, line);
				free(currif);
				return NULL;
			}

			rest = next_word(rest, address_family_name, 80);

			if (rest != NULL) {
				currif->address_family = get_address_family(addr_fams, address_family_name);
				if (currif->address_family == NULL) {
					kw = get_keyword(address_family_name);
				} else {
					rest = next_word(rest, method_name, 80);
					if (rest != NULL) {
						currif->method = get_method(currif->address_family, method_name);
						if (currif->method == NULL) {
							kw = get_keyword(method_name);
						}
					}
				}
			}

			if ((currif->address_family == NULL) && (kw == NIL)) {
				fprintf(stderr, "%s:%d: unknown or no address type and no inherits keyword specified\n", filename, line);
				free(currif);
				return NULL;
			}

			if ((currif->method == NULL) && (kw == NIL)) {
				fprintf(stderr, "%s:%d: unknown or no method and no inherits keyword specified\n", filename, line);
				free(currif);
				return NULL;	/* FIXME */
			}

			if (kw == NIL) {
				rest = next_word(rest, inherits, 80);
				if (rest != NULL) {
					kw = get_keyword(inherits);
					if (kw == NIL) {
						fprintf(stderr, "%s:%d: extra parameter for the iface line not understood and ignored: %s\n", filename, line, inherits);
					}
				}
			}

			if (kw != NIL) {
				rest = next_word(rest, inherits, 80);
				if (rest == NULL) {
					fprintf(stderr, "%s:%d: '%s' keyword is missing a parameter\n", filename, line, keywords[kw]);
					free(currif);
					return NULL;
				}
				if (kw == INHERITS) {
					interface_defn *otherif = get_interface(defn, inherits, currif->address_family ? address_family_name : NULL);
					if (otherif == NULL) {
						fprintf(stderr, "%s:%d: unknown iface to inherit from: %s (%s)\n", filename, line, inherits, currif->address_family ? address_family_name : "*");
						free(currif);
						return NULL;
					}

					if (copy_variables(currif, otherif) == NULL) {
						free(currif);
						return NULL;
					}

					if (currif->address_family == NULL) {
						currif->address_family = otherif->address_family;
					}

					if (currif->method == NULL) {
						currif->method = otherif->method;
					}
				}
			}

			currif->logical_iface = strdup(iface_name);
			if (!currif->logical_iface) {
				perror(filename);
				free(currif);
				return NULL;
			}

			if (((!strcmp(address_family_name, "inet")) || (!strcmp(address_family_name, "inet6"))) && (!strcmp(method_name, "loopback")))
				no_loopback = true;

			interface_defn **where = &defn->ifaces;

			while (*where != NULL) {
				where = &(*where)->next;
			}

			*where = currif;
			currif->next = NULL;
			currently_processing = IFACE;
		} else if (strcmp(firstword, "auto") == 0) {
			allowup_defn *auto_ups = get_allowup(&defn->allowups, "auto");

			if (!auto_ups) {
				perror(filename);
				return NULL;
			}

			while ((rest = next_word(rest, firstword, 80)))
				if (!add_allow_up(filename, line, auto_ups, firstword))
					return NULL;

			currently_processing = NONE;
		} else if (strncmp(firstword, "allow-", 6) == 0 && strlen(firstword) > 6) {
			allowup_defn *allow_ups = get_allowup(&defn->allowups, firstword + 6);

			if (!allow_ups) {
				perror(filename);
				return NULL;
			}

			while ((rest = next_word(rest, firstword, 80)))
				if (!add_allow_up(filename, line, allow_ups, firstword))
					return NULL;

			currently_processing = NONE;
		} else if (strcmp(firstword, "no-auto-down") == 0) {
			while ((rest = next_word(rest, firstword, 80)))
				add_to_list(&no_auto_down_int, &no_auto_down_ints, firstword);

			currently_processing = NONE;
		} else if (strcmp(firstword, "no-scripts") == 0) {
			while ((rest = next_word(rest, firstword, 80)))
				add_to_list(&no_scripts_int, &no_scripts_ints, firstword);

			currently_processing = NONE;
		} else {
			switch (currently_processing) {
			case IFACE:
				if (strcmp(firstword, "post-up") == 0)
					strcpy(firstword, "up");

				if (strcmp(firstword, "pre-down") == 0)
					strcpy(firstword, "down");

				if (strlen(rest) == 0) {
					fprintf(stderr, "%s:%d: option with empty value\n", filename, line);
					return NULL;
				}

				if (strcmp(firstword, "pre-up") != 0 && strcmp(firstword, "up") != 0 && strcmp(firstword, "down") != 0 && strcmp(firstword, "post-down") != 0) {
					for (int i = 0; i < currif->n_options; i++) {
						if (strcmp(currif->option[i].name, firstword) == 0) {
							size_t l = strlen(currif->option[i].value);

							currif->option[i].value = realloc(currif->option[i].value, l + strlen(rest) + 2);	/* 2 for NL and NULL */
							if (!currif->option[i].value) {
								perror(filename);
								return NULL;
							}

							currif->option[i].value[l] = '\n';
							strcpy(&(currif->option[i].value[l + 1]), rest);
							rest = currif->option[i].value;
						}
					}
				}

				set_variable(firstword, rest, &currif->option, &currif->n_options, &currif->max_options);
				break;

			case MAPPING:
				if (strcmp(firstword, "script") == 0) {
					if (currmap->script != NULL) {
						fprintf(stderr, "%s:%d: duplicate script in mapping\n", filename, line);
						return NULL;
					} else {
						currmap->script = strdup(rest);
					}
				} else if (strcmp(firstword, "map") == 0) {
					if (currmap->max_mappings == currmap->n_mappings) {
						char **opt;

						currmap->max_mappings = currmap->max_mappings * 2 + 1;
						opt = realloc(currmap->mapping, sizeof(*opt) * currmap->max_mappings);
						if (opt == NULL) {
							perror(filename);
							return NULL;
						}

						currmap->mapping = opt;
					}

					currmap->mapping[currmap->n_mappings] = strdup(rest);
					currmap->n_mappings++;
				} else {
					fprintf(stderr, "%s:%d: misplaced option\n", filename, line);
					return NULL;
				}
				break;

			case NONE:
			default:
				fprintf(stderr, "%s:%d: misplaced option\n", filename, line);
				return NULL;
			}
		}
	}

	if (ferror(f) != 0) {
		perror(filename);
		return NULL;
	}

	fclose(f);
	line = -1;

	return defn;
}

interfaces_file *read_interfaces(const char *filename) {
	interfaces_file *defn;

	defn = calloc(1, sizeof *defn);
	if (defn == NULL)
		return NULL;

	if (!no_loopback)
		add_allow_up(__FILE__, __LINE__, get_allowup(&defn->allowups, "auto"), LO_IFACE);

	defn = read_interfaces_defn(defn, filename);
	if (!defn)
		return NULL;

	if (!no_loopback) {
		interface_defn *lo_if = malloc(sizeof *lo_if);

		if (!lo_if) {
			perror(filename);
			return NULL;
		}

		*lo_if = (interface_defn) {
			.logical_iface = strdup(LO_IFACE),
			.address_family = &addr_inet,
			.method = get_method(&addr_inet, "loopback"),
			.next = defn->ifaces
		};

		defn->ifaces = lo_if;
	}

	clear_seen();

	return defn;
}

allowup_defn *find_allowup(interfaces_file *defn, const char *name) {
	for (allowup_defn *allowups = defn->allowups; allowups; allowups = allowups->next)
		if (strcmp(allowups->when, name) == 0)
			return allowups;

	return NULL;
}
