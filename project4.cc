#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <vector>
#include <iostream>
#include "execute.h"
#include "lexer.h"
#include "project4.h"

using namespace std;

void Parser::syntax_error() {

    cout << "SYNTAX ERROR !!!\n";
    exit(1);

}

Token Parser::expect(TokenType expected_type) {

    Token t = lexer.GetToken();

    if (t.token_type != expected_type) {

        syntax_error();

    }

    return t;

}

int Parser::location(string str) {

    return location_table[str];

}

struct InstructionNode* Parser::parse_program() {

    struct InstructionNode* instList;

    parse_var_section();
    instList = parse_body();
    parse_inputs();

    return instList;
    
}

void Parser::parse_var_section() {

    parse_id_list();
    expect(SEMICOLON);

}

void Parser::parse_id_list() {

    Token t = expect(ID);
    int address = next_available;
    location_table.insert(pair<string, int>(t.lexeme, address));
    mem[next_available] = 0;
    next_available++;
    
    t = lexer.peek(1);

    if (t.token_type == COMMA) {

        expect(COMMA);
        parse_id_list();

    } else if (t.token_type == SEMICOLON) {

        return;

    } else {

        syntax_error();

    }

}

struct InstructionNode* Parser::parse_body() {

    struct InstructionNode* instList;

    expect(LBRACE);
    instList = parse_stmt_list();
    expect(RBRACE);

    return instList;

}

struct InstructionNode* Parser::parse_stmt_list() {

    struct InstructionNode* inst = new InstructionNode;
    struct InstructionNode* instList = new InstructionNode;

    inst = parse_stmt();

    Token t = lexer.peek(1);

    if (t.token_type == ID || t.token_type == WHILE || t.token_type == IF || t.token_type == SWITCH || t.token_type == FOR || t.token_type == OUTPUT || t.token_type == INPUT) {

        instList = parse_stmt_list();

        if (inst->next != NULL) {

            struct InstructionNode* head = new InstructionNode;
            head = inst;

            while (inst->next != NULL) {

                inst = inst->next;

            }

            inst->next = instList;
            inst = head;

        } else {

            inst->next = instList;

        }

    } else if (t.token_type == RBRACE) {

        return inst;

    } else {

        syntax_error();

    }

    return inst;

}

struct InstructionNode* Parser::parse_stmt() {

    Token t = lexer.peek(1);
    struct InstructionNode* inst = new InstructionNode;

    if (t.token_type == ID) {

        inst = parse_assign_stmt();

    } else if (t.token_type == WHILE) {

        inst = parse_while_stmt();

    } else if (t.token_type == IF) {

        inst = parse_if_stmt();

    } else if (t.token_type == SWITCH) {

        inst = parse_switch_stmt();

    } else if (t.token_type == FOR) {

        inst = parse_for_stmt();

    } else if (t.token_type == OUTPUT) {

        inst = parse_output_stmt();

    } else if (t.token_type == INPUT) {

        inst = parse_input_stmt();

    } else {

        syntax_error();

    }

    return inst;

}

struct InstructionNode* Parser::parse_assign_stmt() {

    Token t = expect(ID);
    struct InstructionNode* inst = new InstructionNode;
    inst->type = ASSIGN;
    inst->assign_inst.left_hand_side_index = location(t.lexeme);

    expect(EQUAL);

    Token t1 = lexer.peek(1);
    Token t2 = lexer.peek(2);

    if ((t1.token_type == ID || t1.token_type == NUM) && (t2.token_type == PLUS || t2.token_type == MINUS || t2.token_type == MULT || t2.token_type == DIV)) {

        vector<string> v = parse_expr();

        if (v[1] == "+") {

            inst->assign_inst.op = OPERATOR_PLUS;

        } else if (v[1] == "-") {

            inst->assign_inst.op = OPERATOR_MINUS;

        } else if (v[1] == "*") {

            inst->assign_inst.op = OPERATOR_MULT;

        } else if (v[1] == "/") {

            inst->assign_inst.op = OPERATOR_DIV;

        }
        
        inst->assign_inst.operand1_index = location(v[0]);
        inst->assign_inst.operand2_index = location(v[2]);
        inst->next = NULL;

    } else if (t1.token_type == ID || t1.token_type == NUM) {

        inst->assign_inst.op = OPERATOR_NONE;
        inst->assign_inst.operand1_index = location(parse_primary().lexeme);
        inst->next = NULL;

    } else {

        syntax_error();

    }

    expect(SEMICOLON);

