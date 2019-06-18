# ReplaceTag

```c

bool replace_link(const char *link, int len, fp_chunk_t *chunk, void *data) {
  chunk->data = (char *)link;
  chunk->len = len;
  return true;
}

bool replace_bold(const char *link, int len, fp_chunk_t *chunk, void *data) {
  char *str = malloc(sizeof(char) * (7 + len));
  memcpy(str, "<b>", 3);
  memcpy(str + 3, link, len);
  memcpy(str + 3 + len, "</b>", 4);
  chunk->data = str;
  chunk->len = len + 7;
  return true;
}

void bold_free(fp_chunk_t *chunk, void *data) {
  free(chunk->data);
}

int main() {

  const char *string = "<@U22kDFDF23> can't :smiley: or look as "
                       "<https//google.com> :should_not_be_Replaceed *it or what*";

  fp_format_t *format = fp_format_new();
  fp_format_pattern_add(format, fp_pattern_new('<', '>', replace_link));

  fp_pattern_t *pattern = fp_pattern_new('*', '*', replace_bold);
  // Allow anything inside tag except '*'
  fp_pattern_set_pattern(pattern, "[^*]");
  fp_pattern_set_release(pattern, bold_free);
  fp_format_pattern_add(format, pattern);
  fp_format_pattern_add(format, fp_pattern_new(':', ':', replace_link));

  fp_buffer_t out;
  fp_format_parse(format, string, strlen(string), &out);

  int size = fp_buffer_size(&out);
  char text[size + 1];

  fp_buffer_write(&out, text);
  printf("%s\n", text);
  fp_buffer_free(&out);
 
  fp_format_free(format);
}

```