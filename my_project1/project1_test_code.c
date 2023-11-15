#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>
#define __NR_get_segments 441
#define __NR_get_physical_addresses 440
#define MAX_BUF_SIZE 128

int bss_value;
int data_value = 123;
int code_function()
{
       return 0;
};

static __thread int thread_local_storage_value = 246;

extern void *func1(void *);
extern void *func2(void *);
extern int main();



struct Segment
{
       void * start_addr;
       void * end_addr;
       char seg_name[MAX_BUF_SIZE];
};


struct ProcessSegments
{
       pid_t pid;
       struct Segment code_seg;
       struct Segment data_seg;
       struct Segment heap_seg;
       struct Segment stack_seg;
};

void * get_phys_addr(void * virtual_address)
{
       //呼叫轉址的system call
       void * physical_address;

       syscall(__NR_get_physical_addresses, &virtual_address, &physical_address);

       return physical_address;
}

struct data_
{
	int  id ;
	char name[16] ;
} ;
typedef struct data_ sdata ;
static __thread  sdata  tx ;  //thread local variable

void hello(int pid)
{
       //void TLS, stack, lib, heap, bss, data, code, _segment_start_addr,  _segment_end_addr, _segment_size_in_bytes;
       int stack_value = 100;
       void * TLS = &thread_local_storage_value;
       void * stack = &stack_value;
       void * lib = getpid;
       void * heap = malloc(sizeof(int));
       void * bss = &bss_value;
       void * data = &data_value;
       void * code = code_function;

       int len = 7;
       // unsigned long vir_addrs[7] = {TLS, stack, lib, heap, bss, data, code};
       unsigned long phy_addrs[7];

       printf("pid = %d\n", pid);
       printf("segment\tvir_addr\tphy_addr\n");
       printf("TLS\t%p\t%p\n", TLS, get_phys_addr(TLS));
       printf("stack\t%p\t%p\n", stack, get_phys_addr(stack));
       printf("lib\t%p\t%p\n", lib, get_phys_addr(lib));
       printf("heap\t%p\t%p\n", heap, get_phys_addr(heap));
       printf("bss\t%p\t%p\n", bss, get_phys_addr(bss));
       printf("data\t%p\t%p\n", data, get_phys_addr(data));
       printf("code\t%p\t%p\n", code, get_phys_addr(code));

       printf("\n=== finding the start, end address and size for this thread segment=== \n");
       struct ProcessSegments thread_segs;
       // tid = syscall(__NR_gettid);(int)gettid())
       thread_segs.pid = pid;
       // get segments //把上面那個get_segments 寫進system call
       syscall(__NR_get_segments, (void *)&thread_segs);
       //code seg
       printf(" segname\tvir_start  -  vir_end\t\t(phy_start  -  phy_end)\t\tsegment size(in words)\n");
       void * _segment_start_addr = get_phys_addr(thread_segs.code_seg.start_addr);
       void * _segment_end_addr = get_phys_addr(thread_segs.code_seg.end_addr);
       unsigned long _segment_size_in_bytes = (unsigned long)_segment_start_addr - (unsigned long)_segment_end_addr;
       printf(" %s:\t%p-%p\t(%p-%p)\t\t\t%lu\n",
              thread_segs.code_seg.seg_name,
              thread_segs.code_seg.start_addr,
              thread_segs.code_seg.end_addr,
              _segment_start_addr, _segment_end_addr,_segment_size_in_bytes);
       //data seg
       _segment_start_addr = get_phys_addr(thread_segs.data_seg.start_addr);
       _segment_end_addr = get_phys_addr(thread_segs.data_seg.end_addr);
       _segment_size_in_bytes = (unsigned long)_segment_start_addr - (unsigned long)_segment_end_addr;
       printf(" %s:\t%p-%p\t(%p-%p)\t%lu\n",
              thread_segs.data_seg.seg_name,
              thread_segs.data_seg.start_addr,
              thread_segs.data_seg.end_addr,
              _segment_start_addr, _segment_end_addr,_segment_size_in_bytes);
       //heap_seg
       _segment_start_addr = get_phys_addr(thread_segs.heap_seg.start_addr);
       _segment_end_addr = get_phys_addr(thread_segs.heap_seg.end_addr);
       _segment_size_in_bytes = (unsigned long)_segment_start_addr - (unsigned long)_segment_end_addr;
       printf(" %s:\t%p-%p\t(%p-%p)\t\t\t%lu\n",
              thread_segs.heap_seg.seg_name,
              thread_segs.heap_seg.start_addr,
              thread_segs.heap_seg.end_addr,
              _segment_start_addr, _segment_end_addr,_segment_size_in_bytes);
       //stack_seg
       _segment_start_addr = get_phys_addr(thread_segs.stack_seg.start_addr);
       _segment_end_addr = get_phys_addr(thread_segs.stack_seg.end_addr);
       _segment_size_in_bytes = (unsigned long)_segment_start_addr - (unsigned long)_segment_end_addr;
       printf(" %s:\t%p-%p\t(%p-%p)\t%lu\n",
              thread_segs.stack_seg.seg_name,
              thread_segs.stack_seg.start_addr,
              thread_segs.stack_seg.end_addr,
               _segment_start_addr, _segment_end_addr,_segment_size_in_bytes);
        printf("====================================================================================================================\n");
}  

void delay(unsigned int milliseconds){

    clock_t start = clock();

    while((clock() - start) * 1000 / CLOCKS_PER_SEC < milliseconds);
}

void *func1(void *arg)
{
	char *p = (char*) arg ;
        int pid  ;
        pid =  syscall( __NR_gettid );
        tx.id = pid ;
        strcpy(tx.name,p) ;
        printf("I am thread with ID %d executing func1().\n",pid);
        hello(pid);
        while(1)
        {
        	//printf("(%d)(%s)\n",tx.id,tx.name) ;
                sleep(1) ;
        }
}

void *func2(void *arg)
{
	char *p = (char*) arg ;
        int pid  ;
        pid =  syscall( __NR_gettid );
        tx.id = pid ;
        strcpy(tx.name,p) ;
        printf("I am thread with ID %d executing func2().\n",pid);
        hello(pid);
        while(1)
        {
        	//printf("(%d)(%s)\n",tx.id,tx.name) ;
                sleep(2) ;
        }
}

int main()
{ 
	pthread_t id[2];
        char p[2][16] ;
        strcpy(p[0],"Thread1") ;
        pthread_create(&id[0],NULL,func1,(void *)p[0]);
	delay(1000);
        strcpy(p[1],"Thread2") ;
        pthread_create(&id[1],NULL,func2,(void *)p[1] );
	delay(1000);


        int pid  ;
        pid =  syscall( __NR_gettid );
        tx.id = pid ;
        strcpy(tx.name,"MAIN") ;
        printf("I am main thread with ID %d.\n", pid);
        hello(pid);
        while(1)
        {
        	//printf("(%d)(%s)\n",tx.id,tx.name) ;
                sleep(5) ;
        }

}
