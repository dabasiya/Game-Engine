#include <stdio.h>

int main() {
        int a = 0, b = 1, c;
        int n, index = 0;
        scanf("%d", &n);

        while(index < n) {
                c = a + b;
                printf("%d\n", a);
                a = b;
                b = c;
                index++;
        }
}
