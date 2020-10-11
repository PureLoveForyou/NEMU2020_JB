#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ, NUM, NEGATIVE, HEXNUM, 
	DEREFERENCE, NOTEQ, AND, OR, NOT, DOLREG


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
	{"-", '-'},					//minus
	{"\\*", '*'},					//multiply
	{"/", '/'},					//divide

	{"\\(", '('},					//left bracket
	{"\\)", ')'},					//right bracket

	{"[0][x|X][0-9a-fA-F][0-9a-fA-F]*", HEXNUM},	//hexadecimal number

	{"\\$[a-z][a-z]*", DOLREG},			//Access registers

	{"==", EQ},					//equal
	{"!=", NOTEQ},					//not equal

	{"&&", AND},					// and
	{"\\|\\|", OR},					// or
	{"!", NOT},					//NOT

	{"[0-9][0-9]*", NUM}				//number
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
					case '+': 	tokens[nr_token++].type = rules[i].token_type;break;
					case '-': 	tokens[nr_token++].type = rules[i].token_type;break;
					case '*': 	tokens[nr_token++].type = rules[i].token_type;break;
					case '/': 	tokens[nr_token++].type = rules[i].token_type;break;
					case '(': 	tokens[nr_token++].type = rules[i].token_type;break;
					case ')': 	tokens[nr_token++].type = rules[i].token_type;break;
					
					case EQ: 	tokens[nr_token++].type = rules[i].token_type;break;
					case NOTEQ: 	tokens[nr_token++].type = rules[i].token_type;break;
					case AND: 	tokens[nr_token++].type = rules[i].token_type;break;
					case OR: 	tokens[nr_token++].type = rules[i].token_type;break;
					case NOT:       tokens[nr_token++].type = rules[i].token_type;break;
					case NOTYPE: 	break;
					case DOLREG:	tokens[nr_token].type = rules[i].token_type;
							int y;
							for(y = 0; y < 32; y++) {
								/*Initialization and record the name of the register*/
								tokens[nr_token].str[y] = '\0';
								if(y <= substr_len -2)
									tokens[nr_token].str[y] = substr_start[y+1];
							}
							for(y = 0; y < 8; y++) {
								/*Find out which register it is and record it's position*/
								if(strcmp(tokens[nr_token].str, regsl[y])==0) {
									tokens[nr_token].str[4] = y + '0';break;
								}
								else if(strcmp(tokens[nr_token].str, regsw[y])==0) {
									tokens[nr_token].str[5] = y + '0';break;
								}
								else if(strcmp(tokens[nr_token].str, regsb[y])==0) {
                                                                        tokens[nr_token].str[6] = y + '0';break;
                                                                }
								else if(strcmp(tokens[nr_token].str, "eip") ==0) {
                                                                        tokens[nr_token].str[7] = '1';break;
                                                                }
							}
							nr_token++;break;
					case HEXNUM: 	tokens[nr_token].type = rules[i].token_type; 
							int k, l;
							for(k = 0; k < 32; k++)
								tokens[nr_token].str[k] = '0';
							for(k = 32 - substr_len + 2, l = 2; k < 32; k++, l++)
								tokens[nr_token].str[k] = substr_start[l];
							nr_token++;break;

					case NUM: 	tokens[nr_token].type = rules[i].token_type;
						  	int j, i;
						  	for( j = 0; j < 32; j++) {
							  	tokens[nr_token].str[j] = '0';
						  	}
						  	for( j = 32 - substr_len, i = 0; j < 32; j++, i++) {
							  	tokens[nr_token].str[j] = substr_start[i];
						  	}
						  	nr_token++;break;

					default: 	panic("please implement me");
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
	if( tokens[p].type != '(' || tokens[q].type != ')' )
		flag = false;
	else {
		p++;
		q--;//Make sure the leftmost and the rightmost parentheses are matched
		while(p <= q) {
			if( tokens[p].type == '(' ) 
				stack_count++;
			else if( tokens[p].type == ')' && stack_count > 0)
				stack_count--;
			else if( tokens[p].type == ')' && stack_count == 0)
				return false;//Right parentheses are more than the left
			p++;
		}
		if(stack_count == 0)
			flag = true;
		else
			flag = false;
	}
	return flag;
}

