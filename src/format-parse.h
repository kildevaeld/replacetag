#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FP_CHUNKS_MAX 200
#define FP_PATTERN_CHARS_MAX 200

typedef struct fp_format_replace_s fp_format_t;
typedef struct fp_pattern_s fp_pattern_t;

typedef struct fp_chunk_s {
  char *data;
  int len;
  fp_pattern_t *pattern;
} fp_chunk_t;

typedef struct fp_buffer_s {
  fp_chunk_t chunks[FP_CHUNKS_MAX];
  int len;
} fp_buffer_t;

typedef bool (*fp_replace_fn)(const char *input, int len, fp_chunk_t *chunk,
                              void *data);

typedef void (*fp_release_fn)(fp_chunk_t *chunk, void *data);

fp_format_t *fp_format_new();
void fp_format_free(fp_format_t *format);
void fp_format_pattern_add(fp_format_t *format, fp_pattern_t *pattern);

fp_pattern_t *fp_pattern_new(char open, char close, fp_replace_fn replace);
void fp_pattern_set_release(fp_pattern_t *pattern, fp_release_fn release);
void fp_pattern_set_pattern(fp_pattern_t *pattern, const char *p);
void fp_pattern_set_data(fp_pattern_t *pattern, void *data);
void *fp_pattern_get_data(fp_pattern_t *pattern);
void fp_pattern_set_include_tags(fp_pattern_t *pattern, bool include);
bool fp_pattern_get_include_tags(fp_pattern_t *pattern);
void fp_pattern_free(fp_pattern_t *pattern);

bool fp_format_parse(fp_format_t *format, const char *input, int len,
                     fp_buffer_t *output);

int fp_buffer_size(fp_buffer_t *buf);
void fp_buffer_write(fp_buffer_t *buf, char *out);
void fp_buffer_print(fp_buffer_t *buf);
char *fp_buffer_to_string(fp_buffer_t *buf);
void fp_buffer_free(fp_buffer_t *buf);

#ifdef __cplusplus
}
#endif