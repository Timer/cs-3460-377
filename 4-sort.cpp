#include <future>
#include <cstdio>
#include <cstdlib>
#include <time.h>

#define PRINT true

void merge_sort(int array[], const int left, const int right) {
  if (right - left < 1) return;
  const int middle = (left + right) / 2;
  auto lh = std::async(std::launch::async, [array, left, middle]() {
    merge_sort(array, left, middle);
  });
  auto rh = std::async(std::launch::async, [array, middle, right]() {
    merge_sort(array, middle + 1, right);
  });
  lh.get();
  rh.get();
  int temp[right - left + 1];
  int k = 0, l = left, m = middle + 1;
  while (l <= middle && m <= right) {
    if (array[l] < array[m]) {
      temp[k++] = array[l++];
    } else {
      temp[k++] = array[m++];
    }
  }
  while (l <= middle) temp[k++] = array[l++];
  while (m <= right) temp[k++] = array[m++];
  for (int v = 0, i = left; i <= right; ++i, ++v) array[i] = temp[v];
}

int main(int argc, char **argv) {
  srand(time(nullptr));
  int array_size = 100;
  int *arr = new int[array_size];
  for (int i = 0; i < array_size; ++i) arr[i] = rand() % 1000;
  merge_sort(arr, 0, array_size - 1);
#if PRINT
  for (int i = 0; i < array_size; ++i) printf("%d ", arr[i]);
  puts("");
#endif
  delete[] arr;
  return 0;
}
