//
// Created by PC on 2017/7/13.
//

#ifndef FFMPEGPLAYER_DEMO_QUEUE_H
#define FFMPEGPLAYER_DEMO_QUEUE_H

#include <malloc.h>
#include <pthread.h>
typedef  struct _Queue  Queue;
struct  _Queue{
    //最大容量
    int size;
    int count;
    //元素
    void** tab;
    //
    int next_to_write;
    int next_to_read;


};
//释放队列中元素
typedef void* (*queue_fill_fun)(void *) ;
//释放队列中元素
typedef void* (*queue_free_fun)(void *) ;
/**
 * 初始化队列
 */
Queue * queue_init(int size,queue_fill_fun fill_fun);
/**
 * 添加元素，返回原始数据
 */
void * queue_push(Queue * queue,pthread_mutex_t* pthread_mutex,pthread_cond_t* pthread_cond);

/**
 * 弹出下一个元素
 */
void * queue_pop(Queue * queue,pthread_mutex_t* pthread_mutex,pthread_cond_t* pthread_cond);

/**
 * 销毁队列
 */
void queue_free(Queue * queue,queue_free_fun free_fun);
int enable_free_item(Queue * queue);
#endif //FFMPEGPLAYER_DEMO_QUEUE_H
