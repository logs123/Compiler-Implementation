#ifndef _PROJECT4_H_
#define _PROJECT4_H_

#include "lexer.h"
#include "execute.h"

#include <map>
#include <vector>

class Parser {
    public:
        struct InstructionNode* parse_program();
        void parse_var_section();
        void parse_id_list();
        struct InstructionNode* parse_body();
        struct InstructionNode* parse_stmt_list();
        struct InstructionNode* parse_stmt();
        struct InstructionNode* parse_assign_stmt();
        std::vector<std::string> parse_expr();
        Token parse_primary();
        std::string parse_op();
        struct InstructionNode* parse_output_stmt();
        struct InstructionNode* parse_input_stmt();
        struct InstructionNode* parse_while_stmt();
        struct InstructionNode* parse_if_stmt();
        std::vector<std::string> parse_condition();
        std::string parse_relop();
        struct InstructionNode* parse_switch_stmt();
        struct InstructionNode* parse_for_stmt();
        struct InstructionNode* parse_case_list(std::string str);
        struct InstructionNode* parse_case(std::string str);
        struct InstructionNode* parse_default_case();
        void parse_inputs();
        void parse_num_list();
        int location(std::string var);
        std::map<std::string, int> location_table;
    private:
        LexicalAnalyzer lexer;
        void syntax_error();
        Token expect(TokenType expected_type);
};

#endif