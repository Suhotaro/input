
#ifndef _INPUT_H_
#define _INPUT_H_

#define EXIT_IF_FAIL(expr) \
if (!(expr)) \
{ \
    printf("ERROR: [%s:%d]" #expr " \n", __func__, __LINE__); \
    exit(1); \
}

#define EXIT_IF_FAIL_MSG(expr, fmt, ARG...) \
if (!(expr)) \
{ \
    printf("ERROR: [%s:%d] " #expr " " fmt "\n", __func__, __LINE__, ##ARG); \
    exit(1); \
}

#define EXIT_IF_PTR_NULL(ptr) \
if (ptr == NULL) \
{ \
    printf("ERROR: [%s:%d]" #ptr "\n", __func__, __LINE__); \
    exit(1); \
}

#define EPOLLEVENTS 5
typedef struct input_device  input_dev, *input_dev_ptr;

struct input_device
{
    int fd;
    char name[256];
    
    input_dev_ptr next;
};


#endif //_INPUT_H_
