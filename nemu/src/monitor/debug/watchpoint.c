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
	free_->next = NULL;

	/*Add a node to head*/
	q = head;
	if(q != NULL) {
		while(q->next != NULL) {
			q = q->next;
		}
		q->next = p;
	}
	else
		head = p;
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
	else {
		while(p->next != NULL && p->next != wp) {
			p = p->next;
		}
		if(p->next == wp)
			p->next = p->next->next;
		else
			assert(0);
	}

	if(free_ == NULL) {
		free_ = wp;
		free_->next = NULL;
	}
	else {
		p = free_->next;
		free_ = wp;
		free_->next = p;
	}
	free_->NO = 0;
	free_->var = 0;
	int i;
	for(i = 0; i < 32; i++)
		free_->str[i] = '\0';
}

