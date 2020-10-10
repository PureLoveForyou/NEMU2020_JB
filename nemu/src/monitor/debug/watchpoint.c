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
	WP *p, *q;
	q = head;
	while(q->next != wp) {
		q = q->next;	
	}
	q->next = wp->next;
	
	if(free_ == NULL)
		free_ = wp;
	else {
		p = free_->next;
		free_ = wp;
		free_->next = p;
	}
}

