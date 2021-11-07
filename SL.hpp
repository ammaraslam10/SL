#ifndef SL_H
#define SL_H

//#define DEBUG
#if __GNUC__ > 3
#define LINUX
#endif

#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include "plf_stack.hpp"
#include "growing_array.hpp"
#include "hash_table.hpp"
#include "heap_manager.hpp"
#include <memory>
#include <math.h>

#ifndef EMPTYOBJ
#define EMPTYOBJ
struct Empty
{
};
#endif

namespace SL3
{
#define string std::string
#define vector std::vector
#define cout std::cout
#define chrono std::chrono
#define stack plf::stack
#define unordered_map std::unordered_map
#define to_string std::to_string
#define shared_ptr std::shared_ptr
#define unique_ptr std::unique_ptr

	// Do something about errors
	inline void err(string e, int err_type = 0)
	{
		cout << e;
		exit(0);
	}

	enum TOKENS
	{
		ERROR,
		STRING,
		NUMERIC,
		IDENTIFIER,
		LITERAL,
		OPERATOR,
		PAREN_O,
		PAREN_C,
		CURLY_O,
		CURLY_C,
		SQUARE_O,
		SQUARE_C,
		COMMA,
		SEMICOLON
	};
	struct TOKEN
	{
		TOKENS token;
		string value;
		int line;
	};
	// Make tree
	class Analyze
	{
		int pointer, length, line;
		string code;
		// Skip endls
		inline void check_line(int &line)
		{
			if (code[pointer] == '\n')
				line++;
		}
		// Skip comments
		void skip_comments(int &line, bool single = true, bool doubl = true)
		{
			if (single && pointer + 1 < length && code[pointer] == '/' && code[pointer + 1] == '/')
			{
				while (pointer < length && code[pointer] != '\n')
				{
					pointer++;
				}
				line++;
				skip_comments(line, single, doubl);
			}
			if (doubl && pointer + 1 < length && code[pointer] == '/' && code[pointer + 1] == '*')
			{
				pointer += 2;
				while (pointer + 1 < length && (code[pointer] != '*' || code[pointer + 1] != '/'))
				{
					check_line(line);
					pointer += 1;
				}
				if (pointer < length && pointer + 1 < length)
					pointer += 2;
				skip_comments(line, single, doubl);
			}
		}
		// Functions for each token type
		TOKEN _string()
		{
			int start = pointer;
			char quote = code[pointer++];
			TOKEN ret;
			ret.line = line;
			while (pointer < length)
			{
				skip_comments(ret.line);
				if (code[pointer] != quote)
				{
					check_line(ret.line);
					pointer++;
				}
				else
					break;
			}
			ret.token = TOKENS::STRING;
			ret.value = code.substr(start + 1, pointer - start - 1);
			pointer++;
			return ret;
		}
		TOKEN numeric()
		{
			int start = pointer;
			bool dec = false;
			TOKEN ret;
			ret.line = line;
			while (pointer < length)
			{
				skip_comments(ret.line, false, true);
				if (code[pointer] >= '0' && code[pointer] <= '9')
				{
					pointer++;
				}
				else if (code[pointer] == '.' && dec == false)
				{
					pointer++;
					dec = true;
				}
				else
					break;
			}
			ret.value = code.substr(start, pointer - start);
			ret.token = NUMERIC;
			return ret;
		}
		TOKEN identifier()
		{
			int start = pointer++;
			TOKEN ret;
			ret.line = line;
			if (pointer < length && (code[pointer] >= 'A' && code[pointer] <= 'Z') ||
				(code[pointer] >= 'a' && code[pointer] <= 'z') || code[pointer] == '_')
				pointer++;
			while (pointer < length)
			{
				skip_comments(ret.line, false, true);
				if ((code[pointer] >= '0' && code[pointer] <= '9') || (code[pointer] >= 'A' && code[pointer] <= 'Z') ||
					(code[pointer] >= 'a' && code[pointer] <= 'z') || code[pointer] == '_')
				{
					pointer++;
				}
				else
					break;
			}
			ret.value = code.substr(start, pointer - start);
			ret.token = TOKENS::IDENTIFIER;
			return ret;
		}

	public:
		vector<TOKEN> analyze(string &c)
		{
			vector<TOKEN> token_tree;
			code = c;
			length = c.length();
			line = 1;
			for (pointer = 0; pointer < length;)
			{
				skip_comments(line);
				// string
				if (code[pointer] == '"' || code[pointer] == '\'')
				{
					token_tree.push_back(_string());
				}
				// numeric
				else if (code[pointer] >= '0' && code[pointer] <= '9')
				{
					token_tree.push_back(numeric());
				}
				// iden
				else if (code[pointer] == '$')
				{
					token_tree.push_back({OPERATOR, "$", line});
					pointer++;
					token_tree.push_back(identifier());
				}
				// literal
				else if ((code[pointer] >= 'A' && code[pointer] <= 'Z') ||
						 (code[pointer] >= 'a' && code[pointer] <= 'z') ||
						 code[pointer] == '_')
				{
					token_tree.push_back(identifier());
					token_tree[token_tree.size() - 1].token = LITERAL;
				}
				// spaces
				else if (code[pointer] == ' ' || code[pointer] == '\t')
				{
					pointer++;
				}
				// operator
				else if (code[pointer] == '=' || code[pointer] == '<' || code[pointer] == '>' ||
						 code[pointer] == '!' || code[pointer] == '+' || code[pointer] == '-' ||
						 code[pointer] == '*' || code[pointer] == '/' || code[pointer] == '%')
				{
					string p = "";
					p = p + code[pointer];
					if (pointer + 1 < length && code[pointer + 1] == '=')
					{
						pointer++;
						p = p + "=";
					}
					else if (pointer + 1 < length && code[pointer] == '+' && code[pointer + 1] == '+')
					{
						pointer++;
						p = p + "+";
					}
					else if (pointer + 1 < length && code[pointer] == '-' && code[pointer + 1] == '-')
					{
						pointer++;
						p = p + "-";
					}
					if (pointer + 2 < length && code[pointer + 2] == '=')
					{
						pointer++;
						p = p + "=";
					}
					token_tree.push_back({OPERATOR, p, line});
					pointer++;
				}
				else if (c[pointer] == '(')
				{
					token_tree.push_back({PAREN_O, "(", line});
					pointer++;
				}
				else if (c[pointer] == ')')
				{
					token_tree.push_back({PAREN_C, ")", line});
					pointer++;
				}
				else if (c[pointer] == '{')
				{
					token_tree.push_back({CURLY_O, "{", line});
					pointer++;
				}
				else if (c[pointer] == '}')
				{
					token_tree.push_back({CURLY_C, "}", line});
					pointer++;
				}
				else if (c[pointer] == '[')
				{
					token_tree.push_back({SQUARE_O, "[", line});
					pointer++;
				}
				else if (c[pointer] == ']')
				{
					token_tree.push_back({SQUARE_C, "]", line});
					pointer++;
				}
				else if (c[pointer] == ',')
				{
					token_tree.push_back({COMMA, ",", line});
					pointer++;
				}
				else if (c[pointer] == ';')
				{
					token_tree.push_back({SEMICOLON, ";", line});
					pointer++;
				}
				else if (c[pointer] == '\n' || c[pointer] == '\r')
				{
					line++;
					pointer++;
				}
				else
				{
					err("Syntax error, unexpected '" + to_string(c[pointer]) + "' at line " + to_string(line));
					exit(0);
				}
			}
			return token_tree;
		}
		void display(vector<TOKEN> &tree)
		{
			unsigned int tabs = 0;
			for (unsigned int i = 0; i < tree.size(); i++)
			{
				cout << " ";
				if (tree[i].token == STRING)
				{
					cout << "STRING";
				}
				else if (tree[i].token == NUMERIC)
				{
					cout << "NUMERIC";
				}
				else if (tree[i].token == IDENTIFIER)
				{
					cout << "IDENTIFIER";
				}
				else
				{
					cout << tree[i].value;
					if (tree[i].value == "{")
					{
						tabs++;
					}
					if (tree[i].value == "}")
					{
						tabs--;
					}
				}

				if (tree[i].token == SEMICOLON || tree[i].value == "{" || tree[i].value == "}")
				{
					cout << "\n";
					unsigned int j = 0;
					if (i + 1 < tree.size() && tree[i + 1].value == "}")
						j = 1;
					for (; j < tabs && tabs > 0; j++)
						cout << "\t";
				}
			}
		}
	};

	// What num corresponds to what symbol
#define SKIP_KEYW 65
	enum SL_LANG_KEYS
	{
		KEYW_ERR = 0,
		KEYW_ENDL = 10,
		KEYW_NULL = SKIP_KEYW,
		KEYW_FOR,
		KEYW_WHILE,
		KEYW_IF,
		KEYW_ELSE,
		KEYW_RETURN,
		KEYW_BREAK,
		KEYW_CONTINUE,
		KEYW_FUNCTION,
		KEYW_CLASS,
		KEYW_DEFAULT,
		OPR_VAR,
		OPR_SEMICLN /*;*/,
		OPR_FLOWER_BKT_OPN /*{*/,
		OPR_FLOWER_BKT_CLS /*}*/,
		OPR_NOT /*!*/,
		OPR_ROUND_BKT_OPN /*(*/,
		OPR_ROUND_BKT_CLS /*)*/,
		OPR_QUOTE /*'*/,
		OPR_DOUBLE_QUOTE /*"*/,
		OPR_LIST,
		OPR_COMMA /*,*/,
		OPR_SQUARE_BKT_OPN /*[*/,
		OPR_SQUARE_BKT_CLS /*]*/,
		OPR_AND,
		OPR_OR,
		OPR_EQUAL /*=*/,
		OPR_MOD /*%*/,
		OPR_DIV /*/*/,
		OPR_MUL /***/,
		OPR_ADD /*+*/,
		OPR_SUB /*-*/,
		OPR_LESS /*<*/,
		OPR_LESS_EQ /*<=*/,
		OPR_GREATER /*>*/,
		OPR_GREATER_EQ /*>=*/,
		OPR_COMPARE /*==*/,
		OPR_COMPARE_STR /*===*/,
		OPR_NOT_EQ /*!=*/,
		OPR_ADD_EQ /*+=*/,
		OPR_SUB_EQ /*-=*/,
		OPR_MUL_EQ /**=*/,
		OPR_DIV_EQ, /*/=*/
		OPR_MOD_EQ /*%=*/,
		OPR_INCR /*++*/,
		OPR_DECR /*--*/,
		OPR_CNST /*@*/,
		OPR_CNST_RES /*`*/,
		OPR_GO_FWD,
		OPR_GO_BCK,
		OPR_LNO_P,
		OPR_FUNC_OPN,
		OPR_FUNC_CLS,
		OPR_SKP
	};
#define OPR_STRT OPR_LIST
#define OPR_END OPR_DECR

