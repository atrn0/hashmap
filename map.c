#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CAPACITY 17
#define TOMBSTONE ((char*)-1)
#define CAP_THRESHOLD 80  // %

typedef struct {
  char* key;
  int value;
} Item;

typedef struct {
  Item* items;
  int count;
  int capacity;
} HashMap;

void map_put(HashMap* map, char* key, int value);
void print_map(HashMap* map);

// generate FNV-1a hash
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
uint64_t hash(char* str) {
  uint64_t i = 14695981039346656037U;
  for (int j = 0; str[j]; j++) {
    i ^= str[j];
    i *= 1099511628211U;
  }
  return i;
}

HashMap* create_map(int cap) {
  HashMap* map = calloc(1, sizeof(HashMap));
  map->items = calloc(cap, sizeof(Item));
  map->count = 0;
  map->capacity = cap;
  return map;
}

void expand_map(HashMap* map) {
  int newcap = map->capacity * 2;
  HashMap* newmap = create_map(newcap);

  // copy all key-values
  for (int i = 0; i < map->capacity; i++) {
    Item* item = &map->items[i];
    if (!item->key || item->key == TOMBSTONE) continue;
    map_put(newmap, item->key, item->value);
  }

  *map = *newmap;
}

void map_put(HashMap* map, char* key, int value) {
  if ((map->count * 100) / map->capacity > CAP_THRESHOLD) expand_map(map);

  uint64_t hs = hash(key);
  for (int i = 0; i < map->capacity; i++) {
    Item* item = &(map->items[(hs + i) % map->capacity]);

    if (item->key == NULL) {
      item->key = key;
      item->value = value;
      map->count++;
      return;
    }

    if (item->key == TOMBSTONE) {
      item->key = key;
      item->value = value;
      return;
    }

    if (strcmp(item->key, key) == 0) {
      item->key = key;
      item->value = value;
      return;
    }
  }

  fprintf(stderr, "no space\n");
  exit(1);
  return;
}

int* map_find(HashMap* map, char* key) {
  uint64_t hs = hash(key);
  for (int i = 0; i < map->capacity; i++) {
    Item* item = &map->items[(hs + i) % map->capacity];
    if (item->key == NULL) return NULL;
    if (item->key == TOMBSTONE) continue;
    if (strcmp(item->key, key) == 0) return &item->value;
  }

  return NULL;
}

void map_remove(HashMap* map, char* key) {
  uint64_t hs = hash(key);
  for (int i = 0; i < map->capacity; i++) {
    Item* item = &map->items[(hs + i) % map->capacity];
    if (item->key == NULL) return;
    if (item->key == TOMBSTONE) continue;
    if (strcmp(item->key, key) == 0) {
      item->key = TOMBSTONE;
      return;
    }
  }
}

void print_map(HashMap* map) {
  printf("  {");
  for (int i = 0; i < map->capacity; i++) {
    Item* item = &map->items[i];
    if (item->key && item->key != TOMBSTONE)
      printf("%s: %d, ", item->key, item->value);
  }
  printf("}\n");
}

char* make_test_key(int i) {
  char* buf = malloc(10 * sizeof(char));
  sprintf(buf, "key_%d", i);
  return buf;
}

int main() {
  HashMap* map = create_map(INIT_CAPACITY);

  for (int i = 0; i < 500000; i++) {
    map_put(map, make_test_key(i), i);
  }
  for (int i = 0; i < 100000; i++) {
    map_remove(map, make_test_key(i));
  }
  for (int i = 0; i < 100000; i++) {
    int* v = map_find(map, make_test_key(i));
    assert(v == NULL);
  }
  for (int i = 100000; i < 500000; i++) {
    int* v = map_find(map, make_test_key(i));
    if (v == NULL) printf("map_find(map, \"Key_%d\") == NULL\n", i);
    assert(v && (*v == i));
  }

  printf("OK\n");
  return 0;
}
