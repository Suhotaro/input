#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/input.h>

#include "input.h"

const char *dir_name = "/dev/input/";
static input_dev_ptr input;

void open_input_devices()
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

void close_input_devices()
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

void register_input_fds_in_epoll(int efd)
{
    input_dev_ptr itr = NULL;
    struct epoll_event event;
    int ret = 0;

    for (itr = input; itr != NULL; itr = itr->next)
    {
        event.data.fd = itr->fd;
        event.events = EPOLLIN;
        
        ret = epoll_ctl(efd, EPOLL_CTL_ADD, itr->fd, &event);
        EXIT_IF_FAIL(ret != -1);
    }   
}

int is_our_event(int fd)
{
    input_dev_ptr itr = NULL;

    for (itr = input; itr != NULL; itr = itr->next)
        if (fd == itr->fd)
            return 1;
    return 0;
}

int main()
{
    struct epoll_event *events = NULL;
    struct input_event ev[3];
    input_dev_ptr itr = NULL;
    int efd, result = 0, size = sizeof(struct input_event)*3;
    int t= 1;

    if (getuid() != 0)
    {
        printf("ERROR: have to be root\n");
        return 1;
    }

    open_input_devices();

    for (itr = input; itr != NULL; itr = itr->next)
    {
        ioctl(itr->fd, EVIOCGNAME(sizeof(itr->name)), itr->name);
        printf("  fd:%d device:%s\n", itr->fd, itr->name);

        result = ioctl(itr->fd, EVIOCGRAB, 1);
        printf("%s\n", (result == 0) ? "SUCCESS\n" : "FAILURE\n");
    }

    efd = epoll_create1(0);
    EXIT_IF_FAIL(efd != -1);

    register_input_fds_in_epoll(efd);

    events = calloc (EPOLLEVENTS, sizeof(struct epoll_event));
    EXIT_IF_PTR_NULL(events);
    

    while(1)
    {
        int n, i, rd;

        n = epoll_wait(efd, events, EPOLLEVENTS, -1);
        for(i = 0; i < n; i++)
        {
            if (!(events[i].events & EPOLLIN))
            {
                printf("epoll error: %d\n", events[i].data.fd);
                close(events[i].data.fd);
                continue;
            }
            else if (is_our_event(events[i].data.fd))
            {

                printf("---------------->>\n");
                int j = 0, num = 0;
                printf("EVENT: fd:%d t:%d\n", events[i].data.fd, t++);


                rd = read(events[i].data.fd, ev, size);

                printf("  read:%d size:%d\n", rd, size);

                if (rd < size)
                    continue;

                num = rd / sizeof(struct input_event);

                printf("  num:%d\n", num);
                printf("<<----------------\n\n");
        

                for (j = 0; j < num; j++)
                    printf ("    %d: Type[%d] Code[%d] Value[%d]\n",  events[i].data.fd, ev[j].type, ev[j].code, ev[j].value);
            }
        }
    }
    

    
    
    


    

    close_input_devices();

    return 0;
}