	unordered_map<string, int> SL_LANG_KEYS = {{"null", KEYW_NULL}, {"for", KEYW_FOR}, {"while", KEYW_WHILE}, {"if", KEYW_IF}, {"else", KEYW_ELSE}, {"function", KEYW_FUNCTION}, {"return", KEYW_RETURN}, {"default", KEYW_DEFAULT}, {"=", OPR_EQUAL}, {",", OPR_COMMA}, {"$", OPR_VAR}, {";", OPR_SEMICLN}, {"[", OPR_SQUARE_BKT_OPN}, {"]", OPR_SQUARE_BKT_CLS}, {"{", OPR_FLOWER_BKT_OPN}, {"&&", OPR_AND}, {"||", OPR_OR}, {"and", OPR_AND}, {"or", OPR_OR}, {"}", OPR_FLOWER_BKT_CLS}, {"(", OPR_ROUND_BKT_OPN}, {")", OPR_ROUND_BKT_CLS}, {"!", OPR_NOT}, {"%", OPR_MOD}, {"/", OPR_DIV}, {"*", OPR_MUL}, {"+", OPR_ADD}, {"-", OPR_SUB}, {"<", OPR_LESS}, {"<=", OPR_LESS_EQ}, {">", OPR_GREATER}, {">=", OPR_GREATER_EQ}, {"==", OPR_COMPARE}, {"===", OPR_COMPARE_STR}, {"!=", OPR_NOT_EQ}, {"+=", OPR_ADD_EQ}, {"-=", OPR_SUB_EQ}, {"*=", OPR_MUL_EQ}, {"/=", OPR_DIV_EQ}, {"%=", OPR_MOD_EQ}, {"'", OPR_QUOTE}, {"\"", OPR_DOUBLE_QUOTE}, {"++", OPR_INCR}, {"--", OPR_DECR}, {"@", OPR_CNST}, {"`", OPR_CNST_RES}, {"~array", OPR_LIST}};

	// Dynamic memory should start from string onwards
	// Array must come before list & tuple, list & tuple need to be the last elems (= optimization)
	enum VARIABLE_TYPES
	{
		VAR_TYPE_NULL,
		VAR_TYPE_BOOL,
		VAR_TYPE_INT,
		VAR_TYPE_DOUBLE,
		VAR_TYPE_ERROR,
		VAR_TYPE_VAR,
		VAR_TYPE_STRING,
		VAR_TYPE_ARRAY,
		VAR_TYPE_LIST,
		VAR_TYPE_TUPLE
	};
	struct variable;
	struct VScope;

	// Structure to store & pre-load memory that may be used
	// Array of VScope (Which contains list of variable references),
	// Array of dynamic arrays & A variable stack for recursion
	struct Memory
	{
		heap_manager<VScope> vscope_heap;
		heap_manager<hash_table<variable>> var_ht_heap;
		growing_array<variable> vstack;
		int vstack_count;
		Memory() : vscope_heap(16, 8), var_ht_heap(16, 8), vstack(128) { vstack_count = 0; }
	} MGlobals;

	bool is_num(string);
	struct variable
	{
		union
		{
			long int_data;
			double double_data;
			string *string_data;
			hash_table<variable> *array_data;
			variable *var_data;
		} data;
		unsigned char type;

		inline variable() { type = VAR_TYPE_ERROR; }
		inline variable(int num)
		{
			type = VAR_TYPE_INT;
			data.int_data = num;
		}
		inline variable(long num)
		{
			type = VAR_TYPE_INT;
			data.int_data = num;
		}
		inline variable(double num)
		{
			type = VAR_TYPE_DOUBLE;
			data.double_data = num;
		}
		inline variable(string str, bool fix = false) { var_value_store(str, fix); }
		// Store another variable as a reference
		// inline variable(variable* ref) { type = VAR_TYPE_VAR; data.var_data = ref; }
		inline variable(char _type, int extra)
		{
			type = _type;
			if (type == VAR_TYPE_ARRAY || type == VAR_TYPE_LIST)
			{
				data.array_data = MGlobals.var_ht_heap.get();
				//new hash_table<variable>(extra);
			}
		}
		// Return a referenced version of self
		inline variable ref()
		{
			if (type == VAR_TYPE_VAR)
				return *this;
			variable ref;
			ref.type = VAR_TYPE_VAR;
			ref.data.var_data = this;
			return ref;
		}
		// Dereference
		inline variable deref()
		{
			if (type == VAR_TYPE_VAR)
				return *(this->data.var_data);
			return *this;
		}
		// Return a unreferenced version of self
		inline variable deref_move()
		{
			variable temp;
			if (type == VAR_TYPE_VAR)
			{
				temp.type = data.var_data->type;
				temp.data = data.var_data->data;
				data.var_data->type = VAR_TYPE_NULL;
			}
			else
			{
				temp.type = type;
				temp.data = data;
				type = VAR_TYPE_NULL;
			}
			return temp;
		}
		// Move constructor
		inline variable(variable &&var)
		{
			type = var.type;
			data = var.data;
			var.type = VAR_TYPE_NULL;
		}
		// Copy constructor
		inline variable(const variable &var)
		{
			type = var.type;
			data = var.data;
			if (type >= VAR_TYPE_STRING)
			{
				//cout << "array cpy cons";
				if (type == VAR_TYPE_STRING)
				{
					data.string_data = new string(*var.data.string_data);
				}
				else
				{
					data.array_data = MGlobals.var_ht_heap.get();
					data.array_data->copy(var.data.array_data);
					//data.array_data = new hash_table<variable>(*data.array_data);
				}
			}
		}
		inline ~variable()
		{
			if (type >= VAR_TYPE_STRING)
			{
				//cout << "array des";
				if (type == VAR_TYPE_STRING)
				{
					delete data.string_data;
				}
				else
				{
					data.array_data->clear();
					MGlobals.var_ht_heap.ret(data.array_data); // delete data.array_data;
				}
			}
			type = VAR_TYPE_NULL;
		}

