%{
#include <iostream>
#include <string>

#include "lang.h"

int yylex();
void yyerror(const char* s);
std::string* trim_whitespace(std::string* s, bool trailing = false);
int read_input(char* buffer, int* num_bytes_read, int max_bytes_to_read);

lang::Node root;
lang::Node* current_node = &root;

int new_level = 0;

%}

%union {
	// @todo Make sure we delete this after use
	std::string* sval;
}

%token ASSIGNMENT SECTION_START SECTION_END CONTINUE NEWLINE
%token <sval> STRING

%type <sval> key value text

%%

input
	: input line
	| line
	;

line
	: section NEWLINE
	| assignment NEWLINE
	| NEWLINE
	;

section
	: section_start key section_end { 
		while (new_level <= current_node->level) {
			current_node = current_node->parent;
		}
		auto it = current_node->children.find(*$2);
		if (it == current_node->children.end()) {
			current_node->children.emplace(*$2, lang::Node());
			auto& new_node = current_node->children[*$2];
			new_node.name = *$2;
			new_node.parent = current_node;
		}
		current_node = &current_node->children[*$2];
		current_node->level = new_level;
	}
	| section_start section_end {
		current_node = &root;
	}
	;

section_start
	: section_start SECTION_START {
		new_level++;
	}
	| SECTION_START {
		new_level = 1;
	}
	;

section_end
	: section_end SECTION_END
	| SECTION_END
	;

assignment
	: key ASSIGNMENT value {
		auto it = current_node->children.find(*$1);
		if (it == current_node->children.end()) {
			current_node->children.emplace(*$1, lang::Node());
			auto& new_node = current_node->children[*$1];
			new_node.name = *$1;
			new_node.parent = current_node;
		}
		auto t = trim_whitespace($3);
		lang::Value value(*t);
		current_node->children[*$1].value = value;
		delete t;
		delete $1;
		delete $3;
	}
	;

// @todo Remove leading and trailing spaces
key
	: STRING {
		$$ = trim_whitespace($1, true);
		delete $1;
	}
	;

// @todo Remove leading and trailing spaces
value
	: value CONTINUE NEWLINE text {
		auto t = trim_whitespace($4);
		delete $4;

		$$ = new std::string(*$1 + *t);
		delete $1;
		delete t;
	}
	| text

// @todo We should keep track of whitespace
text
	: text STRING {
		$$ = new std::string(*$1 + *$2);
		delete $1;
		delete $2;
	}
	| STRING

%%

lang::Node& get_root() {
	return root;
}

void yyerror(const char* s) {
	std::cerr << "Error: " << s << '\n';

	exit(-1);
}

std::string* trim_whitespace(std::string* s, bool trailing) {
	const auto begin = s->find_first_not_of(" \t");
	if (begin == std::string::npos) {
		return new std::string();
	}

	if (trailing) {
		const auto end = s->find_last_not_of(" \t");
		const auto range = end - begin + 1;

		return new std::string(s->substr(begin, range));
	}
	return new std::string(s->substr(begin));
}
