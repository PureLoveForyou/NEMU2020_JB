#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp()
{
	WP *p, *q;
	if(free_ == NULL) {
		printf("No watchpoint avalible\n");
		assert(0);
	}

	/*Delete a node from free*/
	p = free_;
	free_ = free_->next;

	/*Add a node to tail*/
	q = head;
	if(q == NULL) {
		head = p;
		p->next = NULL;
	}
	else {
		while(q->next != NULL) {
			q = q->next;
		}
		q->next = p;
		p->next = NULL;
	}

	return p;
}

void free_wp(WP *wp)
{
	WP *p;
	p = head;
	if(p == NULL) {
		printf("No watchpoints\n");
		assert(0);
	}
	/*Delete a node*/
	else if(p->NO == wp->NO) {
		head = p->next;
	}
	else {
		while(p->next != NULL && p->next->NO != wp->NO) {
			p = p->next;
		}
		if(p->next->NO == wp->NO)
			p->next = p->next->next;
		else
			assert(0);
	}
	/*Add a node*/
	if(free_ == NULL) {
		free_ = wp;
		free_->next = NULL;
	}
	else {
		p = free_;
		while(p->next != NULL) {
                        p = p->next;
                }
                p->next = wp;
                wp->next = NULL;
	}
	/*Delete the some data in this node*/
	int i;
	for(i = 0; i < 32; i++)
		free_->str[i] = '\0';
}

bool check_wp()
{
	WP *p;
	uint32_t new_value;
	bool success = true, flag = true;
	p = head;
	while(p != NULL) {
		new_value = expr(p->str, &success);
		if(!success)
			assert(0);
		if(new_value != p->value) {
			flag = false;
			printf("watchpoint %d: %s changed\n", p->NO, p->str);
			printf("Old value: %u\nNew value: %u\n", p->value, new_value);
			p->value = new_value;
		}
		p = p->next;
	}
	return flag;
}

void info_wp()
{
	WP *p;
	bool success = true;
	p = head;
	if(p == NULL) {
		printf("NO watchpoint\n");
	}
	else {
		printf("Number\tType\t\tWhat\t\tValue\n");
		while(p != NULL) {
			if(success)
				printf("   %d\tWatchpoint\t%s\t\t%-u\n", p->NO, p->str, p->value);
			p = p->next;
		}
	}
}

void delete_wp(int n)
{
	WP *p = head;
	if(p == NULL)
		printf("No watchpoint\n");
	else {
		while(p != NULL && p->NO != n) {
			p = p->next;
		}
		if(p == NULL) {
			printf("Watchpoint %u doesn't exist\n", n);
		}
		else if(p->NO == n) {
			free_wp(p);
		}
	}
}
