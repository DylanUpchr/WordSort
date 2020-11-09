#include <stdio.h>
#include <pthread.h> //Multithreading
#include "main.h"    //Main header
#include <string.h>  //String functions
#include <stdlib.h>
#include <sys/time.h> //Timing functions
#include <math.h>

//Queue https://github.com/petercrona/StsQueue
#include "inc/sts_queue/sts_queue.h"
#include <ctype.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
/*#define handle_error_en(en, msg) \
    do                           \
    {                            \
        errno = en;              \
        perror(msg);             \
        exit(EXIT_FAILURE);      \
    } while (0)

#define handle_error(msg)   \
    do                      \
    {                       \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)*/

struct thread_info
{                        /* Used as argument to thread_start() */
    pthread_t thread_id; /* ID returned by pthread_create() */
    int thread_num;      /* Application-defined thread # */
    char *argv_string;   /* From command-line argument */
};

/*void *master(void *rawLinesQueue)
{
    //Read file
    char str[10];
    Word word;
    FILE *file = fopen(PATH, "r");

    if (file != NULL)
    {
        while (fgets(str, 10, file) != NULL)
        {
            printf("%s", str);
            StsQueue.push(rawLinesQueue, str);
            //word.value = str;
        }
    }
    else
    {
        printf("Unable to open file %s\r\n", PATH);
    }
    Word word;
    word.value = "Test";
    //Word *currentWord;
    while ((currentWord = StsQueue.pop(queueHead)) != NULL)
    {
        printf("%s\r\n", currentWord->value);
    }

    return 0;
}*/
void reformat_string(char *src, char *dst)
{
    for (; *src; ++src)
        if (!ispunct((unsigned char)*src))
            *dst++ = tolower((unsigned char)*src);
    *dst = 0;
}

