#ifndef MEM_BLOCK_T
#define MEM_BLOCK_T

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>  
#include <pthread.h> //���뻥����
#include<string.h>
#include"../util/log/log_util.h"
#include"mem_block_no_manager.h"

//�ṹ���Ա�ڽṹ���ڵ�ƫ��
#define STRUCT_OFFSET(Type, member) (size_t)&( ((Type*)0)->member) )

//��״̬ ����������, ��æ��δ�ã����ڴ������ݣ�
#define MEM_BLOCK_STATUS_UNUSED 0
#define MEM_BLOCK_STATUS_NORMAL 1
#define MEM_BLOCK_STATUS_BAD    2
#define MEM_BLOCK_STATUS_BUSY   3
#define MEM_BLOCK_STATUS_NO_DATA   4

//��������
#define MEM_BLOCK_ERR_CPY_NULL_PTR        31001
#define MEM_BLOCK_ERR_GET_MEM_NULL_PTR    31002
#define MEM_BLOCK_ERR_FROM_OS             31001
#define MEM_BLOCK_ERR_PUT_MEM_TO_OS       31004
#define MEM_BLOCK_ERR_MMAP_NULL_PTR       31005
#define MEM_BLOCK_ERR_MMAP_FILE_TOO_LARGE 31006
#define MEM_BLOCK_ERR_MUMMAP_NULL_PTR     31007
#define MEM_BLOCK_ERR_OPEN_FILE_FAILED    31008
#define MEM_BLOCK_SIZE_LESS_ZERO					31009
#define MEM_BLOCK_ERR_SPACE_SIZE_ZERO		  31010

//��ʱ�û�������ʾ����
#define MEM_BLOCK_LOCK_T        pthread_mutex_t
#define MEM_BLOCK_LOCK(x)       pthread_mutex_lock(x)
#define MEM_BLOCK_UNLOCK(x)     pthread_mutex_unlock(x)   
#define MEM_BLOCK_LOCK_INIT(x)  pthread_mutex_init(x,NULL)   
#define MEM_BLOCK_LOCK_DEST(x)  pthread_mutex_destroy(x)   


//��ʱ������������ʵ�ֻ���������
#define LIST_LOCK_T             pthread_spinlock_t
#define LIST_LOCK(x)            pthread_spin_lock(x)
#define LIST_UNLOCK(x)          pthread_spin_unlock(x)   
#define LIST_TRYLOCK(x)         pthread_spin_trylock(x)   
#define LIST_LOCK_INIT(x)       pthread_spin_init(x,0)

//��ʱ������������ʵ�ָ�ˮλ����
#define HIGH_LEVEL_LOCK_T             pthread_spinlock_t
#define HIGH_LEVEL_LOCK(x)            pthread_spin_lock(x);
#define HIGH_LEVEL_UNLOCK(x)          pthread_spin_unlock(x);
#define HIGH_LEVEL_TRYLOCK(x)         pthread_spin_trylock(x) 
#define HIGH_LEVEL_LOCK_INIT(x)       pthread_spin_init(x,0)
//
//#define TT_HIGH_LEVEL_LOCK(x)            IMPORTANT_INFO("HIGH_LEVEL_LOCK\n");pthread_spin_lock(x);DEBUG("HIGH_LEVEL_LOCK end\n");
//#define TT_HIGH_LEVEL_UNLOCK(x)          IMPORTANT_INFO("HIGH_LEVEL_UNLOCK\n");pthread_spin_unlock(x);DEBUG("HIGH_LEVEL_UNLOCK end\n");
//#define TT_HIGH_LEVEL_TRYLOCK(x)         IMPORTANT_INFO("HIGH_LEVEL_TRYLOCK\n"),pthread_spin_trylock(x)

