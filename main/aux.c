#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Auxiliar para concatenar strings
// char* concat(char* _str1, char* _str2) {
//   char* str1 = (char*)malloc(strlen(_str1) + 1);
//   char* str2 = (char*)malloc(strlen(_str2) + 1);
//   strcpy(str1, _str1);
//   strcpy(str2, _str2);
//   size_t len = strlen(str1) + strlen(str2) + 1;
//   char* _str = (char*)realloc(str1, sizeof(char) * len);
//   str1 = _str;
//   strcat(str1, str2);
//   return str1;
// }
char* concat(char* _str1, char* _str2) {
  char* str = (char*)malloc(strlen(_str1) + strlen(_str2) + 2);
  sprintf(str, "%s%s", _str1, _str2);
  // printf("%s", str);
  return str;
}

// Auxiliar para converter inteiro em string
char* to_s(int _num) {
  int len = snprintf(NULL, 0, "%d", _num) + 1;
  char* str = malloc(len);
  snprintf(str, len, "%d", _num);
  return str;
}

// Auxiliar para converter float em string
char* float_to_s(float _num) {
  int len = snprintf(NULL, 0, "%.2f", _num) + 1;
  char* str = malloc(len);
  snprintf(str, len, "%.2f", _num);
  return str;
}

// Auxiliar para dividir string por um delimitador
char** split(char* _str, char _delim) {
  char** result = 0;
  size_t count = 0;
  char* tmp = _str;
  char* last_comma = 0;
  char delim[2];
  delim[0] = _delim;
  delim[1] = 0;

  while (*tmp) {
    if (_delim == *tmp) {
      count++;
      last_comma = tmp;
    }
    tmp++;
  }

  count += last_comma < (_str + strlen(_str) - 1);
  count++;
  result = malloc(sizeof(char*) * count);

  if (result) {
    size_t idx = 0;
    char* token = strtok(_str, delim);

    while (token) {
      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    assert(idx == count - 1);
    *(result + idx) = 0;
  }

  return result;
}

