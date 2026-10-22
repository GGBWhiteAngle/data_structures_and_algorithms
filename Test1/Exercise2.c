/*B+树代码演示*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//宏定义
#define M 4//M阶树
#define MAX_KEY M//最大关键字数量
#define MAX_CHILD M//最大孩子数量
#define MIN_KEY ((M+1)/2)//关键字允许存在的最小数量
#define MIN_CHILD ((M+1)/2)//孩子结点允许存在的最小数量

//B+树结点内数据关键字的标准形式
typedef struct key
{
	int data;
}key,*nodedata;

//初始化数据关键字
nodedata createdata(int val)
{
	nodedata p = (nodedata)malloc(sizeof(key));
	if (p != NULL)
	{
		p->data = val;
	}
	return p;
}

//创建标准的B+树内结点形式
typedef struct Bpnode
{
	//树内结点通用区域
	union node
	{
		//B+树内的内部索引结点--->路径关键字
		struct internal
		{
			int keys[MAX_KEY+1];
			struct Bpnode* child[MAX_CHILD+1];
		}internal;

		//B+树的叶子结点存储数据--->数据域关键字
		struct leaf
		{
			nodedata keys[MAX_KEY+1];
			struct Bpnode* prev;
			struct Bpnode* next;
		}leaf;
	}node;
	int num_keys;
	int is_leaf;
}Bpnode,*Bptree;

//返回判断是否分裂的结构体
typedef struct Result
{
	Bptree new_node;
	int up_key;
	int has_split;
}Result;

//初始化B+树内的结点,创建一个B+树结点壳子,数据/关键字由外部加入
Bptree createnode(int is_leaf)
{
	Bptree p = (Bptree)malloc(sizeof(Bpnode));
	if (p != NULL)
	{
		//判断当前创建的结点是叶子,还是内部结点
		p->is_leaf = is_leaf;
		p->num_keys = 0;//有效关键字为空
		if (p->is_leaf == 1)
		{
			//置空数据关键字指针
			for (int i = 0;i < MAX_KEY;i++)
			{
				p->node.leaf.keys[i] = NULL;
			}
			p->node.leaf.prev = NULL;//置空前指针
			p->node.leaf.next = NULL;//置空后指针
		}
		else
		{
			//置空孩子指针数组
			for (int i = 0;i < MAX_CHILD;i++)
			{
				p->node.internal.child[i] = NULL;
			}
			//清理索引数组内的随机值关键字
			for (int i = 0;i < MAX_KEY;i++)
			{
				p->node.internal.keys[i] = 0;
			}
		}
	}
	return p;
}

//查找B+树内有无相同的数据
int find(Bptree root, int val)
{
	if (root == NULL)
	{
		return false;
	}
	if (root->is_leaf == 1)
	{
		int i = 0;
		while (i < root->num_keys && val > root->node.leaf.keys[i]->data)
		{
			i++;
		}
		if (i < root->num_keys && val == root->node.leaf.keys[i]->data)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (root->is_leaf == 0)
	{
		int i = 0;
		while (i<root->num_keys && val>=root->node.internal.keys[i])
		{
			i++;
		}
		if (i == 0)
		{
			return false;
		}
		else
		{
			return find(root->node.internal.child[i - 1],val);
		}
	}
}

//往B+树内添加数据
Bptree insertdata(Bptree* root, int val)
{
	if (*root == NULL)
	{
		Bptree p = createnode(1);
		p->node.leaf.keys[0] = createdata(val);
		p->num_keys = 1;
		*root = p;
		return NULL;
	}
	Bptree p = *root;
	//当是叶子时
	if (p->is_leaf == true)
	{
		int i = 0;
		while (i<p->num_keys && val>p->node.leaf.keys[i]->data)
		{
			i++;
		}
		for (int j = p->num_keys-1;j>=i;j--)
		{
			p->node.leaf.keys[j+1] = p->node.leaf.keys[j];
		}
		p->node.leaf.keys[i] = createdata(val);
		p->num_keys++;
		//判断关键字数量是否合法
		//合法直接返回即可
		if (p->num_keys <= MAX_KEY)
		{
			return NULL;
		}
		//不合法返回给分裂后新建的结点
		else if (p->num_keys > MAX_KEY)
		{
			//创建分裂的新结点
			Bptree new_node = createnode(1);
			int mid = p->num_keys / 2;
			int num = p->num_keys;

			//叶子结点链表连接
			Bptree old_next = p->node.leaf.next;//p的旧尾叶子结点
			p->node.leaf.next = new_node;//插入新的结点
			new_node->node.leaf.prev = p;//建立双向
			new_node->node.leaf.next = old_next;//更新尾结点
			if (old_next != NULL)//判断是否为空
			{
				old_next->node.leaf.prev = new_node;
			}

			//搬数据关键字
			for (int j = mid;j < num;j++)
			{
				new_node->node.leaf.keys[j - mid] = p->node.leaf.keys[j];
				p->node.leaf.keys[j] = NULL;
				new_node->num_keys++;
			}
			p->num_keys = p->num_keys - new_node->num_keys;

			//返回新建的结点
			return new_node;
		}
	}
	//不是叶子时继续深入
	else if (p->is_leaf == 0)
	{
		int i = 0;
		while (i<p->num_keys && val>p->node.internal.keys[i])
		{
			i++;
		}
		if (i == 0)
		{
			p->node.internal.keys[0] = val;
			Bptree new_node = insertdata(&(p->node.internal.child[0]), val);
			if (new_node != NULL)
			{
				int up_key;
				if (new_node->is_leaf == 1)
				{
					up_key = new_node->node.leaf.keys[0]->data;
				}
				else
				{
					up_key = new_node->node.internal.keys[0];
				}
				for (int j = p->num_keys-1;j > i;j--)
				{
					p->node.internal.keys[j + 1] = p->node.internal.keys[j];
					p->node.internal.child[j + 1] = p->node.internal.child[j];
				}
				p->node.internal.keys[i + 1] = up_key;
				p->node.internal.child[i + 1] = new_node;
				p->num_keys++;
				if (p->num_keys > MAX_KEY)
				{
					Bptree new_mid = createnode(0);
					int mid = p->num_keys / 2;
					int num = p->num_keys;
					//先搬关键字再搬孩子
					for (int j = mid;j < num;j++)
					{
						new_mid->node.internal.keys[j - mid] = p->node.internal.keys[j];
						new_mid->node.internal.child[j - mid] = p->node.internal.child[j];
						p->node.internal.keys[j] = 0;
						p->node.internal.child[j] = NULL;
						new_mid->num_keys++;
					}
					p->num_keys = p->num_keys - new_mid->num_keys;
					return new_mid;
				}
				return NULL;
			}
			return NULL;
		}
		//i>0且跳出循环时
		else
		{
			Bptree new_node = insertdata(&(p->node.internal.child[i - 1]), val);
			//内部已分裂,此时进行连接
			if (new_node != NULL)
			{
				int up_key;
				if (new_node->is_leaf == 1)
				{
					up_key = new_node->node.leaf.keys[0]->data;
				}
				else
				{
					up_key = new_node->node.internal.keys[0];
				}
				
				//更新当前结点的路径关键字和孩子结点
				for (int j = p->num_keys-1;j >= i;j--)
				{
					p->node.internal.keys[j + 1] = p->node.internal.keys[j];
					p->node.internal.child[j + 1] = p->node.internal.child[j];
				}
				p->node.internal.keys[i] = up_key;
				p->node.internal.child[i] = new_node;
				//更新有效关键字数
				p->num_keys++;

				//判断当前中间结点是否需要分裂
				if (p->num_keys > MAX_KEY)
				{
					Bptree new_mid = createnode(0);
					int mid = p->num_keys / 2;
					int num = p->num_keys;
					//先搬关键字再搬孩子
					for (int j = mid;j < num;j++)
					{
						new_mid->node.internal.keys[j - mid] = p->node.internal.keys[j];
						new_mid->node.internal.child[j - mid] = p->node.internal.child[j];
						p->node.internal.keys[j] = 0;
						p->node.internal.child[j] = NULL;
						new_mid->num_keys++;
					}
					p->num_keys = p->num_keys - new_mid->num_keys;
					return new_mid;
				}
				return NULL;
			}
			//无分裂直接返回
			else
			{
				return NULL;
			}
		}
	}
	//插入失败时
	else
	{
		return NULL;
	}
	
}
//重写插入函数接口
void insertdata_port(Bptree* root, int val)
{
	Bptree p = insertdata(root, val);
	if (p != NULL)//内部产生分裂时
	{
		Bptree left = *root;
		Bptree right = p;
		Bptree new_root = createnode(0);
		int left_up;
		int right_up;
		if (left->is_leaf == 1)
		{
			left_up = left->node.leaf.keys[0]->data;
			right_up = right->node.leaf.keys[0]->data;
		}
		else if(left->is_leaf == 0)
		{
			left_up = left->node.internal.keys[0];
			right_up = right->node.internal.keys[0];
		}
		//选取结点最小关键字作为路径索引
		new_root->node.internal.keys[0] = left_up;
		new_root->node.internal.child[0] = left;
		new_root->num_keys++;
		new_root->node.internal.keys[1] = right_up;
		new_root->node.internal.child[1] = right;
		new_root->num_keys++;
		*root = new_root;
		return;
	}
	return;
}

//B+树删除具体数据关键字函数
Bptree deletedata(Bptree* root, int val,int is_root)
{
	if (*root == NULL)
	{
		printf("\n\t--->树空,无需删除\n");
		return NULL;
	}
	Bptree p = *root;
	//是叶子
	if (p->is_leaf == 1)
	{
		int i = 0;
		while (i<p->num_keys && val>p->node.leaf.keys[i]->data)
		{
			i++;
		}
		//跳出while循环,
		if (i < p->num_keys && val == p->node.leaf.keys[i]->data)
		{
			//找到了要删除的关键字
			//删除val关键字
			nodedata* temp = p->node.leaf.keys[i];
			for (int j = i + 1;j < p->num_keys;j++)
			{
				p->node.leaf.keys[j - 1] = p->node.leaf.keys[j];
			}
			p->node.leaf.keys[p->num_keys - 1] = NULL;
			p->num_keys--;
			free(temp);

			//判断是否既是根又是叶子结点
			if (is_root == 1 && p->num_keys == 0)
			{
				free(p);
				*root = NULL;
				return *root;
			}
			//判断p的有效关键字数,是否下溢,全部返回给上层父节点调整
			//合法
			printf("\n\t叶子结点--->B+树内删除数据: %d,成功\n",val);
			return p;
		}
		//未找到删除的关键字
		printf("\n\t叶子结点--->B+树内无此数据,无需删除\n");
		return p;
	}
	//是中间结点
	else if (p->is_leaf == 0)
	{
		int i = 0;
		while (i<p->num_keys && val>=p->node.internal.keys[i])
		{
			i++;
		}
		int child_idx = (i == 0) ? 0 : i - 1;//真正发生删除的孩子结点下标
		Bptree child_res = NULL;//保存内部进行删除操作的结点
		if (i == 0)
		{
			if (val < p->node.internal.keys[0])
			{
				printf("\n\t中间结点--->B+树内无此数据,无需删除\n");
				return p;
			}
		}
		else
		{
			printf("\n\t进入了内层递归删除--->死循环\n");
			child_res = deletedata(&(p->node.internal.child[child_idx]), val, 0);
		}
		//更新路径
		if (child_res == NULL)return NULL;
		if (child_res->is_leaf == 1)
			p->node.internal.keys[child_idx] = child_res->node.leaf.keys[0]->data;
		else
			p->node.internal.keys[child_idx] = child_res->node.internal.keys[0];

		//借位
		if (child_res->num_keys < MIN_KEY)
		{
			//向左兄弟借
			if (child_idx > 0 && p->node.internal.child[child_idx - 1]->num_keys > MIN_KEY)
			{
				Bptree left = p->node.internal.child[child_idx - 1];
				Bptree right = child_res;
				//把right关键字整体向后搬一个位
				if (right->is_leaf == 1)//是叶子结点
				{
					for (int j = right->num_keys - 1;j >= 0;j--)
					{
						right->node.leaf.keys[j + 1] = right->node.leaf.keys[j];
					}
					right->node.leaf.keys[0] = left->node.leaf.keys[left->num_keys - 1];
					right->num_keys++;
					left->node.leaf.keys[left->num_keys - 1] = NULL;
					left->num_keys--;
					//借完更新路径
					p->node.internal.keys[child_idx] = right->node.leaf.keys[0]->data;
				}
				//是中间结点
				else
				{
					for (int j = right->num_keys - 1;j >= 0;j--)
					{
						right->node.internal.keys[j + 1] = right->node.internal.keys[j];
						right->node.internal.child[j + 1] = right->node.internal.child[j];
					}
					right->node.internal.keys[0] = left->node.internal.keys[left->num_keys - 1];
					right->node.internal.child[0] = left->node.internal.child[left->num_keys - 1];
					right->num_keys++;
					left->node.internal.keys[left->num_keys - 1] = 0;
					left->node.internal.child[left->num_keys - 1] = NULL;
					left->num_keys--;
					//接完更新路径
					p->node.internal.keys[child_idx] = right->node.internal.keys[0];
				}
				return p;
			}
			//向右兄弟借
			else if (child_idx<p->num_keys - 1 && p->node.internal.child[child_idx + 1]->num_keys>MIN_KEY)
			{
				Bptree left = child_res;
				Bptree right = p->node.internal.child[child_idx + 1];
				//叶子结点
				if (left->is_leaf == 1)//是叶子结点
				{
					left->node.leaf.keys[left->num_keys] = right->node.leaf.keys[0];
					left->num_keys++;//借位
					for (int j = 1;j < right->num_keys;j++)
					{
						right->node.leaf.keys[j - 1] = right->node.leaf.keys[j];
					}
					right->num_keys--;
					right->node.leaf.keys[right->num_keys] = NULL;
					//借完更新路径
					p->node.internal.keys[child_idx+1] = right->node.leaf.keys[0]->data;
				}
				//是中间结点
				else
				{
					left->node.internal.keys[left->num_keys] = right->node.internal.keys[0];
					left->node.internal.child[left->num_keys] = right->node.internal.child[0];
					left->num_keys++;//借位
					for (int j = 1;j < right->num_keys;j++)
					{
						right->node.internal.keys[j - 1] = right->node.internal.keys[j];
						right->node.internal.child[j - 1] = right->node.internal.child[j];
					}
					right->num_keys--;
					right->node.internal.child[right->num_keys] = NULL;
					right->node.internal.keys[right->num_keys] = 0;
					//接完更新路径
					p->node.internal.keys[child_idx+1] = right->node.internal.keys[0];
				}
				return p;
			}
			//左右兄弟都不借,进行合并
			else
			{
				//左兄弟存在
				if (child_idx > 0 && p->node.internal.child[child_idx - 1]->num_keys <= MIN_KEY)
				{
					Bptree left = p->node.internal.child[child_idx - 1];
					Bptree right = child_res;
					//叶子结点时
					if (right->is_leaf == 1)
					{
						for (int j = 0;j < right->num_keys;j++)
						{
							left->node.leaf.keys[left->num_keys] = right->node.leaf.keys[j];
							left->num_keys++;
							right->node.leaf.keys[j] = NULL;
						}//合并
						right->num_keys = 0;
						//链表链接
						left->node.leaf.next = right->node.leaf.next;
						if (right->node.leaf.next != NULL)
							right->node.leaf.next->node.leaf.prev = left;
						right->node.leaf.next = NULL;
						right->node.leaf.prev = NULL;

						for (int j = child_idx + 1;j < p->num_keys;j++)//搬当前中间结点
						{
							p->node.internal.keys[j - 1] = p->node.internal.keys[j];
							p->node.internal.child[j - 1] = p->node.internal.child[j];
						}
						p->num_keys--;
						p->node.internal.keys[p->num_keys] = 0;
						p->node.internal.child[p->num_keys] = NULL;
						free(right);
						if (is_root == 1 && p->num_keys == 1)
						{

							p->node.internal.keys[0] = 0;
							p->node.internal.child[0] = NULL;
							free(p);
							*root = left;
							return *root;
						}
						//只要当前结点不是根,直接返回给上一层的父节点中间结点进行调节
						return p;
					}
					//中间结点时
					else if (right->is_leaf == 0)
					{
						for (int j = 0;j < right->num_keys;j++)
						{
							left->node.internal.keys[left->num_keys] = right->node.internal.keys[j];
							left->node.internal.child[left->num_keys] = right->node.internal.child[j];
							left->num_keys++;
							right->node.internal.keys[j] = 0;
							right->node.internal.child[j] = NULL;
						}//中间结点右搬至左
						right->num_keys = 0;

						//搬当前的中间结点p
						for (int j = child_idx+1;j < p->num_keys;j++)
						{
							p->node.internal.keys[j - 1] = p->node.internal.keys[j];
							p->node.internal.child[j - 1] = p->node.internal.child[j];
						}
						p->num_keys--;
						p->node.internal.keys[p->num_keys] = 0;
						p->node.internal.child[p->num_keys] = NULL;
						free(right);
						if (is_root == 1 && p->num_keys == 1)
						{
							p->node.internal.keys[0] = 0;
							p->node.internal.child[0] = NULL;
							free(p);
							*root = left;
							return *root;
						}
						return p;
					}
				}
				//右兄弟存在
				else if (child_idx < p->num_keys - 1 && p->node.internal.child[child_idx + 1]->num_keys <= MIN_KEY)
				{
					Bptree left = child_res;
					Bptree right = p->node.internal.child[child_idx + 1];
					//是叶子结点时
					if (left->is_leaf == 1)
					{
						for (int j = 0;j < right->num_keys;j++)
						{
							left->node.leaf.keys[left->num_keys] = right->node.leaf.keys[j];
							left->num_keys++;
							right->node.leaf.keys[j] = NULL;
						}//合并
						right->num_keys = 0;
						//链表链接
						left->node.leaf.next = right->node.leaf.next;
						if (right->node.leaf.next != NULL)
							right->node.leaf.next->node.leaf.prev = left;
						right->node.leaf.next = NULL;
						right->node.leaf.prev = NULL;

						for (int j = child_idx + 2;j < p->num_keys;j++)//搬当前中间结点
						{
							p->node.internal.keys[j - 1] = p->node.internal.keys[j];
							p->node.internal.child[j - 1] = p->node.internal.child[j];
						}
						p->num_keys--;
						p->node.internal.keys[p->num_keys] = 0;
						p->node.internal.child[p->num_keys] = NULL;
						free(right);
						if (is_root == 1 && p->num_keys == 1)
						{
							p->node.internal.keys[0] = 0;
							p->node.internal.child[0] = NULL;
							free(p);
							*root = left;
							return *root;
						}
						return p;
					}
					//中间结点时
					else if (left->is_leaf == 0)
					{
						for (int j = 0;j < right->num_keys;j++)
						{
							left->node.internal.keys[left->num_keys] = right->node.internal.keys[j];
							left->node.internal.child[left->num_keys] = right->node.internal.child[j];
							right->node.internal.keys[j] = 0;
							right->node.internal.child[j] = NULL;
							left->num_keys++;
						}//中间结点右搬左
						right->num_keys = 0;
						//当前父结点进行覆盖
						for (int j = child_idx + 2;j < p->num_keys;j++)
						{
							p->node.internal.keys[j - 1] = p->node.internal.keys[j];
							p->node.internal.child[j - 1] = p->node.internal.child[j];
						}
						p->num_keys--;
						p->node.internal.keys[p->num_keys] = 0;
						p->node.internal.child[p->num_keys] = NULL;
						free(right);
						if (is_root == 1 && p->num_keys == 1)
						{
							p->node.internal.keys[0] = 0;
							p->node.internal.child[0] = NULL;
							free(p);
							*root = left;
							return *root;
						}
						return p;
					}
				}
			}
		}
		return p;
	}
}

//遍历B+树
void TraverseBptree(Bptree root,int depth)
{
	//判断当前B+树是否为空
	if (root == NULL)
	{
		printf("\n\n\tB+树\t--->|\t为空无需遍历\n");
		return;
	}
	for (int i = 1;i < depth;i++)
	{
		printf("\t");
	}
	//逐步深入知道叶子结点即可,只有叶子结点才存储数据关键字
	if (root->is_leaf == true)
	{
		printf("|--------->数据:{");
		for (int i = 0;i < root->num_keys;i++)
		{
			printf("\t%d", root->node.leaf.keys[i]->data);
		}
		printf("\t}");
		return;
	}
	//中间结点时,逐步深入
	else if (root->is_leaf == false)
	{
		printf("|---------->索引:{");
		for (int j = 0;j < root->num_keys;j++)
		{
			printf("\t%d", root->node.internal.keys[j]);
		}
		printf(" }\n\n");
		for (int i = 0;i < root->num_keys;i++)
		{
			TraverseBptree(root->node.internal.child[i],depth+1);
			printf("\n\n");
		}
	}
}
//在B+树当中范围取值
void LeafList(Bptree root,int front,int rear)
{
	if (root == NULL)
	{
		printf("\n\t此B+树是一颗空树\n");
		return;
	}
	Bptree p = root;
	while (p->is_leaf == 0)
	{
		int i = 0;
		while (i < p->num_keys && front >= p->node.internal.keys[i])
		{
			i++;
		}
		int idx =  (i==0) ? 0 : i-1;
		p = p->node.internal.child[idx];
	}
	int j = 0;
	while (j < p->num_keys && front> p->node.leaf.keys[j]->data)
	{
		j++;
	}
	printf("\n\t------->开始遍历叶子结点链接成的链表<-----\n");
	printf("\nHead");
	printf("--->");
	while (p != NULL)
	{
		for (;j < p->num_keys;j++)
		{
			printf(" %d", p->node.leaf.keys[j]->data);
			if (rear < p->node.leaf.keys[j]->data)
			{
				printf(" ---> ");
				printf("rear\n");
				printf("-----------------------------------------\n");
				return;
			}
		}
		printf(" ---> ");
		p = p->node.leaf.next;
		j = 0;
	}
	printf("NULL\n");
	printf("-----------------------------------------\n");
}
//销毁整棵B+树
void DestoryBptree(Bptree* root)
{
	if (*root == NULL)
	{
		printf("\n\n\tB+树\t--->\t已释放\n");
		return;
	}
	if ((*root)->is_leaf == 1)
	{
		int num = (*root)->num_keys;
		for (int i = 0;i < num;i++)
		{
			free((*root)->node.leaf.keys[i]);
			(*root)->node.leaf.keys[i] = NULL;
			(*root)->num_keys--;
		}
		(*root)->node.leaf.prev = NULL;
		(*root)->node.leaf.next = NULL;
		free((*root));
		*root = NULL;
		//printf("\n叶子结点,销毁成功\n");
		return;
	}
	else if ((*root)->is_leaf == 0)
	{
		for (int i = 0;i < (*root)->num_keys;i++)
		{
			DestoryBptree(&((*root)->node.internal.child[i]));
		}
		free(*root);
		*root = NULL;
		//printf("\n中间结点,销毁成功\n");
		return;
	}
}

//主函数
int main()
{

	Bptree Bp = NULL;
	int val = 8;
	int data[] = {
		15, 8, 22,33,
		4, 11, 18, 25,
		2, 6, 9, 13, 16, 20, 23, 26,
		1, 3, 5, 7, 10, 12, 14, 17, 19, 21, 24, 27,0
	};

	//循环加入
	for (int i = 0;i < 29;i++) 
	{
		if (find(Bp, data[i]) == true)
		{
			printf("\n\t--->数据: %d,已存在,拒绝插入",data[i]);
		}
		else
		{
			insertdata_port(&Bp, data[i]);
		}
	}


	deletedata(&Bp, 10, 1);
	deletedata(&Bp, 14, 1);
	deletedata(&Bp, 13, 1);
	deletedata(&Bp, 19, 1);
	deletedata(&Bp, 22, 1);
	deletedata(&Bp, 24, 1);
	deletedata(&Bp, 25, 1);
	deletedata(&Bp, 26, 1);
	deletedata(&Bp, 23, 1);
	deletedata(&Bp, 33, 1);
	deletedata(&Bp, 8, 1);
	deletedata(&Bp, 12, 1);
	deletedata(&Bp, 9, 1);
	deletedata(&Bp, 0, 1);
	deletedata(&Bp, 3, 1);
	deletedata(&Bp, 1, 1);
	deletedata(&Bp, 5, 1);
	deletedata(&Bp, 11, 1);
	deletedata(&Bp, 27, 1);
	deletedata(&Bp, 4, 1);
	deletedata(&Bp, 7, 1);
	deletedata(&Bp, 17, 1);
	deletedata(&Bp, 21, 1);
	deletedata(&Bp, 20, 1);
	deletedata(&Bp, 18, 1);
	deletedata(&Bp, 15, 1);
	deletedata(&Bp, 16, 1);
	deletedata(&Bp, 6, 1);
	deletedata(&Bp, 2, 1);
	//遍历B+树
	printf("\n\n");
	TraverseBptree(Bp,1);
	printf("\n\n\tB+树\t--->\t遍历成功\n\n");

	//链表范围打印
	LeafList(Bp,0,100);
	//删除B+树内关键字
	//deletedata(&Bp, val,1);

	//销毁B+树
	DestoryBptree(&Bp);

	//测试
	TraverseBptree(Bp, 1);
	printf("\n\t\t\t==========================以上程序全部运行成功=================================\n");
	return 0;
}