long long lock_owner;
#define TT_HIGH_LEVEL_LOCK(x)         IMPORTANT_INFO("HIGH_LEVEL_LOCK\n");ticket_lock(x);DEBUG("HIGH_LEVEL_LOCK end\n");
#define TT_HIGH_LEVEL_UNLOCK(x)       lock_owner--,IMPORTANT_INFO("HIGH_LEVEL_UNLOCK\n");ticket_unlock(x);DEBUG("HIGH_LEVEL_UNLOCK end\n");
#define TT_HIGH_LEVEL_TRYLOCK(x)      ticket_trylock(x)

//#define HIGH_LEVEL_LOCK_T             ticketlock_t
//#define HIGH_LEVEL_LOCK(x)            ticket_lock(x)
//#define HIGH_LEVEL_UNLOCK(x)          ticket_unlock(x)
//#define HIGH_LEVEL_TRYLOCK(x)         ticket_trylock(x)
//#define HIGH_LEVEL_LOCK_INIT(x)       

//��ʱֻ֧�� 20480 ����
#define MAX_BLOCK_NO    20480
//��������
static void * mem_block_no_map[MAX_BLOCK_NO];

struct mem_free_list_t     
{
	LIST_LOCK_T list_lock;
	long long     head;
	long long     tail;
} __attribute__ ((packed, aligned (64)));


//��Ŀ��������ʼ��
//#define INIT_MEM_FREE_LIST(x) \
//(x)->head = 0 ;                \
//LIST_LOCK_INIT(&((x)->list_lock)); 

#define INIT_MEM_FREE_LIST(mb)   \
do{                              \
LIST_LOCK_INIT(  &((mb)->mem_free_list.list_lock)  ); \
(mb)->mem_free_list.head=-1;    \
(mb)->mem_free_list.tail=-1;    \
}while(0);

#define OPEN_MEM_FREE_LIST(mb)   \
do{                              \
LIST_LOCK_INIT(  &((mb)->mem_free_list.list_lock)  ); \
}while(0);

/*
�� �ڴ沼��

=====================

struct  mem_block_t

======================
++++++++++++++++++++++
++++++++++++++++++++++
++++++++++++++++++++++
+++++++data+++++++++++
++++++++++++++++++++++
++++++++++++++++++++++
++++++++++++++++++++++
======================

*/

//�ڴ��������
typedef struct  mem_block_t
{
long                        block_no;                   //���
off_t                       block_size;                 //���С
unsigned  long long         high_level;                 //��ˮλ��    
char                        file_name[256];             //ӳ���ļ���
off_t                       space_size;                 //�������ݿռ��С
struct mem_free_list_t      mem_free_list;              // �����������ڸ����Ѿ�ɾ�����ݼ�¼
char                        splite[2];
void *                      block_start_addr;           //����ʼ��ַ
void *                      block_end_addr;             //�������ַ
void *                      space_start_addr;           //�������ݿռ���ʼ��ַ
void *                      space_end_addr;             //�������ݿռ������ַ
HIGH_LEVEL_LOCK_T           high_level_lock;            //��ˮλ����                
struct mem_block_t *        next;                       //��һ�����ַ
struct mem_block_t *        block_malloc_addr;					//���malloc��ַ�Լ�
int                         status;                     //��״̬ 
key_t                       shmid;                      //�����ڴ�id
int                         fd;                         //ӳ��id
MEM_BLOCK_LOCK_T            block_lock;                 //����
char                        splite2[2];
// redo_log  ��־����δ������ʵ��
} __attribute__ ((packed, aligned (64))) mem_block_t;

#define MEM_BLOCK_HEAD_SIZE sizeof( mem_block_t)
#define SPACE_START_ADDR_OFFSET	(size_t)&( ((mem_block_t*)0)->space_start_addr) 

//allocate_block_no(&((mb)->block_no));    	\
//set_block_no_addr((mb)->block_no,(mb));\
//