    return inst;

}

vector<string> Parser::parse_expr() {

    vector<string> v;
    
    v.push_back(parse_primary().lexeme);
    v.push_back(parse_op());
    v.push_back(parse_primary().lexeme);

    return v;

}

Token Parser::parse_primary() {

    Token t = lexer.peek(1);

    if (t.token_type == ID) {

        t = expect(ID);

    } else if (t.token_type == NUM) {
        
        t = expect(NUM);
        int address = next_available;
        location_table.insert(pair<string, int>(t.lexeme, address));
        mem[next_available] = stoi(t.lexeme);
        next_available++;
        

    } else {

        syntax_error();

    }

    return t;

}

string Parser::parse_op() {

    Token t = lexer.peek(1);

    if (t.token_type == PLUS) {

        t = expect(PLUS);
        return "+";

    } else if (t.token_type == MINUS) {

        t = expect(MINUS);
        return "-";

    } else if (t.token_type == MULT) {

        t = expect(MULT);
        return "*";

    } else if (t.token_type == DIV) {

        t = expect(DIV);
        return "/";

    } else {

        syntax_error();

    }

    return "";

}

struct InstructionNode* Parser::parse_output_stmt() {

    expect(OUTPUT);

    Token t = expect(ID);
    struct InstructionNode* inst = new InstructionNode;
    inst->type = OUT;
    inst->output_inst.var_index = location(t.lexeme);
    inst->next = NULL;

    expect(SEMICOLON);

    return inst;

}

struct InstructionNode* Parser::parse_input_stmt() {

    expect(INPUT);

    Token t = expect(ID);
    struct InstructionNode* inst = new InstructionNode;
    inst->type = IN;
    inst->input_inst.var_index = location(t.lexeme);
    inst->next = NULL;

    expect(SEMICOLON);

    return inst;

}

struct InstructionNode* Parser::parse_while_stmt() {

    expect(WHILE);

    vector<string> v = parse_condition();
    struct InstructionNode* inst = new InstructionNode;
    inst->type = CJMP;
    inst->cjmp_inst.operand1_index = location(v[0]);
    inst->cjmp_inst.operand2_index = location(v[2]);

    if (v[1] == "<>") {

        inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    } else if (v[1] == ">") {

        inst->cjmp_inst.condition_op = CONDITION_GREATER;

    } else if (v[1] == "<") {

        inst->cjmp_inst.condition_op = CONDITION_LESS;

    }

    inst->next = parse_body();

    struct InstructionNode* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = NULL;

    struct InstructionNode* jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->jmp_inst.target = inst;
    jmp->next = noop;

    inst->cjmp_inst.target = noop;

    struct InstructionNode* head = new InstructionNode;
    head = inst;

    while (inst->next != NULL) {

        inst = inst->next;

    }

    inst->next = jmp;

    inst = head;

    return inst;

}

struct InstructionNode* Parser::parse_if_stmt() {

    expect(IF);

    vector<string> v = parse_condition();
    struct InstructionNode* inst = new InstructionNode;
    inst->type = CJMP;
    inst->cjmp_inst.operand1_index = location(v[0]);
    inst->cjmp_inst.operand2_index = location(v[2]);

    if (v[1] == "<>") {

        inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    } else if (v[1] == "<") {

        inst->cjmp_inst.condition_op = CONDITION_LESS;

    } else if (v[1] == ">") {

        inst->cjmp_inst.condition_op = CONDITION_GREATER;

    }

    inst->next = parse_body();

    struct InstructionNode* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = NULL;

    struct InstructionNode* head = new InstructionNode;
    head = inst;

    while (inst->next != NULL) {

        inst = inst->next;

    }

    inst->next = noop;

    inst = head;

    inst->cjmp_inst.target = noop;

    return inst;

}

vector<string> Parser::parse_condition() {

    vector<string> v;

    v.push_back(parse_primary().lexeme);
    v.push_back(parse_relop());
    v.push_back(parse_primary().lexeme);

    return v;

}

string Parser::parse_relop() {

    Token t = lexer.peek(1);

    if (t.token_type == GREATER) {

        t = expect(GREATER);
        return ">";

    } else if (t.token_type == LESS) {

        t = expect(LESS);
        return "<";

    } else if (t.token_type == NOTEQUAL) {

        t = expect(NOTEQUAL);
        return "<>";

    } else {

        syntax_error();

    }

    return "";

}

struct InstructionNode* Parser::parse_switch_stmt() {

    expect(SWITCH);
    