static uint32_t eval(int p, int q) {
	uint32_t result;

	if(p > q) {
		/*bad expression*/
		printf("Illegal expression\n");
		assert(0);
	}
	else if(p == q) {
		/*Single token. And it should be a number*/
		int i;
		if(tokens[p].type == NUM) {
			result = 0;
			for(i = 0; i < 32; i++) {
				result = result*10 + (uint32_t)(tokens[p].str[i] - '0');
			}
		}
		else if(tokens[p].type == DOLREG) {
			/*Access registers*/
			int index = 0;
			for(i = 4; i < 8; i++) {
				if(tokens[p].str[i] >= '0'&&tokens[p].str[i] < '8') {
					index = tokens[p].str[i] - '0';
					break;
				}
			}
			index = tokens[p].str[i] - '0';
			switch(i) {
				case 4:	return reg_l(index);
				case 5: return reg_w(index);
				case 6: return reg_b(index);
				case 7: return cpu.eip;
				default:printf("Register doesn't exist\n");assert(0);
			}
		}	
		else {
			/*The number is a hexadecimal number*/
			result = 0;
			for(i = 0; i < 32; i++) {
				/*Transform hexadecimal number into a decaimal number*/
				if(tokens[p].str[i] >= '0'&&tokens[p].str[i] <= '9')
					result = result*16 + (uint32_t)(tokens[p].str[i] - '0');
				else if(tokens[p].str[i] >= 'a'&&tokens[p].str[i] <= 'f')
					result = result*16 + (uint32_t)(tokens[p].str[i] - 'a') + 10;
				else if(tokens[p].str[i] >= 'A'&&tokens[p].str[i] <= 'F')
					result = result*16 + (uint32_t)(tokens[p].str[i] - 'A') + 10;
				else {
					printf("Illegal hexadecimal number\n");
					assert(0);
				}
			}
		}
	}
	else if(check_parentheses(p, q) == true) {
		/*The expression is "(expression)". Throw '(' and ')' away*/
		result =  eval(p + 1, q - 1);
	}
	else {
		/*First find out where the dominant operator is*/
		int i, lparenthese_num = 0, rparenthese_num = 0, var1, var2, op = 0;
		int priority1 = 0, priority2 = 0, priority3 = 0, priority4 = 0;
		for(i = p; i <= q; i++) {
			if(tokens[i].type == '(') {
				lparenthese_num++;
			}
			else if(tokens[i].type == ')') {
				rparenthese_num++;
			}
			else if(tokens[i].type == AND || tokens[i].type == OR) {
                                if(lparenthese_num == rparenthese_num) {
				/*The left and right parentheses must be equal*/
                                        priority1 = 1;
                                        op = i;
                                }
                        }
			else if(tokens[i].type == EQ || tokens[i].type == NOTEQ) {
                                if(lparenthese_num == rparenthese_num && priority1 == 0) {
					/*parentheses are equal and there is no 'OR', 'AND' */
                                        priority2 = 1;
                                        op = i;
                                }
                        }
			else if(tokens[i].type == '+' || tokens[i].type == '-') {
                                if(lparenthese_num == rparenthese_num&&priority2 == 0&&priority1 == 0) {
					//There isn't  any other lower priority operation, so "+" and "-" can be dominant operator
                                        priority3 = 1;
                                        op = i;
                                }
                        }
			else if(tokens[i].type == '*' || tokens[i].type == '/') {
				if(lparenthese_num == rparenthese_num && priority1 == 0&&priority2 == 0&&priority3 == 0) {
					//There is no "+', "-" or any other lower priority operation, so "*" and "/" can be dominant operator
					op = i;
					priority4 = 1;
				}
			}
		}
		if(priority1 == 0&&priority2 == 0&&priority3 == 0&&priority4 == 0) {
			/*No binary operation, calculate unary operation*/
			if(tokens[p].type == NOT)
				return !eval(p + 1, q);
			else if(tokens[p].type == NEGATIVE)
				return -eval(p + 1, q);
			else if(tokens[p].type == DEREFERENCE) {
				uint32_t add;
				add = eval(p + 1, q);
				return swaddr_read(add, 4);
				assert(0);
			}
		}

		/*Then divide it into two parts to evaluate*/
		if( op == 0 && (tokens[p].type == NEGATIVE||tokens[p].type == '+'))
			var1 = 0;
		else
			var1 = eval(p, op - 1);
		var2 = eval(op + 1, q);
		switch(tokens[op].type) {
			case '+': result = var1 + var2;break;
			case '-': result = var1 - var2;break; 
			case '*': result = var1*var2;break;	
			case '/': result = var1/var2;break;
			case OR: result = (var1||var2);break;
			case AND: result = (var1&&var2);break;
			case EQ: result = (var1==var2);break;
			case NOTEQ: result = (var1!=var2);break;
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
	int m;
        for(m = 0; m < nr_token; m++) {
                /* Mark out which one is minus sign instead of minus, so does dereference operation "*" */
                if(tokens[m].type == '-'&&(m == 0||(tokens[m-1].type != NUM && tokens[m-1].type != HEXNUM
                   && tokens[m-1].type != DOLREG && tokens[m-1].type != ')'))) {
                        tokens[m].type = NEGATIVE;
                }
		else if(tokens[m].type == '*'&&(m == 0||(tokens[m-1].type != NUM && tokens[m-1].type != HEXNUM
                   && tokens[m-1].type != DOLREG && tokens[m-1].type != ')'))) {
                        tokens[m].type = DEREFERENCE;
                }
        }

	return eval(0, nr_token - 1);
	panic("please implement me");
	return 0;
}
