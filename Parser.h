#ifndef _PARSER_H
#define _PARSER_H

#include<iostream>
#include<string>
#include<stack>
#include<sstream>
#include<cmath>

#define LE 'l'
#define GE 'g'
#define SUB 's'
#define DEC 'd'
#define INC 'i'
#define AND 'a'
#define OR 'o'
#define NE 'n'
#define EQ 'e'

#define GETPREC(op) PREC[OPS.find(op)]
#define ISUNARY(op) (UNARYOPS.find(op) != string::npos)
#define ISBINARY(op) (BINARYOPS.find(op) != string::npos)
#define ISOP(op) (OPS.find(op) != string::npos)
#define POS(tok) (((int)tok.tellg())-3)

using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::ostringstream;
using std::isdigit;
using std::exception;

class Parser
{
   public:
     Parser():Parser(""){};
     Parser(const string& sentence);

     void newSentence(const string& sentence);
     string getSentence() const;
     int eval() const;
     int eval(const string& sentence);
   private:
     enum FLAG {
        OPERAND,
        UNARY,
        BINARY,
        PARENS,
        START
     };
     static const int PREC[];
     static const string UNARYOPS;
     static const string BINARYOPS;
     static const string OPS;
     string sentence;

     int eval_bin_op(const int& lhs, const int& rhs, const char& op) const;
     int eval_unary_op(const int& rhs, const char& op) const;
     string sanitize(string sentence) const;
};
#endif