    Token t = expect(ID);
    
    expect(LBRACE);
    
    struct InstructionNode* inst = new InstructionNode;
    inst = parse_case_list(t.lexeme);

    struct InstructionNode* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = NULL;

    struct InstructionNode* head = new InstructionNode;
    head = inst;

    struct InstructionNode* cjmpHead = new InstructionNode;

    while (inst->next != NULL) {

        if (inst->cjmp_inst.target != NULL) {

            cjmpHead = inst->cjmp_inst.target;

            while (cjmpHead->next != NULL) {

                cjmpHead = cjmpHead->next;

            }

            cjmpHead->next = noop;

        }

        inst = inst->next;

    }

    if (inst->cjmp_inst.target != NULL) {

        cjmpHead = inst->cjmp_inst.target;

        while (cjmpHead->next != NULL) {

            cjmpHead = cjmpHead->next;

        }

        cjmpHead->next = noop;

    }

    t = lexer.peek(1);

    if (t.token_type == RBRACE) {

        expect(RBRACE);

    } else if (t.token_type == DEFAULT) {

        inst->next = parse_default_case();
        expect(RBRACE);

    } else {

        syntax_error();

    }

    while (inst->next != NULL) {

        inst = inst->next;

    }

    inst->next = noop;

    return head;

}

struct InstructionNode* Parser::parse_for_stmt() {

    expect(FOR);
    expect(LPAREN);
    struct InstructionNode* assign_stmt_1 = new InstructionNode;
    assign_stmt_1 = parse_assign_stmt();

    vector<string> v = parse_condition();

    struct InstructionNode* inst = new InstructionNode;
    inst->type = CJMP;
    inst->cjmp_inst.operand1_index = location(v[0]);
    inst->cjmp_inst.operand2_index = location(v[2]);

    if (v[1] == "<>") {

        inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    } else if (v[1] == ">") {

        inst->cjmp_inst.condition_op = CONDITION_GREATER;

    } else if (v[1] == "<") {

        inst->cjmp_inst.condition_op = CONDITION_LESS;

    }

    expect(SEMICOLON);

    struct InstructionNode* assign_stmt_2 = new InstructionNode;
    assign_stmt_2 = parse_assign_stmt();

    expect(RPAREN);

    inst->next = parse_body();

    struct InstructionNode* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = NULL;

    inst->cjmp_inst.target = noop;

    struct InstructionNode* jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->jmp_inst.target = inst;
    jmp->next = noop;

    assign_stmt_2->next = jmp;

    struct InstructionNode* head = new InstructionNode;
    head = inst;

    while (inst->next != NULL) {

        inst = inst->next;

    }

    inst->next = assign_stmt_2;

    inst = head;

    assign_stmt_1->next = inst;

    return assign_stmt_1;

}

struct InstructionNode* Parser::parse_case_list(string str) {

    struct InstructionNode* inst = new InstructionNode;
    inst = parse_case(str);

    Token t = lexer.peek(1);

    if (t.token_type == CASE) {

        inst->next = parse_case_list(str);

    } else if (t.token_type == RBRACE || t.token_type == DEFAULT) {

        return inst;

    } else {

        syntax_error();

    }

    return inst;

}

struct InstructionNode* Parser::parse_case(string str) {

    struct InstructionNode* inst = new InstructionNode;
    inst->type = CJMP;
    inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    inst->next = NULL;

    expect(CASE);
    
    Token t = expect(NUM);
    int address = next_available;
    location_table.insert(pair<string, int>(t.lexeme, address));
    mem[next_available] = stoi(t.lexeme);
    next_available++;

    inst->cjmp_inst.operand1_index = location(str);
    inst->cjmp_inst.operand2_index = next_available - 1;
    
    expect(COLON);

    inst->cjmp_inst.target = parse_body();

    return inst;

}

struct InstructionNode* Parser::parse_default_case() {

    expect(DEFAULT);
    expect(COLON);
    return parse_body();

}

void Parser::parse_inputs() {

    parse_num_list();

}

void Parser::parse_num_list() {

    Token t = expect(NUM);
    inputs.push_back(stoi(t.lexeme));

    t = lexer.peek(1);

    if (t.token_type == NUM) {

        parse_num_list();

    } else if (t.token_type == END_OF_FILE) {

        return;

    } else {

        syntax_error();

    }

}

struct InstructionNode * parse_generate_intermediate_representation() {

    Parser parser;
    struct InstructionNode* inst;
    inst = parser.parse_program();
    return inst;

}
