#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void rnd(int a,int b,int n) {
  //random numbers create and output
  //a:min val
  //b:max val
  //n:random number,when n is less than 0,it represents the sum of n random numbers
  printf("> ");
  if (n < 0) {
    int s = 0;
    for (int i=n; i; ++i)
      s += rand() % (b - a + 1) + a;
    printf("%d\n", s);
  } else {
    for (int i=n; i; --i)
      printf("%d%c", rand() % (b - a + 1) + a, i - 1? ',': '\n');
  }
}

int main() {
  srand(time(NULL));
  puts("format: min_val max_val count(<0:sum of rnd num)");
  while (1) {
    int a, b, n;
    printf("INPUT: ");
    if (scanf("%d%d%d", &a, &b, &n) != 3) continue; // invalid input
    rnd(a, b, n);
  }
  return 0;
}
