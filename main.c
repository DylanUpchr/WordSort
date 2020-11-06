#include <stdio.h>
#include <pthread.h> //Multithreading
#include "main.h"    //Main header

//Thread safe queue https://github.com/petercrona/StsQueue
#include "sts_queue/sts_queue.h"

//Asynchronous File I/O
#include "aio.h"
#include "sys/types.h"
#include "errno.h"

int main(int argc, char const *argv[])
{
    //Declaration
    //Word w1, w2;

    //Assign
    //w1.value = "test";
    StsHeader *handle = StsQueue.create();

    //Build queue
    /*StsQueue.push(handle, &w1.value);
    StsQueue.push(handle, &w1.value);
    StsQueue.push(handle, &w1.value);*/

    //Read file
    FILE *file = fopen(PATH, "r");
    char str[10];
    struct aiocb cb;
    if (file != NULL)
    {
        
    }
    else
    {
        printf("Unable to open file %s\r\n", PATH);
    }

    //Element treatment
    Word *currentWord;
    while ((currentWord = StsQueue.pop(handle)) != NULL)
    {
        printf("%s\r\n", currentWord->value);
    }
    return 0;
}
/*void master(Queue q){

}*/