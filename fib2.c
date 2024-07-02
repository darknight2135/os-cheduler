#include <stdio.h>
#include "dummy_main.h"
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}
int main(int argc, char **argv) {

        printf("%d\n", fibonacci(30));
        exit(0);
//   int i, n = 1000;

//   // initialize first and second terms
//   int t1 = 0, t2 = 1;

//   // initialize the next term (3rd term)
//   int nextTerm = t1 + t2;

//   // get no. of terms from user
//   printf("Enter the number of terms: ");
// //   scanf("%d", &n);

//   // print the first two terms t1 and t2
//   printf("Fibonacci Series: %d, %d, ", t1, t2);

//   // print 3rd to nth terms
//   for (i = 3; i <= n; ++i) {
//     printf("%d, ", nextTerm);
//     t1 = t2;
//     t2 = nextTerm;
//     nextTerm = t1 + t2;
//   }

}
