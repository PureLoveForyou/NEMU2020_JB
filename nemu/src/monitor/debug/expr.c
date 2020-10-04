#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},					// equal
	{"-", '-'},					//minus
	{"\\*", '*'},					//multiply
	{"/", '/'},					//divide
	{"\\(", '('},					//left bracket
	{"\\)", ')'},					//right bracket
	{"[1-9][0-9]*", '1'}				//number
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case '+': tokens[nr_token++].type = rules[i].token_type;break;
					case '-': tokens[nr_token++].type = rules[i].token_type;break;
					case '*': tokens[nr_token++].type = rules[i].token_type;break;
					case '/': tokens[nr_token++].type = rules[i].token_type;break;
					case '(': tokens[nr_token++].type = rules[i].token_type;break;
					case ')': tokens[nr_token++].type = rules[i].token_type;break;
					case EQ: tokens[nr_token++].type = rules[i].token_type;break;
					case NOTYPE: break;
					case '1': tokens[nr_token].type = rules[i].token_type;
						  int j;
						  for( j = 32 - substr_len; j < 32; j++) {
							  tokens[nr_token].str[j] = substr_start[j];
						  }
						  for( j = 0; j < 32 - substr_len; j++) {
							  tokens[nr_token].str[j] = '0';
						  }
						  nr_token++;break;
					default: panic("please implement me");
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

static bool check_parentheses(int p, int q)
{
	/*Check if the expression is surrounded by a matchedd pair of parentheses*/
	bool flag;
	int stack_count = 0;
	if( tokens[p].type != '(' || tokens[q].type != ')' ) {
		flag = false;
	}
	else {
		p++;
		q--;//Make sure the leftmost and the rightmost parentheses are matched

		while(p <= q) {
			if( tokens[p].type == '(' ) {
				stack_count++;
			}
			else if( tokens[p].type == ')' && stack_count > 0) {
				stack_count--;
			}
			else if( tokens[p].type == ')' && stack_count == 0) {
				flag = false;//Right parentheses are more than the left
				break;
			}
			p++;
		}
		if(stack_count == 0) {
			flag = false;
		}
		else {
			flag = true;
		}
	}
	return flag;
}

static int eval(int p, int q) {
	int result;
	if(p > q) {
		printf("Bad expression\n");
		assert(0);
	}
	else if(p == q) {
		/*Single token. And it should be a number*/
		sscanf(tokens[p].str, "%d", &result);
	}
	else if(check_parentheses(p, q) == true) {
		result =  eval(p + 1, q - 1);
	}
	else {
		/*First find out where the dominant operator is*/
		int i, lparenthese_num = 0, rparenthese_num = 0, var1, var2, op = 1;
		int lowpriority = 0;
		for(i = p; i <= q; i++) {
			if(tokens[i].type == '(') {
				lparenthese_num++;
			}
			else if(tokens[i].type == ')') {
				rparenthese_num++;
			}
			else if(tokens[i].type == '+' || tokens[i].type == '-') {
				if(lparenthese_num == rparenthese_num) {
					lowpriority++;
					op = i;
				}
			}
			else if(tokens[i].type == '*' || tokens[i].type == '/') {
				if(lparenthese_num == rparenthese_num && lowpriority != 0) {
					op = i;
				}
			}
		}

		/*Then divide it into two parts to evaluate*/
		var1 = eval(p, op - 1);
		var2 = eval(op + 1, q);
		
		switch(tokens[op].type) {
			case '+': result = var1 + var2;break;
			case '-': result = var1 - var2;break;
			case '*': result = var1*var2;break;
			case '/': result = var1/var2;break;
			default: assert(0);
		}
	}
	return result;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	return eval(0, nr_token - 1);
	panic("please implement me");
	return 0;
}
