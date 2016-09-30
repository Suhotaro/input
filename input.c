#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/epoll.h>

#include "input.h"

const char *dir_name = "/dev/input/";
static input_dev_ptr input;

void init()
{
    input_dev_ptr itr = NULL, dev = NULL;
    struct dirent *file = NULL;
    DIR *dir = NULL;
    int fd = 0;

    dir = opendir(dir_name);
    EXIT_IF_PTR_NULL(dir);

    file = readdir(dir);
    EXIT_IF_PTR_NULL(dir);

    while (file != NULL)
    {
        /* printf("file in directory %s: %s\n", dir_name, file->d_name); */

        /* Do not consider mice events */
        if (file->d_name[0] == 'e')
        {
            char str[20];
            memset(str, 0, sizeof(str));

            strncpy(str, dir_name, 11 * sizeof(char));
            strncpy(&str[11], file->d_name, 7*sizeof(char));         

            printf("open file: %s\n", str);

            fd = open(str, O_RDWR);
            EXIT_IF_FAIL_MSG(fd > 0, "fail open file");

            if (input == NULL)
            {
                input = calloc(1, sizeof(input_dev));
                EXIT_IF_PTR_NULL(input);

                input->fd = fd;
                input->next = NULL;
            }
            else
            {
                itr = input;

                /* It is weird */
                while(itr->next)
                    itr = itr->next;

                dev = calloc(1, sizeof(input_dev));
                EXIT_IF_PTR_NULL(dev);

                dev->fd = fd;
                dev->next = NULL;

                itr->next = dev;
            }
        }

        file = readdir(dir);
    }

    closedir(dir);
}

void deinit()
{
    input_dev_ptr itr = NULL, tmp = NULL, dev = NULL;

    if (input->next == NULL)
        free(input);

    itr = input;
    dev = itr->next;

    while(itr != NULL)
    {
        printf("free: %d\n", itr->fd);

        free(itr);

        if (dev == NULL)
            return;

        tmp = dev;
        dev = dev->next;
        itr = tmp;
    } 
}

int main()
{
    init();

    deinit();

    return 0;
}

