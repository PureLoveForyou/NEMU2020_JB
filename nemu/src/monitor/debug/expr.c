#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ, NUM, NEGATIVE, HEXNUM, DEREFERENCE,
	NOTEQ, AND, OR, NOT


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

	{"\\$", '$'},					//Access registers

	{"==", EQ},					//equal
	{"!=", NOTEQ},					//not equal

	{"&&", AND},					// equal
//	{"||", OR},					// equal
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
					case '$':	tokens[nr_token++].type = rules[i].token_type;break;
					case NOT:       tokens[nr_token++].type = rules[i].token_type;break;
					case NOTYPE: 	break;
					case HEXNUM: 	tokens[nr_token].type = rules[i].token_type; 
							int k, l;
							for(k = 0; k < 32; k++)
								tokens[nr_token].str[k] = '0';
							for(k = 32 - substr_len +2, l = 2; k < 32; k++, l++)
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
				return false;//Right parentheses are more than the left
			}
			p++;
		}
		if(stack_count == 0) {
			flag = true;
		}
		else {
			flag = false;
		}
	}
	return flag;
}

static uint32_t eval(int p, int q) {
	uint32_t result;
	int negative_flag = 0;//negative_flag is used to record whether there is a minus sign
	int dereference_flag = 0, not_flag = 0;

	if(p > q) {
		/*bad expression*/
		printf("Illegal expression\n");
		assert(0);
	}
	if((tokens[p].type == NEGATIVE||tokens[p].type == DEREFERENCE||tokens[p].type == NOT)&&p < q) {
		if(tokens[p].type == NEGATIVE) {
			/*The number is a negative*/
	        	result = -eval(p + 1, q);
		}
		else if(tokens[p].type == NOT) {
                	result = !eval(p + 1, q);;
		}
//		else if(tokens[p].type == DEREFERENCE) {
//                      /* Sign dereference '*' */
//                        dereference_flag = 1;   
//                        p++;
//                }
	}
	if(p == q) {
		/*Single token. And it should be a number*/
		int i;
		if(tokens[p].type == NUM) {
			result = 0;
			for(i = 0; i < 32; i++) {
				result = result*10 + (uint32_t)(tokens[p].str[i] - '0');
			}
			if(negative_flag == 1)
				result = -result;//if this number is a negative, return -number
			else if(not_flag == 1)
				result = !result;
			else if(dereference_flag == 1) {
				printf("NO\n");
				assert(0);
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
	else if((tokens[p].type == NEGATIVE)&&(check_parentheses(p + 1, q) == true)) {
		/*The expression is -(expression), return 0-expression*/
		result = 0 - eval(p + 2, q - 1);
	}
	else if(check_parentheses(p, q) == true) {
		/*The expression is (expression). Throw '(' and ')' away*/
		result =  eval(p + 1, q - 1);
	}
	else {
		/*First find out where the dominant operator is*/
		int i, lparenthese_num = 0, rparenthese_num = 0, var1, var2, op = 0;
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
				if(lparenthese_num == rparenthese_num && lowpriority == 0) {
					op = i;//There has no "+' and "-", so "*" and "/" can be dominant operator
				}
			}
		}

		/*Then divide it into two parts to evaluate*/
		if( op == 0)
			var1 = 0;
		else
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
	int m;
	for(m = 0; m < nr_token; m++) {
                /* Mark out which one is minus sign instead of minus, so does "*" */
                if(tokens[m].type == '-'&&(m == 0||tokens[m-1].type == '+'||tokens[m-1].type == '-'
                   ||tokens[m-1].type == '*'||tokens[m-1].type == '/'||tokens[m-1].type == '(')) {
                        tokens[m].type = NEGATIVE;
                }
                else if(tokens[m].type == '*'&&(m == 0||tokens[m-1].type == '+'||tokens[m-1].type == '-'
                        ||tokens[m-1].type == '*'||tokens[m-1].type == '/'||tokens[m-1].type == '(')) {
                        tokens[m].type = DEREFERENCE;
                }
        }

	return eval(0, nr_token - 1);
	panic("please implement me");
	return 0;
}
