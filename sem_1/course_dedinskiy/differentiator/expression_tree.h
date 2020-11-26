#ifndef DIFFERENTIATOR
#define DIFFERENTIATOR

#include "general/c/common.h"
#include "general/c/strings_and_files.h"

#include <cctype>

#include "expression_node.h"

//=====================================================================================================================
// ExpressionTree =====================================================================================================


class ExpressionTree {
private:
// data =======================================================================
	ExprNode *root;
//=============================================================================

//=============================================================================
// Node disk work =============================================================
//=============================================================================

	void reading_ptr_proceed(char **buffer) {
		Char_get_next_symb(buffer);
		char *c = *buffer;
		if (*c == ')') {
			++c;
			Char_get_next_symb(&c);
		}
		*buffer = c;
	}

	void reading_ptr_skip_word(char **buffer) {
		Char_get_next_symb(buffer);
		char *c = *buffer;
		while (!isspace(*c) && *c != '(' && *c != ')') {
			++c;
		}
		*buffer = c;
	}

	ExprNode *load_node_value(char **buffer) {
		double val = 0;
		int symbs_read = 0;
		
		sscanf((const char *) *buffer, "%lg%n", &val, &symbs_read);
		if (symbs_read == 0) {
			return nullptr;
		}

		*buffer += symbs_read;
		reading_ptr_proceed(buffer);

		ExprNode *node = ExprNode::NEW(VALUE, val);
		return node;
	}

	ExprNode *load_node_variable(char **buffer) {
		char val = 0;
		int symbs_read = 0;
		
		sscanf((const char *) *buffer, "%c%n", &val, &symbs_read);
		if (symbs_read != 1) {
			return nullptr;
		}

		*buffer += symbs_read;
		reading_ptr_proceed(buffer);

		ExprNode *node = ExprNode::NEW(VARIABLE, val);
		return node;
	}

	ExprNode *load_node_operation(char **buffer) {
		char operation = 0;
		sscanf(*buffer, "%c", &operation);
		reading_ptr_skip_word(buffer);

		#define OPDEF(name, sign, arg_cnt, prior, calculation, ig1, ig2, ig3, ig4) { \
            case #sign[0]: {                                                         \
            	return ExprNode::NEW(OPERATION, #sign[0], prior);                    \
            }                                                                        \
		}

		switch (operation) {
			
			#include "ops.dsl"

			default: {
				printf("[ERR]<expr_load>: Invalid operation {%c}\n", operation);
				return nullptr;
			}
		}

		#undef OPDEF
	}

	ExprNode *load_node(char **buffer) {
		//printf("--> |%s|\n", *buffer);
		Char_get_next_symb(buffer);

		if (**buffer == ')') {
			return nullptr;
		} else if (isdigit(**buffer)) {
			return load_node_value(buffer);
		} else if ('a' <= **buffer && **buffer <= 'z') {
			return load_node_variable(buffer);
		} else { //===================================== definetly an operation
			if (!(**buffer == '(')) {
				return nullptr; //ERROR
			}
			++(*buffer);

			ExprNode *left_operand = load_node(buffer);

			if (!(**buffer)) {
				return left_operand;
			}

			if (**buffer == ')') {
				++(*buffer);
				Char_get_next_symb(buffer);
				return left_operand;
			}

			ExprNode *node = load_node_operation(buffer);
			if (!node) {return nullptr;} //ERROR

			ExprNode *right_operand = load_node(buffer);

			node->set_L(left_operand);
			node->set_R(right_operand);

			return node;
		}
	}

//=============================================================================

	void dump(ExprNode *node, int depth = 0, int to_format_cnt = 0, FILE *file_ptr = stdout) const {
		if (!node) {return;}

		dump(node->get_L(), depth + 1, to_format_cnt + 1, file_ptr);

		for (int i = 0; i < depth; ++i) {
			if (depth - to_format_cnt- 1 <= i) {
				printf("     |");
			} else {
				printf("      ");
			}
		}

		node->dump(file_ptr);
		if (node->get_L() || node->get_R()) printf("->|");
		printf("\n");

		dump(node->get_R(), depth + 1, to_format_cnt + 1, file_ptr);
	}


