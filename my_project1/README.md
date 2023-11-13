<span class="head1">Linux 作業系統</span>
===
# Project1
<div style="text-align: center;"><span class="t1">第19組 彭冠銘 江旭崧 謝友倫</span></div>

- 實做一個 system call ，將 virtual address 轉為 physical address
- 利用這個 system call 將 thread/process 每個 segment 的 memory location 找出來 ，並把 project address space layout 畫出來


## Kernel 與 OS 版本
- Linux Kernel 5.8.1
- Ubuntu 20.04 LTS desktop-amd
- virtual box 7.0
   - 23 cores
   - 12G RAM
   
## Kernel 編譯與systemcall新增過程
我們主要是參考這兩篇教學步驟：
-- https://dev.to/jasper/adding-a-system-call-to-the-linux-kernel-5-8-1-in-ubuntu-20-04-lts-2ga8
-- https://blog.kaibro.tw/2016/11/07/Linux-Kernel%E7%B7%A8%E8%AD%AF-Ubuntu/

先下載Linux Kernel - 5.8.1
```shell=
wget -P ~/ https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.8.1.tar.xz
```
解壓縮
```shell=+
tar -xvf ~/linux-5.8.1.tar.xz -C ~/
```
移動到目錄
```shell
cd ~/linux-5.8.1/
```
創建system call目錄
```shell
mkdir my_systemcall
```
建立system call的C file 
```shell
vi my_systemcall/get_physical_address.c
```
將以下的code編輯到C file
```C=
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE2(get_physical_addresses, unsigned long *, initial,
		unsigned long *, result)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	unsigned long padder = 0;
	unsigned long page_addr = 0;
	unsigned long page_offset = 0;
	//printk("Here1");
	unsigned long *vir_adds =
		kmalloc(1 * sizeof(unsigned long),GFP_KERNEL); //need to use vmalloc in kernel
	unsigned long *phy_adds =
		kmalloc(1 * sizeof(unsigned long), GFP_KERNEL);

	//copy from user
	//initial need to change type?? ->not need
	//printk("Here2");
	unsigned long a = copy_from_user(vir_adds, initial, 1 * sizeof(unsigned long)); //
	printk("%lu", a);

	int i = 0;
	//Input virtual addresses //*(intial+i) //
	

	pgd = pgd_offset(current->mm, *(vir_adds + i));
	printk("pgd_val = 0x%lx\n", pgd_val(*pgd));
	printk("pgd_index = %lu\n", pgd_index(*(vir_adds + i)));
	if (pgd_none(*pgd)) {
		printk("not mapped in pgdn");
	}

	p4d = p4d_offset(pgd, *(vir_adds + i));
	printk("p4d_val = 0x%lx\n", p4d_val(*p4d));
	printk("p4d_index = %lu\n", p4d_index(*(vir_adds + i)));
	if (p4d_none(*p4d)) {
		printk("not mapped in p4d");
	}

	pud = pud_offset(p4d, *(vir_adds + i));
	printk("pud_val = 0x%lx\n", pud_val(*pud));
	printk("pud_index = %lu\n", pud_index(*(vir_adds + i)));
	if (pud_none(*pud)) {
		printk("not mapped in pudn");
	}

	pmd = pmd_offset(pud, *(initial + i));
	printk("pmd_val = 0x%lx\n", pmd_val(*pmd));
	printk("pmd_index = %lu\n", pmd_index(*(initial + i)));
	if (pmd_none(*pmd)) {
		printk("not mapped in pmdn");
	}

	pte = pte_offset_kernel(pmd, *(initial + i));
	printk("pte_val = 0x%lx\n", pte_val(*pte));
	printk("pte_index = %lu\n", pte_index(*(initial + i)));
	if (pte_none(*pte)) {
		printk("not mapped in pten");
	}

	
	page_addr = pte_val(*pte) & PAGE_MASK; 
	page_offset = *(initial + i) & ~PAGE_MASK;

	//physical address
	*(phy_adds + i) = page_addr | page_offset;

	printk("page_addr = %lx\n", page_addr);
	printk("page_offset = %lx\n", page_offset);

	printk("vaddr =%lx, paddr = %lx\n", *(vir_adds + i), *(phy_adds + i));

	//Use copytouser -> user space
	copy_to_user(result, phy_adds, 1 * sizeof(unsigned long));
	kfree(vir_adds);
	kfree(phy_adds);

	return 0;
}
```
建立Make file
```shell
vi my_systemcall/Makefile
```
編輯Make file
```shell
obj-y := get_physical_address.o
```
打開Linux - 5.8.1中的Make file
```shell 
vi Makefile 
```
搜尋==core-y==找到原始路徑
```shell 
kernel/ certs/ mm/ fs/ ipc/ security/ crypto/ block/
```
將路徑改為
```shell
kernel/ certs/ mm/ fs/ ipc/ security/ crypto/ block/ my_systemcall/
```
將system call對應的function prototype加到system call標頭檔
打開system call標頭檔
```shell 
vi include/linux/syscalls.h 
```
在程式碼最後加上
```shell 
asmlinkage long sys_get_physical_address(void);
```
將system call加入Kernel的system call table
```shell
vi arch/x86/entry/syscalls/syscall_64.tbl
```
在此table中新增
```shell
440     common  get_physical_address           sys_get_physical_address
```
編譯kernel
```shell
make -j23
```
準備kernel安裝器
```shell
sudo make modules_install -j23
```
安裝kernel
```shell
sudo make install -j23
```
更新OS的bootloader
```shell
sudo update-grub
```
重新啟動