//����������ʼ�� //���ٲ����ע����ַ
#define INIT_MEM_BLOCK(mb)  					\
allocate_block_no(&((mb)->block_no));    	\
set_block_no_addr((mb)->block_no,(mb));\
regist_opened_block_name((mb)->file_name,(mb)->block_no);\
(mb)->block_start_addr =  0;					\
(mb)->block_end_addr   =  0;					\
(mb)->space_start_addr =  0;					\
(mb)->space_end_addr   =  0;					\
(mb)->next             =  0;					\
(mb)->status           =  0;					\
(mb)->shmid            =  0;					\
(mb)->space_size       =  0;					\
MEM_BLOCK_LOCK_INIT(&((mb)->block_lock));  \
HIGH_LEVEL_LOCK_INIT(&((mb)->high_level_lock));  \
(mb)->block_malloc_addr=  mb;	                   \
if((mb)->fd<=0 ){ ERROR("MEM_BLOCK_ERR_OPEN_FILE_FAILED\n"); return MEM_BLOCK_ERR_OPEN_FILE_FAILED;}



//������������
//ʹ�ô˺���������һ����������
inline void mem_block_config( struct  mem_block_t * mb ,/*long block_no ,*/ unsigned long block_size , char *file_name ) 
{
//mb->block_no         =  block_no;       
mb->block_size       =  block_size;  
     
strcpy(mb->file_name,file_name);
DEBUG("mb->file_name is %s,mb->block_size is %ld\n",mb->file_name,block_size);
if(!access((mb)->file_name,0))
{
((mb)->fd)  = open((mb)->file_name, O_RDWR, 0666 );
OPEN_MEM_FREE_LIST((mb)); 
}
else
{
((mb)->fd)  = open((mb)->file_name,O_CREAT | O_RDWR, 0666 );
INIT_MEM_FREE_LIST((mb)); 

//allocate_block_no(&((mb)->block_no));    	
//set_block_no_addr((mb)->block_no,(mb));

mb->high_level = 0;
}
	 strcpy(mb->splite,"|\0");
	 strcpy(mb->splite2,"|\0");
if((mb)->fd<=0 ) ERROR("MEM_BLOCK_ERR_OPEN_FILE_FAILED\n");
}


// �鸴��
inline int mem_block_cpy(struct  mem_block_t * to,struct  mem_block_t * from)
{
		if( to->space_size < from->space_size || from->space_size <=0 )return MEM_BLOCK_ERR_CPY_NULL_PTR;
			MEM_BLOCK_LOCK(&(to->block_lock)); 	
			MEM_BLOCK_LOCK(&(from->block_lock)); 	
			
	    memcpy( to->space_start_addr , from->space_start_addr , from->space_size );
	    
	    MEM_BLOCK_UNLOCK(&(from->block_lock)); 	
	    MEM_BLOCK_UNLOCK(&(to->block_lock)); 	
	    
	  return 0;
}

//��Ӳ���ϵͳ���������ڴ�
inline int mem_block_get_mem_from_os(struct  mem_block_t * mb )
{
	   //��os���빲���ڴ�
	  if( NULL == mb   )
	  	{
	  		ERROR("MEM_BLOCK_ERR_GET_MEM_NULL_PTR\n");
	  		return MEM_BLOCK_ERR_GET_MEM_NULL_PTR;
	  	}
	  if(  mb->block_size<=0 )
	  	{
	  		ERROR("MEM_BLOCK_SIZE_LESS_ZERO\n");
	  		return MEM_BLOCK_SIZE_LESS_ZERO;
	    }
    MEM_BLOCK_LOCK(&(mb->block_lock));
		DEBUG("mb is %0x,mb->block_size is: %d\n",mb, mb->block_size);
	  if ((mb->shmid = shmget(IPC_PRIVATE, mb->block_size, 0666)) < 0)  
    {  
        ERROR("MEM_BLOCK_ERR_FROM_OS!");  
        MEM_BLOCK_UNLOCK(&(mb->block_lock));
        return MEM_BLOCK_ERR_FROM_OS;
    }  
    else  
    {  
        DEBUG("Create shared-memory: %d\n", mb->shmid);  
    }  
	  
	  // �� os ��õ��ڴ���ʼ��ַ
	  void * p_addr;
	  p_addr = shmat(mb->shmid,0,0);  	   	 
    DEBUG("block_share_mem_addr is : %0x\n", p_addr);  
    

	  
	  //�����ʼ��ַ
	  mb->block_start_addr =  p_addr;
		DEBUG("mb->block_start_addr is : %0x\n", p_addr);  

	  
	  //��Ľ�����ַ
	  mb->block_end_addr   =  p_addr + mb->block_size;
		DEBUG("mb->block_end_addr is : %0x\n", p_addr + mb->block_size);  
	  //��״̬æ
	  mb->status           =  MEM_BLOCK_STATUS_BUSY;
	 
	  //��״̬Ϊ���ڴ�������
	  mb->status           =  MEM_BLOCK_STATUS_NO_DATA;
	  MEM_BLOCK_UNLOCK(&(mb->block_lock));
	  return 0;
}


