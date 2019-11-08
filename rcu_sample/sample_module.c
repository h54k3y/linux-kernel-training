#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#define READ_THR 2
#define READ_NUM 100
#define MAX_CNT 100

struct student
{
	int id;
	int grade;
	struct list_head node;
	struct rcu_head rcu;
};

LIST_HEAD(students);
DEFINE_SPINLOCK(students_mutex);

static struct task_struct *tsk_add;
static struct task_struct *tsk_update;
static struct task_struct *tsk_delete;
static struct task_struct *tsk_read[READ_THR];

static void add_student(int id, int grade)
{
	struct student *ptr;
	struct student *sp = NULL;
	rcu_read_lock();
	list_for_each_entry_rcu(ptr, &students, node)
	{
		if (ptr->id == id)
		{
			rcu_read_unlock();
                        printk("<RSU_SAMPLE> Add Error: already exisit [id = %d]\n", id);
			return;
		}
	}
	rcu_read_unlock();
	sp = kmalloc(sizeof(*sp), GFP_KERNEL);
	sp->id = id;
	sp->grade = grade;
	spin_lock(&students_mutex);
	list_add_rcu(&sp->node, &students);
	spin_unlock(&students_mutex);
        synchronize_rcu();
	printk("<RSU_SAMPLE> Added a student [id = %d]", id);
}

static void update_grade_info(int id, int grade)
{
	struct student *sp = NULL;
	struct student *new_sp = NULL;
	struct student *old_sp = NULL;
	bool flg_found = false;
	rcu_read_lock();
	list_for_each_entry(sp, &students, node)
	{
		if (sp->id == id)
		{
			flg_found = true;
			new_sp = kmalloc(sizeof(*new_sp), GFP_KERNEL);
			spin_lock(&students_mutex);
			old_sp = rcu_dereference_protected(sp, lockdep_is_held(&students_mutex));
			*new_sp = *old_sp;
			new_sp->grade = grade;
			rcu_assign_pointer(sp, new_sp);
			spin_unlock(&students_mutex);
			synchronize_rcu();
			kfree(old_sp);
			printk("<RSU_SAMPLE> Updated [id = %d] grade to %d\n", id, grade);
			break;
		}
	}
	if (!flg_found)
	{
		printk("<RSU_SAMPLE> Update Error: Unavailable id");
	}
	rcu_read_unlock();
}

static void delete_student(int id)
{
	struct student *sp;
	bool flg_found = false;
	rcu_read_lock();
	list_for_each_entry(sp, &students, node)
	{
		if (sp->id == id)
		{
			flg_found = true;
			spin_lock(&students_mutex);
			list_del_rcu(&sp->node);
			spin_unlock(&students_mutex);
			synchronize_rcu();
			kfree(sp);
			printk("<RSU_SAMPLE> Deleted a student [id = %d]\n", id);
			break;
		}
	}
	if (!flg_found)
	{
		printk("<RSU_SAMPLE> Delete Error: Unavailable id\n");
	}
	rcu_read_unlock();
}

static void print_student_info(void)
{
	struct student *sp;
	rcu_read_lock();
	printk("<RSU_SAMPLE> PRINT START\n");
	list_for_each_entry_rcu(sp, &students, node)
	{
		printk("<RSU_SAMPLE> Printing [id: %d, grade: %d]", sp->id, sp->grade);
	}
	printk("<RSU_SAMPLE> PRINT END\n");
	rcu_read_unlock();
}

static int test_read(void *data)
{
	printk("<RSU_SAMPLE> test_read_start\n");
	msleep(5);
	while (!kthread_should_stop())
	{
		print_student_info();
		msleep(1);
	}
	printk("<RSU_SAMPLE> test_read_end\n");
	return 0;
}

static int test_add(void *data)
{
	int i = 0;
	printk("<RSU_SAMPLE> test_add_start\n");
	while ((i < MAX_CNT) && !kthread_should_stop())
	{
		add_student(i, i);
		i += 1;
		msleep(3);
	}
	printk("<RSU_SAMPLE> test_add_end\n");
	return 0;
}

static int test_update(void *data)
{
	int i = 0;
	printk("<RSU_SAMPLE> test_update_start\n");
	while ((i < MAX_CNT/10) && !kthread_should_stop())
	{
		msleep(10);
		update_grade_info(i, i * -1);
		i += 1;
	}
	printk("<RSU_SAMPLE> test_update_end\n");
	return 0;
}

static int test_delete(void *data)
{
	int i = 0;
	printk("<RSU_SAMPLE> test_delete_start\n");
	while ((i < MAX_CNT/10) && !kthread_should_stop())
	{
		msleep(20);
		delete_student(i);
		i += 1;
	}
	printk("<RSU_SAMPLE> test_delete_end\n");
	return 0;
}

static int sample_module_init(void)
{
	int i = 0;
	tsk_add = kthread_run(test_add, NULL, "test_add");
	tsk_update = kthread_run(test_update, NULL, "test_update");
	tsk_delete = kthread_run(test_delete, NULL, "test_delete");
	for (; i < READ_THR; ++i)
	{
		tsk_read[i] = kthread_run(test_read, NULL, "test_read");
	}
	return 0;
}

static void sample_module_exit(void)
{
	int i = 0;
	kthread_stop(tsk_add);
	kthread_stop(tsk_update);
	kthread_stop(tsk_delete);
	for (; i < READ_THR; ++i)
	{
		kthread_stop(tsk_read[i]);
	}
}

module_init(sample_module_init);
module_exit(sample_module_exit);
MODULE_LICENSE("GPL");