		inline variable operator/(variable a)
		{
			if (a.type == VAR_TYPE_INT)
			{
				if (type == VAR_TYPE_INT)
				{
					if (data.int_data % a.data.int_data == 0)
						return (data.int_data / a.data.int_data);
					else
						return (data.int_data / (double)a.data.int_data);
				}
				if (type == VAR_TYPE_DOUBLE)
					return (data.double_data / a.data.int_data);
			}
			if (a.type == VAR_TYPE_DOUBLE)
			{
				if (type == VAR_TYPE_INT)
					return (data.int_data / a.data.double_data);
				if (type == VAR_TYPE_DOUBLE)
					return (data.double_data / a.data.double_data);
			}
			err("Variable:: Operation / between types " + var_type() + ", " + a.var_type());
			return variable();
		}
		inline variable operator%(variable a)
		{
			if (a.type == VAR_TYPE_INT)
			{
				if (type == VAR_TYPE_INT)
					return (data.int_data % a.data.int_data);
				if (type == VAR_TYPE_DOUBLE)
					return ((int)data.double_data % a.data.int_data);
			}
			if (a.type == VAR_TYPE_DOUBLE)
			{
				if (type == VAR_TYPE_INT)
					return (data.int_data % (int)a.data.double_data);
				if (type == VAR_TYPE_DOUBLE)
					return ((int)data.double_data % (int)a.data.double_data);
			}
			err("Variable:: Operation % between types " + var_type() + ", " + a.var_type());
			return variable();
		}
		inline variable *operator[](variable a)
		{
			if (a.type == VAR_TYPE_INT)
			{
				if (type == VAR_TYPE_ARRAY)
					return data.array_data->get_add(a.data.int_data);
			}
			if (a.type == VAR_TYPE_DOUBLE)
			{
				if (type == VAR_TYPE_ARRAY)
					return data.array_data->get_add((int)a.data.double_data);
			}
			if (a.type == VAR_TYPE_STRING)
			{
				if (type == VAR_TYPE_ARRAY)
					return data.array_data->get_add(a.data.string_data->c_str(), a.data.string_data->length());
			}
			err("Variable:: Operation [] between types " + var_type() + ", " + a.var_type());
			return 0;
		}
		/*inline void operator=(variable* a) {
			this->~variable();
			type = VAR_TYPE_VAR; data.var_data = a;
		}*/
		inline void tuple_declare(variable &a)
		{
			if (type == VAR_TYPE_TUPLE && a.type == VAR_TYPE_TUPLE)
			{
				int l = data.array_data->length(), m = a.data.array_data->length();
				for (int i = 0; i < l && i < m; i++)
				{
					if (data.array_data->access(i)->type == VAR_TYPE_VAR)
					{
						variable temp = a.data.array_data->access(i)->deref();
						if (temp.type == VAR_TYPE_ERROR)
							continue;
						data.array_data->access(i)->data.var_data->data = temp.data;
						data.array_data->access(i)->data.var_data->type = temp.type;
						temp.type = VAR_TYPE_NULL;							// Don't throw contents at garbage
						a.data.array_data->access(i)->type = VAR_TYPE_NULL; // Don't throw contents at garbage
					}
				}
			}
		}
		inline void operator=(variable &&a)
		{
#ifdef DEBUG
			if (type >= VAR_TYPE_STRING)
				err("Memory leakage!");
#endif
			type = a.type;
			data = a.data;
			a.type = VAR_TYPE_NULL;
		}
		inline void operator=(const variable &a)
		{
			if (type == VAR_TYPE_VAR)
			{
				data.var_data->type = a.type;
				if (a.type >= VAR_TYPE_STRING)
				{
					if (a.type == VAR_TYPE_STRING)
					{
						data.var_data->data.string_data = new string(*a.data.string_data);
					}
					else
					{
						data.var_data->data.array_data = MGlobals.var_ht_heap.get();
						data.var_data->data.array_data->copy(a.data.array_data);
						//data.var_data->data.array_data = new hash_table<variable>(*a.data.array_data);
					}
				}
				else
				{
					data.var_data->data = a.data;
				}
				return;
			}
			this->~variable();
			type = a.type;
			if (a.type >= VAR_TYPE_STRING)
			{
				if (a.type == VAR_TYPE_STRING)
				{
					data.string_data = new string(*a.data.string_data);
				}
				else
				{
					data.array_data = MGlobals.var_ht_heap.get();
					data.array_data->copy(a.data.array_data);
					//data.array_data = new hash_table<variable>(*a.data.array_data);
				}
			}
			else
			{
				data = a.data;
			}
		}
		// Normal operations
#define n_oper(X)                                           \
	if (a.type == VAR_TYPE_INT)                             \
	{                                                       \
		if (type == VAR_TYPE_INT)                           \
			return (data.int_data X a.data.int_data);       \
		if (type == VAR_TYPE_DOUBLE)                        \
			return (data.double_data X a.data.int_data);    \
	}                                                       \
	else if (a.type == VAR_TYPE_DOUBLE)                     \
	{                                                       \
		if (type == VAR_TYPE_INT)                           \
			return (data.int_data X a.data.double_data);    \
		if (type == VAR_TYPE_DOUBLE)                        \
			return (data.double_data X a.data.double_data); \
	}
		// Promote integer datatype to double
#define p_oper(X)                                           \
	if (a.type == VAR_TYPE_INT)                             \
	{                                                       \
		if (type == VAR_TYPE_INT)                           \
			return (data.int_data X a.data.int_data);       \
		if (type == VAR_TYPE_DOUBLE)                        \
			return (data.double_data X a.data.int_data);    \
	}                                                       \
	else if (a.type == VAR_TYPE_DOUBLE)                     \
	{                                                       \
		if (type == VAR_TYPE_INT)                           \
		{                                                   \
			type = VAR_TYPE_DOUBLE;                         \
			data.double_data = data.int_data;               \
			return (data.double_data X a.data.double_data); \
		}                                                   \
		if (type == VAR_TYPE_DOUBLE)                        \
			return (data.double_data X a.data.double_data); \
	}
		// Conditional operations apply to null in addition to other types
#define c_oper(X) \
	n_oper(X) if (type == VAR_TYPE_NULL) return false;
		// Compute the conditions through U(X) if nothing valid was done, throw error
#define expandoperators(X, T, U)                                                              \
	T operator X(variable &a)                                                                 \
	{                                                                                         \
		U(X)                                                                                  \
		err("Variable:: Operation " #X " between types " + var_type() + ", " + a.var_type()); \
		return 0;                                                                             \
	}
		expandoperators(+, variable, n_oper);
		expandoperators(-, variable, n_oper);
		expandoperators(*, variable, n_oper);
		expandoperators(==, bool, c_oper);
		expandoperators(>, bool, c_oper);
		expandoperators(<, bool, c_oper);
		expandoperators(<=, bool, c_oper);
		expandoperators(>=, bool, c_oper);
		expandoperators(+=, bool, p_oper);
		expandoperators(-=, bool, p_oper);
		expandoperators(*=, bool, p_oper);
		expandoperators(/=, bool, p_oper);
		expandoperators(&&, bool, p_oper);
		expandoperators(||, bool, p_oper);

		inline bool operator!() const
		{
			if (type == VAR_TYPE_NULL || type == VAR_TYPE_ERROR)
				return true;
			return false;
		}
		inline void operator=(Empty e) { this->~variable(); }
		// Just some tiny display code
		void display()
		{
			if (type == VAR_TYPE_ERROR)
			{
				cout << "ERR";
			}
			else if (type == VAR_TYPE_NULL)
			{
				cout << "NULL";
			}
			else if (type == VAR_TYPE_INT)
			{
				cout << data.int_data;
			}
			else if (type == VAR_TYPE_DOUBLE)
			{
				cout << data.double_data;
			}
			else if (type == VAR_TYPE_STRING)
			{
				cout << *data.string_data;
			}
			else if (type == VAR_TYPE_ARRAY)
			{
				cout << "[";
				if (data.array_data->length() > 0)
				{
					for (unsigned int i = 0; i < data.array_data->length() - 1; i++)
					{
						variable *temp = data.array_data->access(i);
						if (temp)
							temp->display();
						cout << ", ";
					}
					variable *temp = data.array_data->access(data.array_data->length() - 1);
					if (temp)
						temp->display();
				}
				cout << "]";
			}
			else if (type == VAR_TYPE_LIST || type == VAR_TYPE_TUPLE)
			{
				if (type == VAR_TYPE_LIST)
					cout << "LIST(";
				if (type == VAR_TYPE_TUPLE)
					cout << "TUPLE(";
				if (data.array_data->length() > 0)
				{
					for (unsigned int i = 0; i < data.array_data->length() - 1; i++)
					{
						variable *temp = data.array_data->access(i);
						if (temp)
							temp->display();
						cout << ", ";
					}
					variable *temp = data.array_data->access(data.array_data->length() - 1);
					if (temp)
						temp->display();
				}
				cout << ")";
			}
			else if (type == VAR_TYPE_VAR)
			{
				cout << "REF ";
				data.var_data->display();
			}
			else
			{
				cout << "???";
			}
		}
		/*// Convert self to an array
		static void var_convert_arr(int size) {
			hash_table<variable>* tmp = new hash_table<variable>(size + 1);
			tmp->add(this);
			data.array_data = tmp;
			type = VAR_TYPE_ARRAY;
		}*/
		void var_value_store(string str, bool fix = false)
		{
			data.string_data = new string(str);

			type = VAR_TYPE_STRING;
			/*const char * cpy = str.c_str();
			strcpy_s(data.string_data.string_data, data.string_data.length+1, cpy);*/
			if (fix && is_num(*data.string_data))
				to_num(*data.string_data);
		}
		inline void to_num()
		{
			if (type == VAR_TYPE_STRING)
				to_num(*data.string_data);
			else
			{
				type = VAR_TYPE_INT;
				data.int_data = 0;
			}
		}
		void to_num(string str)
		{
			bool isNeg = false, isDbl = false;
			int result = 0;
			double dresult = 0;
			if (str[0] == '-')
			{
				isNeg = true;
			}
			int i = 0;
			if (str[0] == '-' || str[0] == '+')
			{
				i = 1;
			}
			int size = str.length();
			while (i < size)
			{
				if (str[i] >= '0' && str[i] <= '9')
					result = result * 10 + (str[i] - '0');
				else if (str[i] == '.')
				{
					isDbl = true;
					dresult = result;
					break;
				}
				else
					break;
				i++;
			}
			// Loop for decimal points
			if (isDbl)
			{
				double decimal = 10;
				for (i++; i < size; i++)
				{
					if (str[i] >= '0' && str[i] <= '9')
						dresult = dresult + (str[i] - '0') / decimal;
					else
						break;
					decimal *= decimal;
				}
			}
			if (isNeg)
			{
				result *= -1;
			}
			if (isDbl)
			{
				data.double_data = dresult;
				type = VAR_TYPE_DOUBLE;
			}
			else
			{
				data.int_data = result;
				type = VAR_TYPE_INT;
			}
		}
		inline string var_type()
		{
			if (type == VAR_TYPE_ERROR)
				return "error";
			else if (type == VAR_TYPE_INT)
				return "integer";
			else if (type == VAR_TYPE_STRING)
				return "string";
			else if (type == VAR_TYPE_ARRAY)
				return "array";
			else if (type == VAR_TYPE_DOUBLE)
				return "double";
			else if (type == VAR_TYPE_LIST)
				return "list";
			else if (type == VAR_TYPE_VAR)
				return "variable";
			else if (type == VAR_TYPE_TUPLE)
				return "tuple";
			else
				return "? (" + to_string(type) + ")";
		}
	};

#define INIT_NO 1
	unordered_map<string, variable *(*)(variable &)> builtin_functions_list; // = { { "echo", BFUNC_ECHO } };
	struct CScope
	{
		growing_array<variable *> constants_list;
		unordered_map<string, int> real_constants_list;
		int constants_list_id = INIT_NO + 2; // 1 = NULL, 2 = ERR
		CScope(int minimum = 32) : constants_list(minimum)
		{
			constants_list[1] = new variable();
			constants_list[1]->type = VAR_TYPE_NULL;
			constants_list[2] = new variable();
			constants_list[2]->type = VAR_TYPE_ERROR;
		}
	} CGlobals;
	struct VScope
	{
		growing_array<variable *> variable_list;
		unordered_map<string, int> real_variable_list;
		int real_variable_id = INIT_NO;
		VScope(int minimum = 32) : variable_list(minimum)
		{
			int s = variable_list.size();
			for (int i = 0; i < s; i++)
			{
				variable_list.direct_access(i) = 0;
			}
		}
		VScope(const VScope &scp)
		{
			int s = ((VScope &)scp).variable_list.size();
			for (int i = 0; i < s; i++)
			{
				variable_list[i] = ((VScope &)scp).variable_list.direct_access(i);
			}
		}
		~VScope()
		{
			int s = variable_list.size();
			for (int i = 0; i < s; i++)
			{
				if (variable_list.direct_access(i) != 0)
				{
					delete variable_list.direct_access(i);
					variable_list.direct_access(i) = 0;
				}
			}
		}
	} VGlobals;
	struct FScope
	{
		growing_array<unsigned long long int> functions_list;
		unordered_map<string, int> real_functions_list;
		unordered_map<int, VScope> real_functions_vscope;
		int real_functions_id = INIT_NO + builtin_functions_list.size();
		FScope(int minimum = 32) : functions_list(minimum) {}
	} FGlobals;

	// Map back
	string int_to_human(int n)
	{
		if (n == KEYW_NULL)
			return "null";
		else if (n == KEYW_FOR)
			return "for";
		else if (n == KEYW_IF)
			return "if";
		else if (n == KEYW_ELSE)
			return "else";
		else if (n == KEYW_WHILE)
			return "while";
		else if (n == KEYW_RETURN)
			return "return";
		else if (n == KEYW_FUNCTION)
			return "function";
		else if (n == OPR_FUNC_OPN)
			return "f(";
		else if (n == OPR_FUNC_CLS)
			return ")";
		else if (n == OPR_EQUAL)
			return "=";
		else if (n == OPR_COMMA)
			return ",";
		else if (n == OPR_VAR)
			return "$";
		else if (n == OPR_SEMICLN)
			return ";";
		else if (n == OPR_FLOWER_BKT_OPN)
			return "{";
		else if (n == OPR_AND)
			return "and";
		else if (n == OPR_OR)
			return "or";
		else if (n == OPR_FLOWER_BKT_CLS)
			return "}";
		else if (n == OPR_ROUND_BKT_OPN)
			return "(";
		else if (n == OPR_ROUND_BKT_CLS)
			return ")";
		else if (n == OPR_NOT)
			return "!";
		else if (n == OPR_MOD)
			return "%";
		else if (n == OPR_DIV)
			return "/";
		else if (n == OPR_MUL)
			return "*";
		else if (n == OPR_ADD)
			return "+";
		else if (n == OPR_SUB)
			return "-";
		else if (n == OPR_LESS)
			return "<";
		else if (n == OPR_LESS_EQ)
			return "<=";
		else if (n == OPR_GREATER)
			return ">";
		else if (n == OPR_GREATER_EQ)
			return ">=";
		else if (n == OPR_COMPARE)
			return "==";
		else if (n == OPR_COMPARE_STR)
			return "===";
		else if (n == OPR_NOT_EQ)
			return "!=";
		else if (n == OPR_ADD_EQ)
			return "+=";
		else if (n == OPR_SUB_EQ)
			return "-=";
		else if (n == OPR_MUL_EQ)
			return "*=";
		else if (n == OPR_DIV_EQ)
			return "/=";
		else if (n == OPR_MOD_EQ)
			return "%=";
		else if (n == OPR_QUOTE)
			return "'";
		else if (n == OPR_DOUBLE_QUOTE)
			return "\"";
		else if (n == OPR_INCR)
			return "++";
		else if (n == OPR_DECR)
			return "--";
		else if (n == OPR_CNST)
			return "@";
		else if (n == OPR_CNST_RES)
			return "`";
		else if (n == OPR_GO_FWD)
			return "~>";
		else if (n == OPR_GO_BCK)
			return "<~";
		else if (n == OPR_SKP)
			return "SKP";
		else if (n == OPR_SQUARE_BKT_OPN)
			return "[";
		else if (n == OPR_SQUARE_BKT_CLS)
			return "[]";
		else if (n == OPR_LIST)
			return "arr";

		return to_string(n);
	}

	// Given bytes convert to num
	inline int str_to_int(char *re, int &index)
	{
		if ((unsigned int)re[index] <= 127)
		{
			index++;
			return (int)re[index - 1];
		}
		else if ((unsigned int)re[index + 1] <= 127)
		{
			index += 2;
			return (int)(((unsigned char)re[index - 2] - 127) * 128 + (int)re[index - 1]);
		}
		else if ((unsigned int)re[index + 2] <= 127)
		{
			index += 3;
			return (int)(((unsigned char)re[index - 3] - 127) * 16384 + ((unsigned char)re[index - 2] - 127) * 128 + (int)re[index - 1]);
		}
		else if ((unsigned int)re[index + 3] <= 127)
		{
			index += 4;
			return (int)(((unsigned char)re[index - 4] - 127) * 2097152 + ((unsigned char)re[index - 3] - 127) * 16384 + ((unsigned char)re[index - 2] - 127) * 128 + (int)re[index - 1]);
		}
		else
		{
			err("str_to_int:: Overflow (value exceeds 4 bytes)");
			return -1;
		}
	}