//�ѿ��е��ڴ滹������ϵͳ,���ͷ�ָ������
inline int mem_block_put_mem_to_os(struct  mem_block_t * mb)
{
		DEBUG("Enter mem_block_put_mem_to_os() \n");	

		MEM_BLOCK_LOCK(&(mb->block_lock));
	   //��״̬æ
	  mb->status   =  MEM_BLOCK_STATUS_BUSY;
    //��������ڴ�
    if ( -1 == shmctl(mb->shmid, IPC_RMID, NULL) )
    {  
         ERROR("%s\n","rm bad");  
         MEM_BLOCK_UNLOCK(&(mb->block_lock));
        return MEM_BLOCK_ERR_PUT_MEM_TO_OS;  
     }  
     	DEBUG("mb->shmid = %d have deleted!\n",mb->shmid);	
     //��״̬δ��
	  mb->status   =  MEM_BLOCK_STATUS_UNUSED;
	  
	  //���ٲ����ɾ�����ַ
	  del_block_no_addr(mb->block_no);
	  unregist_block_name(mb->file_name);
	  DEBUG("del_block_no_addr ok!\n");	
	  MEM_BLOCK_UNLOCK(&(mb->block_lock));
	  DEBUG("mem_block_put_mem_to_os() ok!\n");	
		
		MEM_BLOCK_LOCK_DEST(&(mb->block_lock));
    return 0;
}


