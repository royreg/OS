#include "types.h"
#include "user.h"



int main() {
    int status;
    if (fork() >0) {
        wait(&status);
        if (status != 137) {
            printf(1, "Fail\n");
            printf(1,"the status is: %d\n",status);
        } else {
            printf(1, "Passed\n");
             printf(1,"the status is: %d\n",status);
        }
        exit(0);
    } else {
        exit(137);
    }
    return 0;
}