	// Given bytes convert to num
	int str_to_int(string &re, int &index)
	{
		if ((unsigned int)re[index] <= 127)
		{
			index++;
			return (int)re[index - 1];
		}
		else if ((unsigned int)re[index + 1] <= 127)
		{
			index += 2;
			return (int)(((unsigned char)re[index - 2] - 127) * 128 + (int)re[index - 1]);
		}
		else if ((unsigned int)re[index + 2] <= 127)
		{
			index += 3;
			return (int)(((unsigned char)re[index - 3] - 127) * 16384 + ((unsigned char)re[index - 2] - 127) * 128 + (int)re[index - 1]);
		}
		else if ((unsigned int)re[index + 3] <= 127)
		{
			index += 4;
			return (int)(((unsigned char)re[index - 4] - 127) * 2097152 + ((unsigned char)re[index - 3] - 127) * 16384 + ((unsigned char)re[index - 2] - 127) * 128 + (int)re[index - 1]);
		}
		else
		{
			err("str_to_int:: Overflow (value exceeds 4 bytes)");
			return -1;
		}
	}

	int str_to_int(string re, int index, bool n) { return str_to_int(re, index); }

	// Given num convert to bytes
	inline string int_to_str(int num)
	{
		string res;
		if (num <= 127)
		{
			res = "0";
			res[0] = (unsigned char)num;
		}
		else if (num <= (255 - 127) * 128 + 127)
		{
			res = "00";
			res[0] = (unsigned char)((num / 128) + 127);
			res[1] = (unsigned char)(num % 128);
		}
		else if (num <= (255 - 127) * 16384 + (255 - 127) * 128 + 127)
		{
			res = "000";
			int r = (int)ceil((num - 127) / (float)16384) + 126;
			res[0] = (unsigned char)r;
			res[1] = (unsigned char)(((num - ((r - 127) * 16384) - num % 128) - 0) / 128 + 127);
			res[2] = (unsigned char)(num % 128);
		}
		else if (num <= (255 - 127) * 2097152 + (255 - 127) * 16384 + (255 - 127) * 128 + 127)
		{
			int r = (int)ceil((num - 127) / (float)2097152) + 125;
			if (r - 127 == 0)
			{
				cout << "lol fix these thanks";
				exit(0);
			}
			res = int_to_str(num - ((r - 127) * 2097152));
			res = "0" + res;
			res[0] = (unsigned char)r;
		}
		else
		{
			err("int_to_str:: Overflow (value exceeds 4 bytes)");
		}
		return res;
	}
	// Find value of key in hashtable, -1 on failure
	inline int umap_find(unordered_map<string, int> &u, string &c)
	{
		unordered_map<string, int>::const_iterator got = u.find(c);
		if (got != u.end())
			return got->second;
		return -1;
	}
	// Find value of key in hashtable, 0 on failure
	inline shared_ptr<variable> umap_find(unordered_map<int, shared_ptr<variable>> &u, int &c)
	{
		unordered_map<int, shared_ptr<variable>>::const_iterator got = u.find(c);
		if (got != u.end())
			return got->second;
		return 0;
	}
	// Find value of key in hashtable, 0 on failure
	inline VScope *umap_find(unordered_map<int, VScope> &u, int &c)
	{
		unordered_map<int, VScope>::const_iterator got = u.find(c);
		if (got != u.end())
			return (VScope *)&(got->second);
		return 0;
	}
	// Find key by value in umap
	inline string umap_find(unordered_map<string, int> &u, int &c)
	{
		for (unordered_map<string, int>::const_iterator it = u.begin(); it != u.end(); ++it)
		{
			if (it->second == c)
				return it->first;
		}
		return "NULL";
	}
	/*void var_set_value(int& number, variable& value, Scope* scope = 0) {
		if (scope == 0) scope = &Globals;
		scope->variable_list.direct_access(number) = &value;
	}
	void var_set_var(variable& varb, variable& value, Scope* scope = 0) {
		varb = value;
	}
	variable* var_get_value(int& number, Scope* scope = 0) {
		if (scope == 0) scope = &Globals;
		variable* got = 0;
		if(number < scope->variable_list.size()) {
			got = scope->variable_list.direct_access(number);
			if (got)
				return got;
		}
		err("VAR_VALUE:: Resolving unreferenced variable " + to_string(number));
		//variable v = 0; v.type = VAR_TYPE_ERROR;
		//return std::make_shared<variable>(v);
	}*/
	// Is the string a valid number and not an operation?
	bool is_num(string in)
	{
		int i = 0;
		if ((in[0] == '-' || in[0] == '+') && in.length() > 1)
		{
			i++;
		}
		int size = in.length();
		bool dec = 0;
		for (; i < size; i++)
		{
			if (in[i] < '0' || in[i] > '9')
			{
				if (in[i] == '.' && !dec)
				{
					dec = true;
				}
				else
				{
					return false;
				}
			}
		}
		return true;
	}

	char OPR_PRES[64] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	inline void make_opr_precedence()
	{
		int s = SKIP_KEYW;
		OPR_PRES[OPR_INCR - s] = OPR_PRES[OPR_DECR - s] = 2;
		OPR_PRES[OPR_MUL - s] = OPR_PRES[OPR_DIV - s] = OPR_PRES[OPR_MOD - s] = 5;
		OPR_PRES[OPR_ADD - s] = OPR_PRES[OPR_SUB - s] = 6;
		OPR_PRES[OPR_LESS - s] = OPR_PRES[OPR_LESS_EQ - s] = OPR_PRES[OPR_GREATER - s] = OPR_PRES[OPR_GREATER_EQ - s] = 9;
		OPR_PRES[OPR_COMPARE - s] = OPR_PRES[OPR_COMPARE_STR - s] = OPR_PRES[OPR_NOT_EQ - s] = 10;
		OPR_PRES[OPR_AND - s] = 14;
		OPR_PRES[OPR_OR - s] = 15;
		OPR_PRES[OPR_SQUARE_BKT_OPN - s] = OPR_PRES[OPR_SQUARE_BKT_CLS - s] = 15;
		OPR_PRES[OPR_EQUAL - s] = OPR_PRES[OPR_MUL_EQ - s] = OPR_PRES[OPR_DIV_EQ - s] = OPR_PRES[OPR_MOD_EQ - s] = OPR_PRES[OPR_ADD_EQ - s] = OPR_PRES[OPR_SUB_EQ - s] = 16;
		OPR_PRES[OPR_COMMA - s] = 17;
		OPR_PRES[OPR_ROUND_BKT_OPN - s] = OPR_PRES[OPR_ROUND_BKT_CLS - s] = 18; // BREAK EXP EVAL ON ()
		OPR_PRES[OPR_LIST - s] = 18;
	}

	inline int opr_precedence(int &op)
	{
		return (int)OPR_PRES[op - SKIP_KEYW];
	}