void InsertionSort(char **list, int listSize)
{
    for (int i = 1; i < listSize; i++)
    {
        int j = i;

        while (j > 0 && strcmp(list[j - 1], list[j]) > 0)
        {
            char tmp[MAX_STRING_LEN];
            strncpy(tmp, list[j - 1], sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';

            strncpy(list[j - 1], list[j], sizeof(list[j - 1]) - 1);
            list[j - 1][sizeof(list[j - 1]) - 1] = '\0';

            strncpy(list[j], tmp, sizeof(list[j]));
            list[j][sizeof(list[j]) - 1] = '\0';

            --j;
        }
    }
}
int arrayMin(char **array, int arraySize)
{
    int c, index = 0;

    for (c = 1; c < arraySize; c++)
        if (strcmp(array[c], array[index]) > 0)
            index = c;

    return index;
}
int arrayMax(char **array, int arraySize)
{
    int c, index = 0;

    for (c = 1; c < arraySize; c++)
        if (strcmp(array[c], array[index]) < 0)
            index = c;

    return index;
}
void EqualizeArrays(char **array1, char **array2, int array1Size, int array2Size)
{

    for (size_t i = 0; i < MAX(array1Size, array2Size); i++)
    {
        char *tmp;
        tmp = strdup(array1[arrayMax(array1, array1Size)]);
        array1[arrayMax(array1, array1Size)] = strdup(array2[arrayMin(array2, array2Size)]);
        array2[arrayMin(array2, array2Size)] = strdup(tmp);
    }
}

int main(int argc, char const *argv[])
{
    //Declaration
    void *res;
    int s, opt, num_master_threads;
    pthread_attr_t attr;
    size_t stack_size;
    //Word w1, w2;
    //Assign
    //w1.value = "test";
    //StsHeader *rawLinesQueue = StsQueue.create();
    StsHeader *unSortedWordsQueue = StsQueue.create();
    StsHeader *treatedLineQueue = StsQueue.create();
    StsHeader *unSortedArraysQueue = StsQueue.create();
    long queueSize = 0;
    //Accept arguments
    char *readPath, *writePath;
    if (argc == 1)
    {
        readPath = DEFAULT_READ_PATH;
        writePath = DEFAULT_WRITE_PATH;
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "-h") == 0)
        {
            ShowHelp(strdup(argv[0]));
        }
        else
        {
            readPath = strdup(argv[1]);
            writePath = DEFAULT_WRITE_PATH;
        }
    }
    else if (argc == 3)
    {
        readPath = strdup(argv[1]);
        writePath = strdup(argv[2]);
    }

    FILE *file = fopen(readPath, "r");
    double ingestTimeSeconds, separationTimeSeconds, sortTimeSeconds, writeTimeSeconds, totalTimeSeconds;
    char str[LINE_LENGTH];
    char treatedLine[LINE_LENGTH];
    struct timeval stop, start;

    //If file handle opened succesfully, read and seperate line into words
    if (file != NULL)
    {
        //Get start timestamp
        gettimeofday(&start, NULL);
        //Ingest lines from file
        while (fgets(str, LINE_LENGTH, file) != NULL)
        {
            //Remove newline specified in EOL
            strtok(str, EOL);

            //Remove punctuation and make lowercase
            reformat_string(str, treatedLine);
            //Push to queue
            StsQueue.push(treatedLineQueue, strdup(treatedLine));
        }
        //Get end timestamp
        gettimeofday(&stop, NULL);
        //Calculate difference between start and stop timestamps
        ingestTimeSeconds = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
        printf("File ingest took %f seconds\n", ingestTimeSeconds);
    }
    else
    {
        fprintf(stderr, "Unable to open file %s\n", readPath);
        exit(1);
    }
    fclose(file);

    //Separate words
    char *line, *word, *ptr, *token;
    gettimeofday(&start, NULL);
    while ((line = StsQueue.pop(treatedLineQueue)) != NULL)
    {
        ptr = line;
        while ((token = strtok_r(ptr, " ", &ptr)))
        {
            StsQueue.push(unSortedWordsQueue, token);
            queueSize++;
        }
    }
    gettimeofday(&stop, NULL);
    separationTimeSeconds = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    printf("Word separation took %f seconds\n", separationTimeSeconds);

    free(treatedLineQueue);

    //Sort words
    gettimeofday(&start, NULL);
    int nbArrays = ceil(queueSize / ARRAY_MAX_SIZE) + 1;
    char **sortedWordsArrays[nbArrays];
    for (int i = 0; i < nbArrays; i++)
    {
        if (queueSize - i * queueSize <= ARRAY_MAX_SIZE)
        {
            sortedWordsArrays[i] = malloc(sizeof(char *) * queueSize);
        }
        else
        {
            sortedWordsArrays[i] = malloc(sizeof(char *) * ARRAY_MAX_SIZE);
        }
        //StsQueue.push(unSortedArraysQueue, sortedWordsArrays[i]);
        /*int arrayLength = sizeof(sortedWordsArrays[i]);
        StsQueue.push(unSortedArraysQueue, arrayLength);*/
    }

    for (long i = 0; i < queueSize; i++)
    {
        word = StsQueue.pop(unSortedWordsQueue);
        //printf("%s\n", word);
        sortedWordsArrays[(i + 1) / ARRAY_MAX_SIZE][i % ARRAY_MAX_SIZE] = strdup(word);
        //printf("%s\n", sortedWordsArrays[(i + 1) / ARRAY_MAX_SIZE][i % ARRAY_MAX_SIZE]);
        /*array = StsQueue.pop(unSortedArraysQueue);
        printf("%s\n", array[i % ARRAY_MAX_SIZE]);*/
    }
    printf("Start equalization\n");

    /*char **array1, **array2;
    int array1Size, array2Size;
    if (nbArrays > 1)
    {
        for (size_t i = 0; i < nbArrays * 2 + 1; i++)
        {
            for (size_t i = 0; i < sizeof(sortedWordsArrays) / sizeof(sortedWordsArrays[0]) - 1; i++)
            {
                array1 = sortedWordsArrays[i];
                array2 = sortedWordsArrays[i + 1];
                array1Size = sizeof(array1) / sizeof(array1[0]);
                array1Size = sizeof(array2) / sizeof(array2[0]);
                EqualizeArrays(array1, array2, array1Size, array2Size);
            }
        }
    }*/
    printf("Start insertion sort\n");

    for (size_t i = 0; i < nbArrays; i++)
    {
        int arraySize = 0;
        while (sortedWordsArrays[i][arraySize] != NULL)
        {
            arraySize++;
        }
        InsertionSort(sortedWordsArrays[i], arraySize - 1);
    }

    /*for (size_t i = 0; i < nbArrays; i++)
    {
        int arraySize = sizeof(sortedWordsArrays) / sizeof(sortedWordsArrays[i]);
        InsertionSort(sortedWordsArrays[i], queueSize);
    }*/
    /*char** array;
    int arrayLength;
    while ((array = StsQueue.pop(unSortedArraysQueue)) != NULL)
    {
        //arrayLength = sizeof(array)/sizeof(array[0]);
        arrayLength = StsQueue.pop(unSortedArraysQueue);
        printf("%i\n", arrayLength);
    }*/

    gettimeofday(&stop, NULL);
    sortTimeSeconds = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    printf("Sorting took %f seconds\n", sortTimeSeconds);

    //Write sorted words to file
    gettimeofday(&start, NULL);
    file = fopen(writePath, "w+");
    for (size_t i = 0; i < nbArrays; i++)
    {
        int arraySize = 0;
        while (sortedWordsArrays[i][arraySize] != NULL)
        {
            arraySize++;
        }

        for (size_t j = 0; j < arraySize - 1; j++)
        {
            fprintf(file, "%s\n", sortedWordsArrays[i][j]);
        }
    }

    fclose(file);

    gettimeofday(&stop, NULL);
    writeTimeSeconds = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    printf("Write took %f seconds\n", writeTimeSeconds);

    free(unSortedWordsQueue);

    //Show total elapsed time
    totalTimeSeconds = ingestTimeSeconds + separationTimeSeconds + sortTimeSeconds + writeTimeSeconds;
    printf("Total time elapsed %f seconds\n", totalTimeSeconds);

    return 0;
}