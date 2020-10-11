#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>


void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmp_w(char *args);

static int cmp_d(char *args);

//static int cmp_bt(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Execute one instruction.Format:si n.Execute n instructions", cmd_si },
	{ "info", "Print the information of registers or watchpoints", cmd_info },
	{ "x", "Scan memory", cmd_x },
	{ "p", "Evaluation", cmd_p },
	{ "w", "Set a watchpoint", cmp_w },
	{ "d", "Delete a watchpoint", cmp_d },
//	{ "bt", "Print stack frame chain", cmp_bt },

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

static int cmd_si(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int num;

	if(arg == NULL) {
		/* no argument given*/
		cpu_exec(1);
	}
	else {
		sscanf(arg,"%d",&num);
		cpu_exec(num);
	}
	return 0;	
}

static int cmd_info(char *args)
{
	char *arg = strtok(NULL, " ");
	if(arg == NULL) {
		printf("Argument required. 'r' for registers and 'w' for watchpoints\n");
		printf("Usage: info r/w\n");
	}
	else if(*arg == 'r'){
		int i;
		for(i = 0; i < 8; i++) {
			printf("%s\t0x%08x\t%d\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
		}
		printf("\n");
		for(i = 0; i < 8; i++) {
			printf("%s\t0x%08x\t%d\n", regsw[i], cpu.gpr[i]._16, cpu.gpr[i]._16);
		}
		printf("\n");
		for(i = 0; i < 4; i++) {
			printf("%s\t0x%08x\t%d\n", regsb[i], cpu.gpr[i]._8[0], cpu.gpr[i]._8[0]);
		}
		printf("\n");
		for(i = 4; i < 8; i++) {
			printf("%s\t0x%08x\t%d\n", regsb[i], cpu.gpr[i%4]._8[1], cpu.gpr[i%4]._8[1]);
		}
		printf("\neip\t0x%08x\t%d\n", cpu.eip, cpu.eip);
	}
	else if(*arg == 'w') {
		info_wp();
	}
	else {
		printf("Unknow command\nUsage: info r/w\n");
	}
	return 0;
}

static int cmd_x(char *args)
{
	char *arg1 = strtok(NULL, " ");
	char *arg2 = strtok(NULL, "");
	int num, i;
	bool success = 1;
	int VirtualAddress,content;
	if(arg1 == NULL || arg2 == NULL) {
		printf( "Arguments required.\n"
			"Usage: x number expression/address\n"
			"Example: x 10 0x100000\n");
	}
	else {
		sscanf(arg1, "%d", &num);
		VirtualAddress = expr(arg2, &success);
		if(success) {
			for(i = 0; i < num; i++) {
				content = swaddr_read(VirtualAddress + i*4, 4);
				printf("0x%08x:\t0x%08x\n", VirtualAddress + i*4, content);
			}
		}
		else
			assert(0);
	}
	return 0;
}

static int cmd_p(char *args)
{
	if(args == NULL) {
		printf( "Expression required.\n"
			"Usage: p ArithmeticExpression.\n");
	}
	else {
		bool success = 1;
		uint32_t result;
		result = expr(args, &success);
		if(success)
			printf("%d\n", result);
		else
			assert(0);
	}
	return 0;
}

static int cmp_w(char *args)
{
	uint32_t value;
	WP *p;
	bool success = 1;
	if(args == NULL) {
		printf("Argument required\nUsage: w expression\n");
	}
	else {
		value = expr(args, &success);
		if(success) {
			p = new_wp();
			p->value = value;
			strcpy(p->str, args);
			printf("Watchpoint %d %s: %u\n", p->NO, p->str, p->value);
		}
		else
			assert(0);
	}
	return 0;
}

static int cmp_d(char *args)
{
	char *arg = strtok(NULL, " ");
	if(arg == NULL) {
		printf("Argument required\nUsage: d N\n");
	}
	else {
		int num;
		sscanf(arg, "%d", &num);
		delete_wp(num);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