	class Intermediate
	{
		string code;
		vector<TOKEN> tree;
		int pointer = 0;
		// Return # of next
		int identify(int &pointer)
		{
			if (pointer < tree.size())
			{
				return umap_find(SL_LANG_KEYS, tree[pointer++].value);
			}
			pointer++;
			return -2;
		}
		// Return # of next, without incrementing
		int identify(int pointer, bool dummy) { return identify(pointer); }
		// Return # of next OR display error
		int identify_expectation(int &pointer, int expectation)
		{
			int res = identify(pointer);
			if (res == expectation)
				return res;
			err("Intermediate::convert Expecting " + int_to_human(expectation) + " on line " + to_string(tree[pointer - 1].line) + " (got " + tree[pointer - 1].value + ")");
			return -2;
		}
		// Solve variables
		string var_ref_solver(string name, VScope *scope = 0)
		{
			string ret = "";
			// Find variable number, if not, give it one
			int find_var = umap_find(scope->real_variable_list, name);
			if (find_var == -1)
			{
				find_var = scope->real_variable_id;
				scope->real_variable_list[name] = scope->real_variable_id++;
			}
			ret = int_to_str(OPR_VAR) + int_to_str(find_var);
			return ret;
		}
		// Solve variables
		string var_ref_solver(int &pointer, VScope *scope = 0)
		{
			// No scope? It is Global
			if (scope == 0)
				scope = &VGlobals;
			// Is there even some data?
			if (pointer >= tree.size())
			{
				err("Intermediate::var_ref_solver Expecting identifier on line " + to_string(tree[pointer - 1].line));
			}
			string var_iden = tree[pointer].value;
			string ret = "";
			// Find variable number, if not, give it one
			int find_var = umap_find(scope->real_variable_list, var_iden);
			if (find_var == -1)
			{
				if (tree[pointer].token != TOKENS::IDENTIFIER)
				{
					err("Intermediate::var_ref_solver Expecting identifier on line " + to_string(tree[pointer].line));
				}
				find_var = scope->real_variable_id;
				scope->real_variable_list[var_iden] = scope->real_variable_id++;
			}
			ret = int_to_str(OPR_VAR) + int_to_str(find_var);
			pointer++;
			return ret;
		}
		// Make a constant
		string const_find_make(string value, CScope *scope = 0, bool fix = false)
		{
			if (scope == 0)
				scope = &CGlobals;
			int find_cns = umap_find(scope->real_constants_list, value);
			if (find_cns == -1)
			{
				scope->real_constants_list[value] = scope->constants_list_id++;
				scope->constants_list[scope->constants_list_id - 1] = new variable(value, fix);
				find_cns = scope->constants_list_id - 1;
			}
			return int_to_str(OPR_CNST) + int_to_str(find_cns);
		}
		// Give null
		string const_find_null(CScope *scope = 0)
		{
			if (scope == 0)
				scope = &CGlobals;
			return int_to_str(OPR_CNST) + int_to_str(1);
		}
		// Give error
		string const_find_error(CScope *scope = 0)
		{
			if (scope == 0)
				scope = &CGlobals;
			return int_to_str(OPR_CNST) + int_to_str(2);
		}
		// Solve constants
		string const_ref_solver(int &pointer, CScope *scope = 0)
		{
			if (tree[pointer].token == TOKENS::STRING)
				return const_find_make(tree[pointer++].value, 0, false);
			if (tree[pointer].token == TOKENS::NUMERIC)
				return const_find_make(tree[pointer++].value, 0, true);
			err("const_ref_solver:: Expecting NUMERIC or STRING on line " + to_string(tree[pointer].line) + " (got " + tree[pointer].value + ")");
			return "";
		}
		string const_ref_solver_f(int &pointer, CScope *scope = 0)
		{
			if (tree[pointer].token == TOKENS::LITERAL)
				return "";
			return const_ref_solver(pointer, scope);
		}
		// Create function
		int func_ref_create(int pointer, unsigned long long where, FScope *scope = 0)
		{
			if (scope == 0)
				scope = &FGlobals;
			int find_func = umap_find(scope->real_functions_list, tree[pointer].value);
			if (find_func == -1)
			{
				find_func = scope->real_functions_id;
				scope->functions_list[find_func] = where;
				scope->real_functions_list[tree[pointer].value] = scope->real_functions_id++;
			}
			return find_func;
		}
		// Solve functions
		string func_ref_solver(int &pointer, FScope *scope = 0)
		{
			if (tree[pointer].token == TOKENS::LITERAL)
			{
				if (scope == 0)
					scope = &FGlobals;
				int find_func = umap_find(scope->real_functions_list, tree[pointer].value);
				if (find_func == -1)
				{
					// Temporarily commented the line below
					//int find_func = umap_find(builtin_functions_list, tree[pointer].value);
					if (find_func == -1)
					{
						find_func = scope->real_functions_id;
						scope->real_functions_list[tree[pointer].value] = scope->real_functions_id++;
					}
				}
				return int_to_str(OPR_FUNC_OPN) + int_to_str(find_func);
			}
			err("func_ref_solver:: Expecting LITERAL on line " + tree[pointer].line);
			return "";
		}
		// If it is supposed to be a function, do the stuff
		inline bool function_fix(int &pointer, string &output, VScope *scope)
		{
			if (pointer + 1 < tree.size() && tree[pointer + 1].value == "(")
			{
				output += func_ref_solver(pointer);
				string parameter_code;
				pointer += expression_converter_sub(pointer + 1, parameter_code, scope);
				if (parameter_code.length() == 0)
					parameter_code += const_find_error();
				// Make sure that parameter list is a tuple
				bool confirm = false;
				int tmp = 0;
				while (tmp < parameter_code.length())
				{
					if (str_to_int(parameter_code, tmp) == OPR_COMMA)
						confirm = true;
					else
						confirm = false;
				}
				// Not tuple
				if (confirm == false)
				{
					parameter_code += (const_find_error() + int_to_str(OPR_COMMA));
				}
				output += parameter_code;
				output += int_to_str(KEYW_ENDL);
				return true;
			}
			return false;
		}
		// Solve [, ] separately for arrays
		void array_fix(int &pointer, string &output, VScope *scope = 0)
		{
			if (tree[pointer].value != "[")
				err("Expression Evaluator Arr:: Expecting [ at line " + to_string(tree[pointer].line));
			int no_of_bkt_close = 1, start = pointer, arrlen = 1;
			Intermediate i;
			i.pointer = 0;
			i.code = "";
			// HANDLE [] (Empty array) (TODO: Stop creating array with null on empty arrays)
			if (start + 1 < tree.size() && tree[start + 1].value == "]")
			{
				i.tree.insert(i.tree.begin(), TOKEN{LITERAL, "null", tree[start].line});
				arrlen = 0;
			}
			while (no_of_bkt_close && ++pointer < tree.size())
			{
				if (tree[pointer].value == "[")
					no_of_bkt_close++;
				else if (tree[pointer].value == "]")
					no_of_bkt_close--;
				if (no_of_bkt_close)
				{
					i.tree.push_back(tree[pointer]);
					if (no_of_bkt_close == 1 && tree[pointer].value == ",")
						arrlen++;
					// TODO: Currently overestimating (wrong answer if function calls are inside array)
				}
			}
			i.tree.insert(i.tree.begin(), TOKEN{OPERATOR, "~array", tree[start].line});
			i.tree.insert(i.tree.begin(), TOKEN{NUMERIC, to_string(arrlen), tree[start].line});
			if (no_of_bkt_close != 0)
			{
				err("Expression Evaluator Arr:: Expecting ] at line " + to_string(tree[pointer - 1].line));
			}
			i.expression_converter(i.pointer, i.code, scope);
			output += i.code;
			pointer++;
		}
		// Solve [] operator by taking index operator as ()
		int array_index_opr_fix(int &pointer, string &output, VScope *scope = 0)
		{
			if (tree[pointer].value != "[")
				err("Expression Evaluator Arr:: Expecting [ at line " + to_string(tree[pointer].line) + " got " + tree[pointer].value);
			Intermediate i;
			i.pointer = 0;
			i.code = "";
			i.tree.push_back({TOKENS::PAREN_O, "(", tree[pointer].line});
			int no_of_bkt_close = 1;

			pointer++;
			// Push sub-expression in i and evaluate it there
			while (no_of_bkt_close != 0 && pointer < tree.size())
			{
				int try_find_next = identify(pointer);
				if (try_find_next == OPR_SQUARE_BKT_OPN)
					no_of_bkt_close++;
				else if (try_find_next == OPR_SQUARE_BKT_CLS)
					no_of_bkt_close--;
				i.tree.push_back(tree[pointer - 1]);
			}
			i.tree[i.tree.size() - 1] = {TOKENS::PAREN_C, ")", tree[pointer - 1].line};
			i.expression_converter(i.pointer, i.code, scope);

			if (no_of_bkt_close != 0)
			{
				err("Expression Evaluator:: unbalanced at line " + to_string(tree[pointer - 1].line));
			}
			else
			{
				output += (i.code + int_to_str(OPR_SQUARE_BKT_CLS));
			}
			return i.pointer;
		}
		// Evaluate till next )
		int expression_converter_sub(int pointer, string &output, VScope *scope = 0)
		{
			if (tree[pointer].value != "(")
				err("Expression Evaluator Sub:: Expecting ( at line " + to_string(tree[pointer].line));
			Intermediate i;
			i.pointer = 0;
			i.code = "";
			i.tree.push_back(tree[pointer]);
			int no_of_bkt_close = 1;

			pointer++;
			// Push sub-expression in i and evaluate it there
			while (no_of_bkt_close != 0 && pointer < tree.size())
			{
				int try_find_next = identify(pointer);
				if (try_find_next == OPR_ROUND_BKT_OPN)
					no_of_bkt_close++;
				else if (try_find_next == OPR_ROUND_BKT_CLS)
					no_of_bkt_close--;
				i.tree.push_back(tree[pointer - 1]);
			}

			i.expression_converter(i.pointer, i.code, scope);

			if (no_of_bkt_close != 0)
			{
				err("Expression Evaluator:: unbalanced at line " + to_string(tree[pointer - 1].line));
			}
			else
			{
				output += i.code;
			}
			return i.pointer;
		}
		// Add a resolved constant value (int_to_str'd) to expression converted from string
		string dtype_to_str_cnst(string n, bool fix = false)
		{
			if (n == "")
				err("Expression Evaluator:: Expected a constant");
			string cns = const_find_make(n, 0, fix);
			return cns;
			//return int_to_str(OPR_CNST_RES) + int_to_str(n.length()) + n;
		}
		void expression_converter_start(int &pointer, string &output, VScope *scope = 0)
		{
			// -[SOMETHING] ++[SOMETHING] --[SOMETHING] ![SOMETHING]
			if (pointer < tree.size() && (tree[pointer].value == "+" || tree[pointer].value == "++" ||
										  tree[pointer].value == "-" || tree[pointer].value == "--" || tree[pointer].value == "!"))
			{
				int symbol = identify(pointer);
				if (symbol == OPR_NOT)
					symbol = OPR_NOT_EQ;
				int try_find_next = identify(pointer);

				// ++/-- on variables only
				if (symbol == OPR_INCR || symbol == OPR_DECR)
				{
					if (try_find_next != OPR_VAR)
						err("Expression Evaluator:: ++/-- on non-variable");
					else
						output += (const_find_null() + var_ref_solver(pointer) + int_to_str(symbol));
				}

				// [SOMETHING] = Variable
				else if (try_find_next == OPR_VAR)
				{
					output += (dtype_to_str_cnst("0", true) + var_ref_solver(pointer) + int_to_str(symbol));
				}
				// -[SOMETHING] = -(Expression)
				else if (try_find_next == OPR_ROUND_BKT_OPN)
				{
					output += dtype_to_str_cnst("0", true);
					pointer += expression_converter_sub(pointer - 1, output, scope) - 2;
					output += int_to_str(symbol);
				}
				// [SOMETHING] = CONSTANT
				else if (tree[pointer - 1].token == TOKENS::NUMERIC)
				{
					output += (dtype_to_str_cnst("0", true) + const_find_make(tree[pointer - 1].value, 0, true) + int_to_str(symbol));
				}
				// FUNCTIONS
				else if (tree[pointer - 1].token == TOKENS::LITERAL)
				{
					string tmpoutput;
					pointer--;
					if (!function_fix(pointer, tmpoutput, scope))
					{
						pointer++;
						err("Expression Evaluator:: unexpected constant at line " + to_string(tree[pointer - 1].line));
					}
					output += dtype_to_str_cnst("0", true);
					output += tmpoutput;
					output += int_to_str(symbol);
				}
				// +/- on Strings???
				else
				{
					err("Expression Evaluator:: invalid operation at line " + to_string(tree[pointer - 1].line));
				}
			}
			// Handle array creation
			else if (pointer < tree.size() && tree[pointer].value == "[" && tree[pointer - 1].value != "]")
				array_fix(pointer, output, scope);
		}
		// Convert MATHS and LOGIC (Infix -> Postfix)
		void expression_converter(int &pointer, string &output, VScope *scope = 0)
		{
			stack<string> st;
			int try_find = identify(pointer);
			// Remove + - sign at start, do rest separately, fix afterwards (EVERYTHING_ELSE 0 -)
			if (try_find == OPR_ADD || try_find == OPR_SUB || try_find == OPR_NOT ||
				try_find == OPR_INCR || try_find == OPR_DECR)
			{
				if (try_find == OPR_NOT)
					try_find = OPR_NOT_EQ;
				if (pointer >= tree.size())
					err("Expression Evaluator:: Expected an expression");
				string const_val = dtype_to_str_cnst("0", true);

				// ++/-- on variables only
				if (try_find == OPR_INCR || try_find == OPR_DECR)
				{
					if (identify(pointer, false) != OPR_VAR)
						err("Expression Evaluator:: ++/-- on non-variable");
					else
						const_val = const_find_null();
					output += const_val;
					//output += int_to_str(try_find); //RESULTS IN ++ ++??
					//expression_converter(pointer, output, scope); idk, was this ever tested?
				}
				else
				{
					output += const_val;
					//expression_converter(pointer, output, scope); idk, was this ever tested?
					//output += int_to_str(try_find); //RESULTS IN ++ ++??
				}
				expression_converter_start(pointer, output, scope);
			}
			// [
			else if (try_find == OPR_SQUARE_BKT_OPN)
			{
				array_fix(--pointer, output, scope);
				try_find = identify(pointer);
			}

			// Solve exp till ;
			while (try_find != -2 && try_find != OPR_SEMICLN)
			{
				// $
				if (try_find == OPR_VAR)
				{

					output += var_ref_solver(pointer, scope);

					// [variable++]?
					int val = identify(pointer, false);
					if (val == OPR_INCR || val == OPR_DECR)
					{
						pointer++;
						output += const_find_null() + int_to_str(val);
					}
				}
				// NUMERIC STRING LITERAL UNIDENTIFIED
				else if (try_find == -1)
				{
					pointer--;
					string ref = const_ref_solver_f(pointer);
					// The result wasn't a string/number, maybe it is a function?
					if (ref == "")
					{
						// Do function fix if possible or stop
						if (!function_fix(pointer, output, scope))
							err("Expression Evaluator:: unexpected " + tree[pointer].value + " at line " + to_string(tree[pointer - 1].line));
					}
					// The result was a string/number, OK
					else
						output += ref;
				}
				// (
				else if (try_find == OPR_ROUND_BKT_OPN)
				{
					st.push(int_to_str(OPR_ROUND_BKT_OPN));
					expression_converter_start(pointer, output, scope);
				}
				// )
				else if (try_find == OPR_ROUND_BKT_CLS)
				{
					if (st.empty())
					{
						err("Expression Evaluator:: unexpected ) at line " + to_string(tree[pointer - 1].line));
					}
					while (!st.empty() && st.top() != int_to_str(OPR_ROUND_BKT_OPN))
					{
						output += st.top();
						st.pop();
					}
					if (!st.empty())
					{
						if (st.top() == int_to_str(OPR_ROUND_BKT_OPN))
							st.pop();
					}
					else
					{
						err("Expression Evaluator:: unexpected ) at line " + to_string(tree[pointer - 1].line));
					}
				}
				// [ => treat like ()
				else if (try_find == OPR_SQUARE_BKT_OPN)
					array_index_opr_fix(--pointer, output, scope);
				// , to --
				else if (try_find >= OPR_STRT && try_find <= OPR_END)
				{
					int tmpn = 0;
					int tmpo = 0;
					if (!st.empty())
						tmpn = str_to_int(st.top(), 0, true);
					while (!st.empty() && opr_precedence(try_find) >= opr_precedence(tmpn))
					{
						output += st.top();
						st.pop();
						if (!st.empty())
							tmpn = str_to_int(st.top(), 0, true);
					}
					// Spit out arr index operator (]) asap instead of keeping it in stack
					if (try_find == OPR_SQUARE_BKT_CLS)
						output += int_to_str(try_find);
					else
						st.push(int_to_str(try_find));

					expression_converter_start(pointer, output, scope);
				}
				// null
				else if (try_find == KEYW_NULL)
					st.push(const_find_null());
				// Something illegal was found
				else
				{
					err("Expression Evaluator:: did not expect " + tree[pointer - 1].value + " at line " + to_string(tree[pointer - 1].line));
				}
				try_find = identify(pointer);
			}
			while (!st.empty())
			{
				output += st.top();
				st.pop();
			}
		}
		// Place code in output, move pointer ahead,
		void convert(int &pointer, string &output, bool early_return = false, VScope *scope = 0)
		{
			int try_find = identify(pointer);
			while (try_find != -2)
			{
				//	FOR
				if (try_find == KEYW_FOR)
				{
					//	(
					int line = pointer - 1;
					try_find = identify_expectation(pointer, OPR_ROUND_BKT_OPN);
					// INIT
					string init;
					int line2 = pointer;
					expression_converter(pointer, init, scope);
					if (init.length() > 0)
						output += int_to_str(tree[line2].line) + init + int_to_str(KEYW_ENDL);
					// CONDITION
					string while_condition;
					expression_converter(pointer, while_condition, scope);
					if (while_condition.length() == 0)
						while_condition = const_find_make("1", 0, true);
					while_condition += int_to_str(KEYW_ENDL);
					// OPERATION
					pointer--;
					tree[pointer].value = "(";
					int operation_line = pointer;
					string while_operation;
					pointer += expression_converter_sub(pointer, while_operation, scope);
					if (while_operation.length() > 0)
						while_operation = int_to_str(tree[operation_line].line) + while_operation + int_to_str(KEYW_ENDL);
					pointer -= 2;

					//	)
					try_find = identify_expectation(pointer, OPR_ROUND_BKT_CLS);
					// {
					try_find = identify_expectation(pointer, OPR_FLOWER_BKT_OPN);
					// INSIDE CODE
					string inside_code;
					convert(pointer, inside_code, false, scope);
					int bytes_fwd = while_condition.length() + while_operation.length() + inside_code.length();
					bytes_fwd += int_to_str(tree[pointer - 1].line).length() + int_to_str(OPR_GO_BCK).length();
					bytes_fwd += int_to_str(bytes_fwd).length() + 2;

					output += int_to_str(tree[line].line) + int_to_str(KEYW_IF) + int_to_str(bytes_fwd) + while_condition;
					output += inside_code;
					output += while_operation;
					output += int_to_str(tree[pointer - 1].line) + int_to_str(OPR_GO_BCK) + int_to_str(bytes_fwd) + int_to_str(KEYW_ENDL);
				}
				//	WHILE
				else if (try_find == KEYW_WHILE)
				{
					// (
					int line = pointer - 1;
					try_find = identify_expectation(pointer, OPR_ROUND_BKT_OPN);
					// CONDITION
					pointer--;
					int condition_line = pointer;
					string while_condition;
					pointer += expression_converter_sub(pointer, while_condition, scope);
					if (while_condition.length() == 0)
						while_condition = const_find_make("1", 0, true);
					while_condition += int_to_str(KEYW_ENDL);
					pointer -= 2;

					// )
					try_find = identify_expectation(pointer, OPR_ROUND_BKT_CLS);
					// {
					try_find = identify_expectation(pointer, OPR_FLOWER_BKT_OPN);
					// INSIDE CODE
					string inside_code;
					convert(pointer, inside_code, false, scope);
					int bytes_fwd = while_condition.length() + inside_code.length();
					bytes_fwd += int_to_str(tree[pointer - 1].line).length() + int_to_str(OPR_GO_BCK).length(); // +int_to_str(KEYW_ENDL).length();
					bytes_fwd += int_to_str(bytes_fwd).length() + 1 + 1;										// ASSUMING it's gonna be 2 (no choice)

					output += int_to_str(tree[line].line) + int_to_str(KEYW_IF) + int_to_str(bytes_fwd) + while_condition;
					output += inside_code;
					output += int_to_str(tree[pointer - 1].line) + int_to_str(OPR_GO_BCK) + int_to_str(bytes_fwd + int_to_str(bytes_fwd).length() - 1) + int_to_str(KEYW_ENDL);
				}
				//	IF
				else if (try_find == KEYW_IF)
				{
					//	(
					int line = pointer - 1;
					try_find = identify_expectation(pointer, OPR_ROUND_BKT_OPN);
					// CONDITION
					pointer--;
					int condition_line = pointer;
					string if_condition;
					pointer += expression_converter_sub(pointer, if_condition, scope);
					if (if_condition.length() == 0)
						if_condition = const_find_make("1", 0, true);
					if_condition += int_to_str(KEYW_ENDL);
					pointer -= 2;

					//	)
					try_find = identify_expectation(pointer, OPR_ROUND_BKT_CLS);
					// {
					try_find = identify_expectation(pointer, OPR_FLOWER_BKT_OPN);
					// INSIDE CODE
					string inside_code;
					convert(pointer, inside_code, false, scope);
					int bytes_fwd = if_condition.length() + inside_code.length();
					// ELSE? ELSE IF?
					int else_pointer = pointer;
					bool if_chain = true;
					try_find = identify(pointer);
					string else_code;
					if (try_find == KEYW_ELSE)
					{
						try_find = identify(pointer);
						// ELSE IF
						if (try_find == KEYW_IF)
						{
							// GO SOLVE IF SEPARATELY (AND THE ELSE IF'S THAT FOLLOW), do early return (JUST if statement, no more)
							pointer--;
							convert(pointer, else_code, true, scope);
						}
						// ELSE
						else if (try_find == OPR_FLOWER_BKT_OPN)
						{
							convert(pointer, else_code, false, scope);
						}
						else
							err("Intermediate:: Expecting { or IF after ELSE on line " + tree[pointer - 1].line);
					}
					// Nothing after the if statement
					else
					{
						if_chain = false;
						pointer = else_pointer;
					}
					string else_code_skip;
					if (if_chain)
					{
						else_code_skip = int_to_str(tree[else_pointer].line) + int_to_str(OPR_GO_FWD) + int_to_str(else_code.length() + int_to_str(KEYW_ENDL).length()) + int_to_str(KEYW_ENDL);
						bytes_fwd += else_code_skip.length();
					}
					else
						else_code_skip = "";

					output += int_to_str(tree[line].line) + int_to_str(KEYW_IF) + int_to_str(bytes_fwd + 1) + if_condition;
					output += inside_code;
					output += else_code_skip;
					output += else_code;

					if (early_return)
						return;
					// Early return is done when an if statement calls a sub-query just to resolve else-if statements, recursively fixing it all
				}
				//	FUNCTION
				else if (try_find == KEYW_FUNCTION)
				{
					// NAME
					int func_no = 0;
					if (tree[pointer++].token == LITERAL)
					{
						if (umap_find(FGlobals.real_functions_list, tree[pointer].value) == -1)
						{
							func_no = func_ref_create(pointer - 1, output.length());
							output += int_to_str(tree[pointer - 1].line) + int_to_str(KEYW_FUNCTION);
						}
						else
							err("Intermediate::convert Function redefinition on line " + to_string(tree[pointer - 1].line));
					}
					else
						err("Intermediate::convert Expecting function name on line " + to_string(tree[pointer - 1].line) + " (got " + tree[pointer - 1].value + ")");
					// Function VScope to resolve variables
					//FGlobals.real_functions_vscope[output.length()] = VScope(8);
					VScope *temp = &FGlobals.real_functions_vscope[output.length()];
					//	(
					int line = pointer - 1;
					try_find = identify_expectation(pointer, OPR_ROUND_BKT_OPN);
					// PARAMETERS
					pointer--;
					int condition_line = pointer;
					string parameter_code;
					pointer += expression_converter_sub(pointer, parameter_code, temp);
					//int tmpp = 0; debug_single_line(parameter_code, tmpp, false);
					if (parameter_code.length() != 0)
					{
						// Make sure that parameter list is a tuple
						bool confirm = false;
						int tmp = 0;
						while (tmp < parameter_code.length())
						{
							if (str_to_int(parameter_code, tmp) == OPR_COMMA)
								confirm = true;
							else
								confirm = false;
						}
						// No?
						if (confirm == false)
						{
							parameter_code += (const_find_null() + int_to_str(OPR_COMMA));
						}
					}
					parameter_code += int_to_str(KEYW_ENDL);
					pointer -= 2;
					// )
					try_find = identify_expectation(pointer, OPR_ROUND_BKT_CLS);
					// {
					try_find = identify_expectation(pointer, OPR_FLOWER_BKT_OPN);
					string inside_code;
					convert(pointer, inside_code, false, temp);
					// Function Length
					output += int_to_str(parameter_code.length() + inside_code.length());
					output += parameter_code;
					output += inside_code;
				}
				// return
				else if (try_find == KEYW_RETURN)
				{
					output += int_to_str(tree[pointer - 1].line);
					output += int_to_str(KEYW_RETURN);
					expression_converter(pointer, output, scope);
					output += int_to_str(KEYW_ENDL);
				}
				// } => return
				else if (try_find == OPR_FLOWER_BKT_CLS)
				{
					return;
				}
				// probably maths, go try
				else
				{
					output += int_to_str(tree[--pointer].line);
					expression_converter(pointer, output, scope);
					output += int_to_str(KEYW_ENDL);
				}
				try_find = identify(pointer);
			}
		}
		// Display single line of intermediate code
		VScope *vscope = &VGlobals;
		int vscope_end = -1;
		void debug_single_line(string &code, int &pointer, bool var_resolve = false)
		{
			cout << "#" << str_to_int(code, pointer) << ": ";
			bool func_open = false;
			if (vscope_end != -1 && pointer >= vscope_end)
			{
				vscope_end = -1;
				vscope = &VGlobals;
			}
			while (pointer < code.length())
			{
				int val = str_to_int(code, pointer);

				// SPECIAL, attached to next value
				if (val == OPR_VAR || val == OPR_CNST || val == OPR_FUNC_OPN || val == OPR_CNST_RES || val == OPR_GO_FWD ||
					val == OPR_GO_BCK || val == KEYW_WHILE || val == KEYW_IF || val == KEYW_FUNCTION)
				{
					// If it is variable and resolve mode is on, find human name and display that
					if (var_resolve && (val == OPR_VAR || val == OPR_CNST || val == OPR_FUNC_OPN))
					{
						int n = str_to_int(code, pointer);
						if (val == OPR_VAR)
							cout << "$" << umap_find(vscope->real_variable_list, n) << " ";
						if (val == OPR_CNST)
						{
							if (CGlobals.constants_list[n]->type == VAR_TYPE_STRING)
							{
								cout << '"';
								CGlobals.constants_list[n]->display();
								cout << '"';
							}
							else
								CGlobals.constants_list[n]->display();
							cout << " ";
						}
						if (val == OPR_FUNC_OPN)
						{
							func_open = true;
							cout << umap_find(FGlobals.real_functions_list, n) << "( ";
						}
					}
					else
					{
						cout << int_to_human(val) << " ";
						if (val == OPR_FUNC_OPN)
						{
							func_open = true;
						}
						if (val == OPR_GO_FWD || val == KEYW_IF || val == KEYW_FUNCTION)
						{
							int temp = pointer;
							if (val == KEYW_FUNCTION)
							{
								for (unsigned int i = 0; i < FGlobals.functions_list.size(); i++)
								{
									if (FGlobals.functions_list[i] == temp)
										temp = FGlobals.functions_list[i];
								}
								cout << umap_find(FGlobals.real_functions_list, temp) << " ";
							}
							cout << "(#";
							temp = pointer;
							int pos = str_to_int(code, temp);
							if (val == KEYW_IF)
								temp = (temp - pointer) + pointer + pos - 1;
							else
								temp = (temp - pointer) + pointer + pos;
							cout << str_to_int(code, temp);
							cout << ") ";
							if (val == KEYW_FUNCTION)
							{
								vscope_end = temp;
								temp = pointer;
								vscope = umap_find(FGlobals.real_functions_vscope, temp);
								if (vscope == 0)
									vscope = &VGlobals;
							}
						}
						else if (val == OPR_GO_BCK)
						{
							cout << "(#";
							int temp = pointer;
							int pos = str_to_int(code, temp);
							temp = pointer - pos;
							cout << str_to_int(code, temp);
							cout << ") ";
						}
						if (val == OPR_CNST_RES)
						{
							int n = str_to_int(code, pointer);
							cout << "(" << n << ") " << str_to_int(code, pointer) << " ";
						}
						else
							cout << str_to_int(code, pointer) << " ";
					}
				}
				else if (val == KEYW_ENDL)
				{
					if (func_open)
					{
						cout << "\\n) ";
						func_open = false;
						continue;
					}
					cout << "\\n\n";
					return;
				}
				// NOT special
				else
				{
					cout << int_to_human(val) << " ";
				}
			}
		}