public:
	ExpressionTree():
	root(0)
	{}

	~ExpressionTree() {}

	void ctor() {

	}

	static ExpressionTree *NEW() {
		ExpressionTree *cake = (ExpressionTree*) calloc(1, sizeof(ExpressionTree));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	void dtor() {

	}

	static void DELETE(ExpressionTree *classname) {
		if (!classname) {
			return;
		}

		classname->dtor();
		free(classname);
	}

	ExprNode *get_root() const {
		return root;
	}

	void set_root(ExprNode *root_) {
		root = root_;
	}

//=============================================================================
// Disk work ==================================================================
//=============================================================================

	int load(const char *file_name) {
		if (file_name == nullptr) {
			printf("[ERR]<detree>: [file_name](nullptr)\n");
			return -1;
		}

		File *file = (File*) calloc(1, sizeof(File));
		if (File_construct(file, file_name) < 0) {
			printf("[ERR]<detree>: [file_name](%s) unexistance\n", file_name);
		}

		root = load_node((char**) &file->cc);
		//db_file = file;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11!!!!!!!!!!!!!!!!!!!!!!!! DELETE FILE

		return 0;
	}

	int save(const char *file_name) {
		if (file_name == nullptr) {
			printf("[ERR]<detree>: [file_name](nullptr)\n");
			return -1;
		}

		FILE *file = fopen(file_name, "w");
		if (!file) {
			printf("[ERR]<detree>: [file_name](%s) can't be opened\n", file_name);
			return -1;
		}

		//save_node(root, 0, false, file);

		fclose(file);
		return 0;
	}

//=============================================================================

	double evaluate(const double *var_table = nullptr, const size_t var_table_len = 0) {
		return root ? root->evaluate(var_table, var_table_len) : 0.0;
	}

	ExpressionTree *differentiate() {
		ExpressionTree *tree = NEW();
		if (root) {
			tree->set_root(root->differentiate());
		}

		return tree;
	}

	bool simplify_step() {
		if (!root) return 0;

		char success = 0;
		root = root->simplify(&success);
		root = root->simplify_neutral_elements(&success);
		return success;
	}

	bool simplify_structure_step() {
		if (!root) return 0;
		char success = 0;
		root = root->simplify_structure(&success);
		return success;
	}

	void simplify() {
		if (!root) return;
		while (simplify_step() || simplify_structure_step()) {}
	}

	void show_off(const char *file_name) {
		if (!root) {
			printf("[ERR]<expr_swof>: [root](nullptr)\n");
			return;
		}

		if (file_name == nullptr) {
			printf("[ERR]<expr_swof>: [file_name](nullptr)\n");
			return;
		}

		FILE *file = fopen(file_name, "w");
		if (!file) {
			printf("[ERR]<detexpr_swofree>: [file_name](%s) can't be opened\n", file_name);
			return;
		}

		fprintf(file, "\\documentclass{article}\n");
		fprintf(file, "\\usepackage{hyperref}\n");
		fprintf(file, "\\begin{document}\n\\Large\n");
		fprintf(file, "So we are given an expression:\n");

		fprintf(file, "$$ ");
		root->latex_dump(file);
		fprintf(file, "$$\n\n");

		fprintf(file, "Let's diffirintiate it!\n");

		ExpressionTree *differed = this;

		fprintf(file, "$$ ");
		differed->get_root()->latex_dump(file);
		fprintf(file, "$$\n\n");

		fprintf(file, "Uhhh, let's simplify it a bit...\n");

		char flag = 1;
		while(flag) {
			differed->dump();
			flag = 0;
			while (differed->simplify_step()) {
				flag = 1;
				fprintf(file, "SIMPLE $$ ");
				differed->get_root()->latex_dump(file);
				fprintf(file, "$$\n");
			}
			differed->dump();

			while (differed->simplify_structure_step()) {
				flag = 1;
				fprintf(file, "STRUCTURE $$ ");
				differed->get_root()->latex_dump(file);
				fprintf(file, "$$\n");
			}
			if (flag) {
			}
		}

		while(differed->simplify_step());

		fprintf(file, "So finaly:\n");
		fprintf(file, "$$ ");
		differed->get_root()->latex_dump(file);
		fprintf(file, "$$\n");

		fprintf(file, "\n\\end{document}");

		fclose(file);

		char generate_pdf_command[200];
		sprintf(generate_pdf_command, "pdflatex -output-directory='latex_output' -jobname='%s' %s", file_name, file_name);
		system(generate_pdf_command);

		const int table_size = 257;
		double table[table_size];
		table['x'] = 5;
		printf("dif_result = %lg\n", differed->evaluate(table, table_size));
		printf("-------------------------------------------------------\n");
		//differed->simplify_structure_step();
		//differed->dump();
		//printf("===\n");
		//char a;
		//differed->root->get_L()->fold_multiplication(&a);
		//differed->root = root->simplify_neutral_elements(&a);
		differed->dump();
	}

	void dump(FILE *file_ptr = stdout) const {
		dump(root, 0, 0, file_ptr);
	}

};

#endif // DIFFERENTIATOR