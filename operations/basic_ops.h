#ifndef BASIC_OPS_MAC
#define BASIC_OPS_MAC

#include"../../mem_date_index_ctl/mem_transaction.h"

typedef struct fun_param
{
	void *a;
	void *b;
	void *c;
	void *d;
	void *e;
	void *f;
	int ret;
}fun_param;

typedef struct index_fun_param
{
	void *index;
	unsigned  long long key;
	void *value;
	void *result;
	void *a;
	void *b;
	void *c;
	void *d;
	void *e;
	void *f;
	int ret;
	long                      block_no;    //����ҵ��Ŀ����ڿ��
  unsigned  long long       record_num;
}index_fun_param;


mem_rbtree_entry_t * index4;
mem_rbtree_search(mem_rbtree_index_t *mem_rbtree_index,mem_rbtree_entry_t *root,unsigned  long long key,mem_rbtree_entry_t ** result);



inline int mem_hash_index_select_long(
                        /* in */struct mem_hash_index_t  * mem_hash_index ,
                        /* in */unsigned  long long        key,
                        /* in */int (*hash_fun)(unsigned  long long key,/*�����ռ��*/struct  mem_block_t *,/*���ع�ϣ����к�*/unsigned  long long *),
                       /* out */long                     * block_no,    //����ҵ��Ŀ����ڿ��
                       /* out */unsigned  long long      * record_num,  //����ҵ��Ŀ������к�
                       /* out */struct  record_t         ** record_ptr,  /* IF FOUND ,RETURN FOUNDED RECORD  ����ҵ��Ŀ����ڼ�¼��ָ�� */
                       /* out */struct  mem_hash_entry_t ** array_space_entry //�����Ӧ �����ռ�� hash ������
                        );




index_param->ret = mem_rbtree_search(
									(mem_rbtree_index_t *)(index_param->index),
									((mem_rbtree_index_t *)(index_param->index))->root,
									index_param->key,
									&(index_param->result)
									);
									
long block_no   = index_param->result ->block_no;	//���������ڱ�Ŀ�˳���
struct  mem_block_t * mb; 
get_block_no_addr(block_no,&mb	);         
unsigned  long long     record_num = index_param->result ->record_num;//���������ڵ��к�			
get_record_by_record_num(mem_table,mb,record_num,&(index_param->result));
			
									

index_param->ret = mem_hash_index_select_long(
									(mem_rbtree_index_t *)(index_param->index),
									index_param->key,
                  &(index_param->block_no),
                  &(index_param->record_num),
                  &(index_param->result),
                   index_param->a	);

//ȫ��ɨ��
//fun ��������
//param ��������
inline int full_table_scan(
														struct mem_table_t *mem_table,
														void (*fun)(struct record_t *,fun_param* param),
														fun_param* param
) 
{
int __i = 0;											 
struct record_t     * record_ptr;
struct mem_block_t  * __mem_block_temp = mem_table->config.mem_blocks_table;	

	for(;__i<mem_table->config.mem_block_used;++__i)																
	{
			unsigned  long long * __high_level_temp = 0;

				for(;
				__mem_block_temp->space_start_addr + __mem_block_temp->high_level* mem_table->record_size < __mem_block_temp->space_end_addr - mem_table->record_size ;
				++__high_level_temp
				   )		 															
				{
						// �ҵ����õļ�¼λ��
						record_ptr = (struct record_t *) ( (char *)__mem_block_temp->space_start_addr + (__mem_block_temp->high_level) * (mem_table->record_size) );
						//����ÿһ�м�¼
						fun(record_ptr,param);
				}
			__mem_block_temp = __mem_block_temp->next;      //��һ����
	}
	return 0;
}

//�����Ƶ�ȫ��ɨ��
//fun ��������
//param ��������
inline int full_table_scan_limit(
														struct mem_table_t *mem_table,
														void (*fun)(struct record_t *,fun_param* param),
														fun_param* param,
														unsigned  long long n
) 
{
int __i = 0;											 
struct record_t     * record_ptr;
struct mem_block_t  * __mem_block_temp = mem_table->config.mem_blocks_table;	
unsigned  long long __n = 0;

	for(;__i<mem_table->config.mem_block_used;++__i)																
	{
			unsigned  long long * __high_level_temp = 0;

				for(;
				__mem_block_temp->space_start_addr + __mem_block_temp->high_level* mem_table->record_size < __mem_block_temp->space_end_addr - mem_table->record_size ;
				++__high_level_temp
				   )		 															
				{
						// �ҵ����õļ�¼λ��
						record_ptr = (struct record_t *) ( (char *)__mem_block_temp->space_start_addr + (__mem_block_temp->high_level) * (mem_table->record_size) );
						//����ÿһ�м�¼
						fun(record_ptr,param);
						++__n;
						if(__n == n-1)return 0;
				}
			__mem_block_temp = __mem_block_temp->next;      //��һ����
	}
	return 0;
}

index unique scan
index range scan  //  > < <> >= <= between
//����������ϣ�ֻʹ�ò����н��в�ѯ�����²�ѯ������
//�Է�Ψһ�������Ͻ��е��κβ�ѯ
index full scan
index fast full scan

//1������ - - �ϲ����ӣ�Sort Merge Join�� SMJ��
//    2��Ƕ��ѭ����Nested Loops�� NL��
//    3����ϣ���ӣ�Hash Join�� HJ��
//    ���⣬�ѿ����˻���Cartesian Product��




#endif 