	public:
		Intermediate(bool setup = false)
		{
			if (setup == true)
				make_opr_precedence();
		}
		string intermediate(vector<TOKEN> &t)
		{
			code = "";
			tree = t;
			pointer = 0;

			convert(pointer, code);
			CGlobals.real_constants_list.clear(); // Free unnecessary constants memory

			return code;
		}
		void display(string &code, bool var_resolve = false)
		{
			int pointer = 0;
			cout << "Code length: " << code.length() << ", tree length: " << tree.size() << "\n";
			cout << "VARIABLES:\n";
			unordered_map<string, int>::iterator it;
			for (it = VGlobals.real_variable_list.begin(); it != VGlobals.real_variable_list.end(); it++)
			{
				cout << it->second << ":\t" << it->first << "\n";
			}
			cout << "\nCONSTANTS:\n";
			int sz = CGlobals.constants_list.size();
			for (int i = 0; i < sz; i++)
			{
				variable *tmp = CGlobals.constants_list.direct_access(i);
				if (!tmp)
					continue;
				cout << i << " [" << (int)tmp->type << "]:\t";
				tmp->display();
				cout << "\n";
			}
			cout << "\nFUNCTIONS:\n";
			for (it = FGlobals.real_functions_list.begin(); it != FGlobals.real_functions_list.end(); it++)
			{
				cout << it->second << ":\t" << it->first << "\n";
			}
			cout << "\n";
			while (pointer < code.length())
			{
				debug_single_line(code, pointer, var_resolve);
			}
		}
	};

