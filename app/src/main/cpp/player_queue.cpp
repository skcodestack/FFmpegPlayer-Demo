//
// Created by PC on 2017/7/13.
//

#include "include/player_queue.h"

#include <android/log.h>
#include <pthread.h>

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"_Queue",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"_Queue",FORMAT,##__VA_ARGS__);
#define LOGW(FORMAT,...) __android_log_print(ANDROID_LOG_WARN,"_Queue",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"_Queue",FORMAT,##__VA_ARGS__);





/**
 * 初始化队列
 */
Queue * queue_init(int size,queue_fill_fun fill_fun){
    Queue* queue = (Queue *) malloc(sizeof(Queue));
    queue->size = size;
    queue->count = 0;
    queue->next_to_read = 0;
    queue->next_to_write = 0;

    queue->tab = (void **) malloc(sizeof(*(queue->tab)) *size);
    int i = 0;
    for (i = 0; i <size; ++i) {
        queue->tab[i] = fill_fun(NULL);
    }

    return  queue;
}

/**
 * 下一个位置
 */
int queue_next(Queue * queue,int current){
    return   (current+1) % (queue->size);
}

/**
 * 添加元素，返回原始数据
 */
void * queue_push(Queue * queue,pthread_mutex_t* pthread_mutex,pthread_cond_t* pthread_cond){

    int current ;

    int next ;


    for (;;){
        current =  queue->next_to_write;
        next = queue_next(queue,current);
        if(next != queue -> next_to_read){
            break;
        }

        pthread_cond_wait(pthread_cond,pthread_mutex);

    }
    queue->next_to_write = next;
    queue->count++;
    void * old =  queue->tab[current];
    pthread_cond_broadcast(pthread_cond);
    return old;
}

/**
 * 弹出下一个元素
 */
void * queue_pop(Queue * queue,pthread_mutex_t* pthread_mutex,pthread_cond_t* pthread_cond){

    int current;
    for (;;){
        current = queue->next_to_read;
        if(queue -> next_to_write != queue -> next_to_read){
            break;
        }

        pthread_cond_wait(pthread_cond,pthread_mutex);

    }

    queue->next_to_read = queue_next(queue,current);
    void * old =  queue->tab[current];
    pthread_cond_broadcast(pthread_cond);
    return old;

}

int get_real_count(Queue * queue){

    if(queue->count > queue->size){
        return queue->size;
    }
    return queue->count;
}

int enable_free_item(Queue * queue){

    if(queue->count > queue->size){
        return 0;
    }
    return -1;
}



/**
 * 销毁队列
 */
void queue_free(Queue * queue,queue_free_fun free_fun){

    if(queue != NULL){
        int i = 0;
        int size = get_real_count(queue);
        for (i = 0; i < size; ++i) {

            LOGE("======>%d======%d",i,size);
            free_fun((void *)queue->tab[i]);
        }
        free(queue->tab);
        free(queue);
    }

}





