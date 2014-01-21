// gcc 4.7.2 +
// gcc -std=gnu99 -Wall -g -o helloworld_c helloworld_c.c -lpthread

#include <pthread.h>
#include <stdio.h>

int i = 0;
pthread_mutex_t mutex;

// Note the return type: void*
void* adder(){
    for(int x = 0; x < 1000000; x++){
	pthread_mutex_lock(&mutex);
        i++;
	pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* decrement(){
    for(int x = 0; x < 1000000; x++){
	pthread_mutex_lock(&mutex);
        i--;
	pthread_mutex_unlock(&mutex);
    }
    return NULL;
}


int main(){
    pthread_t adder_thr,dec_thr;
    
    pthread_mutex_init(&mutex,NULL);
    pthread_create(&adder_thr, NULL, adder, NULL);
    pthread_create(&dec_thr, NULL, decrement, NULL);
    
    for(int x = 0; x < 50; x++){
        printf("%i\n", i);
    }

    
    pthread_join(adder_thr, NULL);
    pthread_join(dec_thr, NULL);
    pthread_mutex_destroy(&mutex);
    
    printf("Done: %i\n", i);
    return 0;
    
}