## User space code

```c=
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>

#define __NR_get_physical_addresses 440

extern void *func1(void *);
extern void *func2(void *);
extern int main();

void * get_physical_addresses(void * virtual_address)
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

int a=123 ;  //global variable
int bss ;   // uninitialized data(bss)

void hello(int pid)
{
	int b=10;   //local varialbe
	int *heap = malloc((sizeof(int)) ;  // heap
	*heap = 100 ;

	b=b+pid;
                                 //global variable
	printf("In thread %d \nthe value of gloable varialbe a is %d, the offset of the logical address of  a is %p, ", pid, a, &a);
	printf("the physical address of global variable a is %p\n", get_physical_addresses(&a) );
                                 //local variable
	printf("the value of local varialbe b is %d, the offset of the logical address of b is %p, ", b, &b);
        printf("the physical address of local variable b is %p\n", get_physical_addresses(&b));
                                 //thread local variable
        printf("the offset of the logical address of thread local varialbe tx is %p, ", &tx);
        printf("the physical address of thread local variable tx is %p\n", get_physical_addresses(&tx));
                                 //function
        printf("the offset of the logical address of function hello is %p, ", hello);
        printf("the physical address of function hello is %p\n", get_physical_addresses(hello));
        printf("the offset of the logical address of function func1 is %p, ", func1);
        printf("the physical address of function func1 is %p\n", get_physical_addresses(func1));
        printf("the offset of the logical address of function func2 is %p, ", func2);
        printf("the physical address of function func2 is %p\n", get_physical_addresses(func2));
        printf("the offset of the logical address of function main is %p, ", main);
        printf("the physical address of function main is %p\n", get_physical_addresses(main));
                                 //library function
        printf("the offset of the logical address of library function printf is %p, ", printf);
        printf("the physical address of library function printf is %p\n", get_physical_addresses(printf));
        printf("====================================================================================================================\n");
}

void delay(unsigned int milliseconds)
{
	clock_t start = clock() ;
	while((clock() - start) * 1000 / CLOCKS_PER_SEC < milliseconds) ;
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
	delay(1000) ;
        strcpy(p[1],"Thread2") ;
        pthread_create(&id[1],NULL,func2,(void *)p[1] );
	delay(1000) ;


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
```

## Result