	class Runtime
	{
		char *code;
		int line;
		//heap_manager<variable> hp;
		variable st[128]; // The evaluation stack
		int stp;		  // Stack position

		inline variable type_resolver(int &pointer, VScope *scope)
		{
			int next = str_to_int(code, pointer);
			if (next == OPR_CNST)
			{
				next = str_to_int(code, pointer);
				return *CGlobals.constants_list.direct_access(next); // Copy value
			}
			else if (next == OPR_VAR)
			{
				int var_no = str_to_int(code, pointer);

				variable *to_push = scope->variable_list[var_no];
				if (!to_push)
					scope->variable_list.direct_access(var_no) = to_push = new variable();
				return to_push->ref(); // Create reference
			}
			else if (next == OPR_FUNC_OPN)
			{
				int old_line = line, new_pointer, pointer_to, o_stp = stp;
				VScope *function_scope = MGlobals.vscope_heap.get();
				for (int i = 0; i < o_stp; i++)
				{
					MGlobals.vstack[MGlobals.vstack_count++] = std::move(st[i]);
				}
				variable rval(0);
				rval.type = VAR_TYPE_NULL;
				{
					int func_no = str_to_int(code, pointer);
					expression_evaluator(pointer, scope);
					variable right_list = std::move(st[stp]);

					new_pointer = (int)FGlobals.functions_list[func_no];
					line = str_to_int(code, new_pointer);
					int operation = str_to_int(code, new_pointer);
#ifdef DEBUG
					if (operation != KEYW_FUNCTION)
						err("Type_resolver:: Expecting a function but got something else at line " + line);
#endif
					// Solve function parameters
					pointer_to = new_pointer + str_to_int(code, new_pointer);
					expression_evaluator(new_pointer, function_scope);
					variable left_list = std::move(st[stp]);
					left_list.tuple_declare(right_list);
				}
				while (new_pointer < pointer_to)
				{
					int pos = new_pointer;
					str_to_int(code, new_pointer);
					if (str_to_int(code, new_pointer) == KEYW_RETURN)
					{
						expression_evaluator(new_pointer, function_scope);
						rval = st[stp].deref_move();
						break;
					}
					else
					{
						new_pointer = pos;
						run_line(new_pointer, function_scope);
					}
				}
				for (int i = o_stp - 1; i >= 0; i--)
				{
					st[i] = std::move(MGlobals.vstack.direct_access(--MGlobals.vstack_count));
				}
				MGlobals.vscope_heap.ret(function_scope);
				stp = o_stp;
				line = old_line;
				return rval;
			}
			else
			{
				err("Type_resolver:: Unknown type " + int_to_human(next));
			}
			return 0;
		}
		// If operation is valid, solve and push..
		inline variable operation(variable &operand1, variable &operand2, int &operation)
		{
			// Keep original, needed in some specific cases
			variable operand1o;
			variable operand2o;
			bool opr2wasref = false;
			// I'm a reference I need to be converted
			if (operand2.type == VAR_TYPE_VAR)
			{
				opr2wasref = true;
				operand2o = operand2.ref();
				operand2.type = operand2.data.var_data->type;
				operand2.data = operand2.data.var_data->data;
			}
			// I'm a reference I have additional needs
			if (operand1.type == VAR_TYPE_VAR)
			{
				// = operator stores value in actual variable when type is VAR_TYPE_VAR
				if (operation == OPR_EQUAL && operand2.type <= VAR_TYPE_ARRAY)
				{
					operand1.data.var_data->~variable();
					// Optimization: move value for = when RHS is created on spot
					if (opr2wasref)
					{
						(operand1 = operand2);			// Copy RHS by value
						operand2.type = VAR_TYPE_ERROR; // Don't delete, will be used
					}
					else
					{
						operand1.data.var_data->type = operand2.type;
						operand1.data.var_data->data = operand2.data; // Move RHS
						operand2.type = VAR_TYPE_ERROR;				  // Don't delete, will be used
					}
					return operand1;
				}
				variable *operandt = operand1.data.var_data;
				// Calling operator function for the pointer
				switch (operation)
				{
				case OPR_SQUARE_BKT_CLS:
				{
					return operandt->operator[](operand2)->ref();
				}
				case OPR_INCR:
				{
					if (operand2.type == VAR_TYPE_NULL)
					{
						if (operandt->type == VAR_TYPE_INT)
						{
							operandt->data.int_data++;
							variable t(1);
							return operandt->operator-(t);
						}
						else if (operandt->type == VAR_TYPE_DOUBLE)
						{
							operandt->data.double_data++;
							variable t(1);
							return operandt->operator-(t);
						}
					}
				}
				case OPR_DECR:
				{
					if (operand2.type == VAR_TYPE_NULL)
					{
						if (operandt->type == VAR_TYPE_INT)
						{
							operandt->data.int_data--;
							variable t(1);
							return operandt->operator+(t);
						}
						else if (operandt->type == VAR_TYPE_DOUBLE)
						{
							operandt->data.double_data--;
							variable t(1);
							return operandt->operator+(t);
						}
					}
				}
				case OPR_ADD_EQ:
				{
					operandt->operator+=(operand2);
					return operand1;
				}
				case OPR_SUB_EQ:
				{
					operandt->operator-=(operand2);
					return operand1;
				}
				case OPR_MUL_EQ:
				{
					operandt->operator*=(operand2);
					return operand1;
				}
				case OPR_DIV_EQ:
				{
					operandt->operator/=(operand2);
					return operand1;
				}
				}
				operand1o = operand1.ref();
				operand1.type = operand1.data.var_data->type;
				operand1.data = operand1.data.var_data->data;
			}
			switch (operation)
			{
			case OPR_ADD:
			{
				return operand1 + operand2;
			}
			case OPR_SUB:
			{
				return operand1 - operand2;
			}
			case OPR_DIV:
			{
				return operand1 / operand2;
			}
			case OPR_MUL:
			{
				return operand1 * operand2;
			}
			case OPR_MOD:
			{
				return operand1 % operand2;
			}
			case OPR_LESS:
			{
				return (operand1 < operand2);
			}
			case OPR_LESS_EQ:
			{
				return (operand1 <= operand2);
			}
			case OPR_GREATER:
			{
				return (operand1 > operand2);
			}
			case OPR_GREATER_EQ:
			{
				return (operand1 >= operand2);
			}
			case OPR_COMPARE:
			{
				return (operand1 == operand2);
			}
				//case OPR_COMPARE_STR: { return (operand1.equals(operand2); }
			case OPR_NOT_EQ:
			{
				return !(operand1 == operand2);
			}
			case OPR_AND:
			{
				return (operand1 && operand2);
			}
			case OPR_OR:
			{
				return (operand1 || operand2);
			}
			case OPR_INCR:
			{
				if (operand1.type == VAR_TYPE_NULL)
				{
					if (operand2.type == VAR_TYPE_INT)
					{
						operand2o.data.var_data->data.int_data++;
						return operand2o;
					}
					else if (operand2.type == VAR_TYPE_DOUBLE)
					{
						operand2o.data.var_data->data.double_data++;
						return operand2o;
					}
				}
			}
			case OPR_DECR:
			{
				if (operand1.type == VAR_TYPE_NULL)
				{
					if (operand2.type == VAR_TYPE_INT)
					{
						operand2o.data.var_data->data.int_data--;
						return operand2o;
					}
					else if (operand2.type == VAR_TYPE_DOUBLE)
					{
						operand2o.data.var_data->data.double_data--;
						return operand2o;
					}
				}
			}
			case OPR_COMMA:
			{
				// If neither is array or both are, make array, kinda error-prone since it'll accept any coma separated values, also efficiency issues for operand2
				// EDIT: If one is array and other isn't, same code
				if ((((operand1.type != VAR_TYPE_ARRAY || operand2.type != VAR_TYPE_ARRAY) &&
					  operand1.type != VAR_TYPE_LIST && operand2.type != VAR_TYPE_LIST &&
					  operand1.type != VAR_TYPE_TUPLE && operand2.type != VAR_TYPE_TUPLE) ||
					 (operand1.type == VAR_TYPE_ARRAY && operand2.type == VAR_TYPE_ARRAY)) &&
					st[stp - 1].type == VAR_TYPE_INT)
				{
					variable ret(VAR_TYPE_LIST, st[stp - 1].data.int_data);
					// A bit tricky => array is being built, we don't wan't any re-copying (2d array) and we don't want stack to destruct, An empty
					// variable is added to array and then array from runtime stack is MOVED to that dummy location. avoids all copying and extra destruction
					variable dummy;
					ret.data.array_data->add(dummy);
					variable *t = ret.data.array_data->access(ret.data.array_data->length() - 1);
					t->data = operand1.data;
					t->type = operand1.type;
					ret.data.array_data->add(dummy);
					t = ret.data.array_data->access(ret.data.array_data->length() - 1);
					t->data = operand2.data;
					t->type = operand2.type;
					operand1.type = VAR_TYPE_ERROR;
					operand2.type = VAR_TYPE_ERROR;
					return ret;
				}
				// one isn't array, append
				else if (operand1.type == VAR_TYPE_LIST)
				{
					variable ret;
					ret.data = operand1.data;
					ret.type = VAR_TYPE_LIST;
					// Optimization: If operand 2 is generated on stack, don't do full copy
					if (opr2wasref)
						ret.data.array_data->add(operand2);
					else
					{
						variable dummy;
						ret.data.array_data->add(dummy);
						variable *t = ret.data.array_data->access(ret.data.array_data->length() - 1);
						t->data = operand2.data;
						t->type = operand2.type;
						operand2.type = VAR_TYPE_ERROR;
					}
					operand1.type = VAR_TYPE_ERROR;
					return ret;
				}
				// Append to Tuple
				else if (operand1.type == VAR_TYPE_TUPLE)
				{
					variable ret;
					ret.data = operand1.data;
					ret.type = VAR_TYPE_TUPLE;
					if (operand2o.type == VAR_TYPE_VAR)
						ret.data.array_data->add(operand2o);
					else
					{
						if (opr2wasref)
							ret.data.array_data->add(operand2);
						else
						{
							variable dummy;
							ret.data.array_data->add(dummy);
							variable *t = ret.data.array_data->access(ret.data.array_data->length() - 1);
							t->data = operand2.data;
							t->type = operand2.type;
							operand2.type = VAR_TYPE_ERROR;
						}
					}
					operand1.type = VAR_TYPE_ERROR;
					return ret;
				}
				// Not Tuple => create...
				else
				{
					variable ret(VAR_TYPE_ARRAY, 1);
					ret.type = VAR_TYPE_TUPLE;
					if (operand1o.type == VAR_TYPE_VAR)
						ret.data.array_data->add(operand1o);
					else
						ret.data.array_data->add(operand1);
					if (operand2o.type == VAR_TYPE_VAR)
						ret.data.array_data->add(operand2o);
					else
					{
						if (opr2wasref)
							ret.data.array_data->add(operand2);
						else
						{
							variable dummy;
							ret.data.array_data->add(dummy);
							variable *t = ret.data.array_data->access(ret.data.array_data->length() - 1);
							t->data = operand2.data;
							t->type = operand2.type;
							operand2.type = VAR_TYPE_ERROR;
						}
					}
					operand1.type = VAR_TYPE_ERROR;
					return ret;
				}
			}
			case OPR_LIST:
			{
				if (operand2.type == VAR_TYPE_LIST)
				{
					variable ret;
					ret.data = operand2.data;
					ret.type = VAR_TYPE_ARRAY;
					operand2.type = VAR_TYPE_ERROR;
					return ret;
				}
				else
				{
					variable ret(VAR_TYPE_ARRAY, 1);
					variable dummy;
					ret.data.array_data->add(dummy);
					variable *t = ret.data.array_data->access(0);
					t->data = operand2.data;
					t->type = operand2.type;
					operand2.type = VAR_TYPE_ERROR;
					return ret;
				}
			}
			case OPR_EQUAL:
			{
				if (operand1.type == VAR_TYPE_TUPLE && operand2.type == VAR_TYPE_TUPLE)
				{
					operand1.tuple_declare(operand2);
					return operand1;
				}
			}
			}

			if (operand1o.type != VAR_TYPE_ERROR)
				err("Postfix Evaluator:: undefined operation \"" + int_to_human(operation) + "\" between " + operand1o.var_type() + " and " + operand2.var_type() + " at line " + to_string(line));
			else if (operand2o.type != VAR_TYPE_ERROR)
				err("Postfix Evaluator:: undefined operation \"" + int_to_human(operation) + "\" between " + operand1.var_type() + " and " + operand2o.var_type() + " at line " + to_string(line));
			err("Postfix Evaluator:: undefined operation \"" + int_to_human(operation) + "\" between " + operand1.var_type() + " and " + operand2.var_type() + " at line " + to_string(line));
			return variable();
		}
		// read elements and perform postfix evaluation
		void expression_evaluator(int &pointer, VScope *scope)
		{
			stp = 0;
			int old = pointer, next = str_to_int(code, pointer);
			while (next != KEYW_ENDL)
			{
				if (opr_precedence(next))
				{
#ifdef DEBUG
					if (stp < 2)
						err("Postfix Evaluator:: unbalanced equation at line " + to_string(line));
#endif
					variable &a = st[stp];
					stp -= 1;
					variable &b = st[stp];
					st[stp] = operation(b, a, next);
				}
				else
				{
					pointer = old;
					stp += 1;
					st[stp] = type_resolver(pointer, scope);
				}
				old = pointer;
				next = str_to_int(code, pointer);
			}
#ifdef DEBUG
			if (stp < 0)
				err("Postfix Evaluator:: unbalanced > equation at line " + to_string(line));
			if (stp > 1)
				err("Postfix Evaluator:: unbalanced < equation at line " + to_string(line));
#endif
		}
		inline bool run_line(int &pointer, VScope *scope)
		{
			line = str_to_int(code, pointer);
			int old = pointer;
			// No more code
			if (line == 0)
				return false;

			int operation = str_to_int(code, pointer);
			// CONDITION
			if (operation == KEYW_IF)
			{
				int skip_to = str_to_int(code, pointer), tmp = pointer - 1;
				expression_evaluator(pointer, scope);
				if (st[stp].data.int_data == 0)
				{
					pointer = tmp + skip_to;
				}
			}
			// ++
			else if (operation == OPR_GO_FWD || operation == KEYW_FUNCTION)
			{
				int to = str_to_int(code, pointer);
				pointer += to;
			}
			// --
			else if (operation == OPR_GO_BCK)
			{
				int to = str_to_int(code, pointer);
				pointer -= (to + 1);
			}
			// back
			/*else if (operation == KEYW_RETURN) {
				// TODO: Store value in stack
				expression_evaluator(pointer);
				return true;
			}*/
			// MATHS
			else
			{
				pointer = old;
				expression_evaluator(pointer, scope);
			}
			return true;
		}

