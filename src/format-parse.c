#include "format-parse.h"
#include "re.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fp_format_replace_s {
  fp_pattern_t **patterns;
  int patterns_len;
};

struct fp_pattern_s {
  char open;
  char close;
  re_t pattern;
  fp_replace_fn replacer;
  fp_release_fn releaser;
  void *data;
};

fp_format_t *fp_format_new() {
  struct fp_format_replace_s *s =
      (struct fp_format_replace_s *)malloc(sizeof(struct fp_format_replace_s));

  if (!s) {
    return NULL;
  }
  s->patterns_len = 0;
  s->patterns = NULL;

  return s;
}

void fp_format_pattern_add(fp_format_t *format, fp_pattern_t *pattern) {
  if (!format->patterns) {
    format->patterns =
        (struct fp_pattern_s **)malloc(sizeof(struct fp_pattern_s) * 1);
    format->patterns_len = 1;
  } else {
    format->patterns_len++;
    format->patterns = (struct fp_pattern_s **)realloc(
        format->patterns, sizeof(struct fp_pattern_s) * format->patterns_len);
  }
  format->patterns[format->patterns_len - 1] = pattern;
}

void fp_format_free(fp_format_t *format) {
  if (!format) {
    return;
  }
  if (format->patterns_len > 0) {
    for (int i = 0; i < format->patterns_len; i++) {
      fp_pattern_free(format->patterns[i]);
    }
  }
  free(format);
}

fp_pattern_t *fp_pattern_new(char open, char close, fp_replace_fn replace) {
  struct fp_pattern_s *p =
      (struct fp_pattern_s *)malloc(sizeof(struct fp_pattern_s));
  if (p == NULL)
    return NULL;

  p->open = open;
  p->close = close;
  p->pattern = NULL;
  p->replacer = replace;
  p->releaser = NULL;
  p->data = NULL;

  return p;
}

void fp_pattern_set_release(fp_pattern_t *pattern, fp_release_fn release) {
  pattern->releaser = release;
}

void fp_pattern_set_pattern(fp_pattern_t *pattern, const char *p) {
  re_t re = re_compile(p);
  pattern->pattern = re;
}

void fp_pattern_set_data(fp_pattern_t *pattern, void *data) {
  pattern->data = data;
}

void *fp_pattern_get_data(fp_pattern_t *pattern) { return pattern->data; }

void fp_pattern_free(fp_pattern_t *pattern) {
  if (pattern == NULL)
    return;
  free(pattern);
}

static int match_or(const char *input, int len, fp_pattern_t *p) {
  if (input[0] != p->open) {
    return -1;
  }
  int i = 1;
  char c;
  char current[2];
  current[1] = '\0';
  while (i < len) {
    c = input[i];
    if (c == p->close) {
      return i;
    }
    i++;
    if (p->pattern) {
      current[0] = c;
      if (re_matchp(p->pattern, current) == -1) {
        return -1;
      }
    } else if (isspace(c)) {
      return -1;
    } else {
      continue;
    }
  }
  return -1;
}

bool fp_format_parse(fp_format_t *format, const char *input, int len,
                     fp_buffer_t *out) {

  int i = 0, li = 0;
  char c;

  out->len = 0;

  while (i < len) {
    int pi = 0;
    int ni = -1;
    fp_pattern_t *pp = NULL;
    while (pi < format->patterns_len) {
      pp = format->patterns[pi++];
      ni = match_or(input + i, len - i, pp);
      if (ni != -1) {
        break;
      }
    }
    if (ni != -1) {
      if (i > 0) {
        int idx = out->len++;
        out->chunks[idx].data = (char *)input + li;
        out->chunks[idx].len = i - li;
        out->chunks[idx].pattern = NULL;
      }

      int idx = out->len++;

      if (!pp->replacer(input + i + 1, ni - 1, &out->chunks[idx], pp->data)) {
        return false;
      }
      out->chunks[idx].pattern = pp;
      i += ni + 1;
      li = i;

    } else {
      i++;
    }
  }

  int idx = out->len++;
  out->chunks[idx].data = (char *)input + li;
  out->chunks[idx].len = len + li;

  return true;
}

int fp_buffer_size(fp_buffer_t *buf) {
  int size = 0;
  for (int i = 0; i < buf->len; i++) {
    size += buf->chunks[i].len;
  }
  return size;
}

void fp_buffer_write(fp_buffer_t *buf, char *out) {
  int idx = 0;
  for (int i = 0; i < buf->len; i++) {
    memcpy(out + idx, buf->chunks[i].data, buf->chunks[i].len);
    idx += buf->chunks[i].len;
  }
  out[idx] = '\0';
}

void fp_buffer_print(fp_buffer_t *buf) {
  for (int i = 0; i < buf->len; i++) {
    printf("%.*s", buf->chunks[i].len, buf->chunks[i].data);
  }
}

char *fp_buffer_to_string(fp_buffer_t *buf) {
  char *str = (char *)malloc(sizeof(char) * fp_buffer_size(buf));
  if (buf == NULL)
    return NULL;

  fp_buffer_write(buf, str);

  return str;
}

void fp_buffer_free(fp_buffer_t *buf) {
  for (int i = 0; i < buf->len; i++) {
    if (buf->chunks[i].pattern && buf->chunks[i].pattern->releaser) {
      buf->chunks[i].pattern->releaser(&buf->chunks[i],
                                       buf->chunks[i].pattern->data);
    }
  }
}