// ͨ��ʹ�� mmap ϵͳ���ý����ݼ��ص��ڴ��
inline int mem_block_mmap(struct  mem_block_t * mb)
{
	if( NULL == mb || mb->fd <= 0 )
		{
			ERROR("MEM_BLOCK_ERR_MMAP_NULL_PTR\n");
			return MEM_BLOCK_ERR_MMAP_NULL_PTR;
		}
	int fd;
	struct stat sb; 
	/* ȡ���ļ���С */
	fstat(mb->fd, &sb); 

	//if(sb.st_size > mb->block_size-MEM_BLOCK_HEAD_SIZE  )return MEM_BLOCK_ERR_MMAP_FILE_TOO_LARGE;
	// ����ļ������ڣ��򴴽��ļ�����ֹcoredump
	int is_exist =1;
	if(0 == sb.st_size){
		lseek(mb->fd,mb->block_size-1,SEEK_END);  
    write(mb->fd,"0",1);
    fsync(mb->fd);
    fstat(mb->fd, &sb);
    is_exist = 0;
  }  
		MEM_BLOCK_LOCK(&(mb->block_lock));
	  
   	DEBUG("p_addr = %0x,sb.st_size = %d\n",mb->block_start_addr,sb.st_size);  
	  //���������ʼ��ַ
	  mb->space_start_addr  =  mmap((mb->block_start_addr),sb.st_size , PROT_READ|PROT_WRITE , MAP_SHARED, mb->fd, 0);
		DEBUG("space_start oraginal addr is : %0x\n", mb->space_start_addr);  
		
		//������Ϣ���Ƶ�����ڴ濪ͷ��λ��
	 // memcpy(  mb,mb->space_start_addr , MEM_BLOCK_HEAD_SIZE );
	 // mb->high_level = ((mem_block_t *)mb->space_start_addr)->high_level;
	  
	  //����ļ������ڣ������ƿ���Ϣ���Ƶ�����ڴ濪ͷ��λ��
	  //���򣬽���������Ϣ��д�ؿ��ƿ���
	  if( 0 == is_exist)
	  {
	  memcpy(  mb->space_start_addr,mb , MEM_BLOCK_HEAD_SIZE );
	  //((mem_block_t *)(mb->space_start_addr))->block_malloc_addr = mb->block_malloc_addr;
	  DEBUG("after copy,the mmap block_malloc_addr is %0x\n",((mem_block_t *)(mb->space_start_addr))->block_malloc_addr);
	  //ˮλ�߳�ʼ��СΪ�����ݽ�����ַ
	  mb->high_level       =  0 ;
	  }
	  else
	  {
	  mb->high_level    = ((mem_block_t *)(mb->space_start_addr))->high_level;
	  mb->mem_free_list.head = ((mem_block_t *)(mb->space_start_addr))->mem_free_list.head;
	  
//	  mb->block_no    =  ((mem_block_t *)(mb->space_start_addr))->block_no; 
//    set_block_no_addr(mb->block_no,mb);
	  
	  DEBUG("mb->high_level addr is %0x,mb->high_level is : %d \n", &(mb->high_level),mb->high_level); 
	  }
		//������ݽ�����ַ
	  mb->space_end_addr    =  mb->space_start_addr + mb->block_size;
	  //�����ݵĴ�С = ���С - ��ͷ��С
	  mb->space_size       =   mb->block_size - MEM_BLOCK_HEAD_SIZE;
	  	
	  
	  //������ͷ����mb->space_start_addr �ƶ������ݲ���
		mb->space_start_addr +=  MEM_BLOCK_HEAD_SIZE;
	  DEBUG("space_start_addr is : %0x,space_size IS %d\n", mb->space_start_addr,mb->space_size);  
	   
	  MEM_BLOCK_UNLOCK(&(mb->block_lock));
	  
	  if(0 ==  mb->space_size )return MEM_BLOCK_ERR_SPACE_SIZE_ZERO;
	  
	  DEBUG("mem_block_mmap() end\n");  

	  return 0;
}

// ͨ��ʹ�� munmap ϵͳ���ý������ͻظ��ļ�
inline int mem_block_munmap(struct  mem_block_t * mb)
{
		if( NULL == mb || mb->fd <= 0 )return MEM_BLOCK_ERR_MUMMAP_NULL_PTR;
		DEBUG("Begin to mem_block_munmap() \n");  

		 MEM_BLOCK_LOCK(&(mb->block_lock));


	  
	   mb->space_start_addr -=  MEM_BLOCK_HEAD_SIZE;
	   //������Ϣ���Ƶ�����ڴ濪ͷ��λ��
	   memcpy( mb->space_start_addr , mb , MEM_BLOCK_HEAD_SIZE );
	    /* ���ӳ�� */
     munmap( mb->space_start_addr , mb->space_size );
     //�ر��ļ����
     close(mb->fd);
     MEM_BLOCK_UNLOCK(&(mb->block_lock));
     DEBUG("mem_block_munmap() ok!\n");  
  	 return 0;
}

// ͨ��ʹ�� mem_block_msync ϵͳ���ý������ͻظ��ļ�
inline int mem_block_msync(struct  mem_block_t * mb)
{
		if( NULL == mb || mb->fd <= 0 )return MEM_BLOCK_ERR_MUMMAP_NULL_PTR;
	   /* ���ӳ�� */
	   MEM_BLOCK_LOCK(&(mb->block_lock));
	   //������Ϣ���Ƶ�����ڴ濪ͷ��λ��
	   memcpy( mb->block_start_addr , mb , sizeof(struct  mem_block_t) );
     msync( mb->block_start_addr , mb->block_size,MS_SYNC );
     MEM_BLOCK_UNLOCK(&(mb->block_lock));
  	 return 0;
}

#endif 