	public:
		Runtime(bool setup = false)
		{
			//st = MGlobals.var_heap.get();
			if (setup == true)
				make_opr_precedence();
		}
		int execute(string &c, bool debug = false)
		{
			code = (char *)c.c_str();
			int pointer = 0, len = c.length();
			if (debug)
			{
				while (pointer < len)
				{
					system("cls");
					int old = pointer, line = str_to_int(code, pointer);
					cout << "Line: " << line << "\n";
					cout << "-------------\n";
					unordered_map<string, int>::iterator it;
					for (it = VGlobals.real_variable_list.begin(); it != VGlobals.real_variable_list.end(); it++)
					{
						cout << it->first << ":\t";
						if (VGlobals.variable_list[it->second])
							VGlobals.variable_list[it->second]->display();
						else
							cout << "UNDEC";
						cout << "\n";
					}
					pointer = old;
					run_line(pointer, &VGlobals);
				}
			}
			else
			{
				chrono::seconds ms = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch());
				while (pointer < len)
				{
					run_line(pointer, &VGlobals);
				}
				cout << "\nexecution in " << to_string((chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()) - ms).count()) << " second(s)\n";
				cout << "-------------------------------------------------------------\n";
				unordered_map<string, int>::iterator it;
				for (it = VGlobals.real_variable_list.begin(); it != VGlobals.real_variable_list.end(); it++)
				{
					cout << it->first << ":\t";
					if (VGlobals.variable_list[it->second])
						VGlobals.variable_list[it->second]->display();
					else
						cout << "UNDEC";
					cout << "\n";
				}
			}
			return 0;
		}
		~Runtime()
		{ /*delete[] st; st = 0;*/
		}
	};

	class SL
	{
	public:
		static void run(string code)
		{
			Analyze a;
			vector<TOKEN> tree = a.analyze(code);
			//a.display(tree);

			Intermediate i(true);
			string inter = i.intermediate(tree);
			//i.display(inter, false);
			//i.display(inter, true);

			//system("pause");

			Runtime r;
			//r.execute(inter, true);
			r.execute(inter, false);
		}
		static void clear()
		{
			for (unsigned int i = 0; i < VGlobals.variable_list.size(); i++)
				if (VGlobals.variable_list.direct_access(i))
					delete VGlobals.variable_list.direct_access(i);
			for (unsigned int i = 0; i < CGlobals.constants_list.size(); i++)
				if (CGlobals.constants_list.direct_access(i))
					delete CGlobals.constants_list.direct_access(i);
			VGlobals.variable_list.~growing_array();
			CGlobals.constants_list.~growing_array();
			MGlobals.var_ht_heap.~heap_manager();
			MGlobals.vscope_heap.~heap_manager();
			MGlobals.vstack.~growing_array();
		}
	};
}
#endif