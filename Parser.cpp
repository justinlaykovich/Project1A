/*
   Parser implementation.
   Author: Justin Laykovich
   CS303, Fall 2016
*/

#include"Parser.h"
const string Parser::UNARYOPS = {'!',INC,DEC,'-'};
const string Parser::BINARYOPS = {'^','*','/','%','+',SUB,'>',GE,'<',LE,EQ,NE,AND,OR};
const string Parser::OPS = {')','(','!',INC,DEC,'-','^','*','/','%','+',SUB,'>',GE,'<',LE,EQ,NE,AND,OR};
const int Parser::PREC[] = {  9,  9,  8,  8,  8,  8,  7,  6,  6,  6,  5,  5,  4, 4,  4, 4, 3, 3,  2, 1};

Parser::Parser(string sentence) {
   newSentence(sentence);
}

void Parser::newSentence(string input) {
   sentence = input;
}

int Parser::eval(string input) {
   newSentence(input);
   return eval();
}

string Parser::getSentence() const{
   return sentence;
}

/*
   Rewrites the sentence to an easier format, erasing the need
   for complicated expressions in the main eval loop dealing with multiple
   character operators and right fixity. Also throws an error if an invalid character is
   given.
   Should be linear, as it at slowest iterates one character at a time per input.
*/

string Parser::sanitize(string sentence) const {
   char x,y;
   string result = "";
   sentence = " " + sentence;
   size_t length = sentence.length() - 1;

   for(int i=length; i >= 1; --i) {
      /*
         Eval right to left to correctly pair unary operators
         with right hand operands.
      */
      x = sentence[i-1];
      y = sentence[i];

      if(x == '-' && y == '-') {
         result = (DEC + (" " + result));
         i--;
      }
      else if(isdigit(x) && y == '-') {
         result = SUB + result;
      }
      else if(x == '+' && y == '+') {
         result = (INC + (" " + result));
         i--;
      }
      else if(x == '!' && y == '=') {
         result = (NE + (" " + result));
         i--;
      }
      else if(x == '>' && y == '=') {
         result = (GE + (" " + result));
         i--;
      }
      else if(x == '<' && y == '=') {
         result = (LE + (" " + result));
         i--;
      }
      else if(x == '=' && y == '=') {
         result = (EQ + (" " + result));
         i--;
      }
      else if(x == '&' && y == '&') {
         result = (AND + (" " + result));
         i--;
      }
      else if(x == '|' && y == '|') {
         result = (OR + (" " + result));
         i--;
      }
      else if(isalpha(y) || y != ' ' && !isdigit(y) && !ISOP(y)) {
         ostringstream oss;
         oss << "Invalid character " << y << " @ char " << (i-1);
         throw std::invalid_argument(oss.str());
      }
      else {
         result = y + result;
         if(i <= 1)
            result = x + result;
      }
   }


   /*
      This allows the expression to collapse in the same loop as that which collapses parentheses, so an additional
      outer loop is not necessary. Probably doesn't change efficiency, but makes it nicer.
   */
   result = "(" + result + ")";
   return result;
}

/*
   Main eval. Call to evaluate the current sentence sanitized by sanitize(sentence)
   Throws a formatted string as std::invalid_argument, indicating char position.

   Should be linear. Each loop at most evals processed operands and operators, each
   eval reduces the total stack size of operators and operands by at least one.
*/

int Parser::eval() const {
   std::stack<char> operators;
   std::stack<int> operands;
   istringstream tokens(sanitize(sentence));
   char next_char;

   /*
      PREV Keeps track of the type of the previous token, whether it was an operand or whether it was
      an operator and what type.
      The #defined tokens are BINARY, UNARY, and OPERAND, and PARENS. -1 is undefined.
   */
   int PREV = -1;

   try{
      while(tokens >> std::skipws >> next_char) {
         if(isdigit(next_char)) {
            if(PREV==OPERAND)
               throw std::invalid_argument("Two operands in a row");

            if(PREV==PARENS) // (...)x -> (...)*x
               operators.push('*');

            tokens.putback(next_char);
            int rhs;
            tokens >> rhs;

            /*
               Evals any unary operators on top. Unary operators all have a higher precedence than
               binary operators, and all bind to the right, so any operand to the right of a unary
               operator should be eval'd immediately.
            */

            while(!operators.empty() && ISUNARY(operators.top())) {
               rhs = eval_unary_op(rhs, operators.top());
               operators.pop();
            }

            if(rhs == 0 && (operators.top() == '/' || operators.top() == '%'))
               throw std::invalid_argument("Division by zero");

            operands.push(rhs);
            PREV = OPERAND;
         }
         else if(next_char == '(') {
            char newchar;
            tokens >> std::skipws >> newchar;

            if(ISBINARY(newchar)) {
               if(POS(tokens) == 0)
                  throw std::invalid_argument("Expressions can't start with a binary operator");
               else
                  throw std::invalid_argument("Parenthetical expressions can't start with a binary operator");
            }
            else if(PREV==OPERAND || PREV==PARENS) {

               /*
                  Allows for expressions like x(...) to be interpreted as x*(...), as is
                  commonly written.
               */

               operators.push('*');
               PREV = BINARY;
            }

            tokens.putback(newchar);
            operators.push('(');
         }
         else if(next_char == ')') {
            if(POS(tokens) == 0)
               throw std::invalid_argument("Expressions can't start with a closing parenthesis");

            if(PREV != OPERAND && PREV != PARENS)
               throw std::invalid_argument("Expressions, including parenthetical expressions, can't end with an operator or be empty");

            /* Collapse parenthetical expressions and, if at the end, the outer expression. */

            int lhs;
            int rhs;
            int result;

            while(!operators.empty() && !operands.empty() && operators.top() != '(') {
               rhs = operands.top();
               operands.pop();

               if(rhs == 0 && (operators.top() == '/' || operators.top() == '%'))
                  throw std::invalid_argument("Division by zero");

               if(ISUNARY(operators.top()))
                  result = eval_unary_op(rhs,operators.top());
               else {
                  lhs = operands.top();
                  operands.pop();
                  result = eval_bin_op(lhs,rhs,operators.top());
               }

               operands.push(result);
               operators.pop();
            }

            if(operators.empty())
               throw std::invalid_argument("Closing parenthesis with no opening parenthesis");
            else
               operators.pop();

            PREV = PARENS;
         }
         else {
            if (!operators.empty() && operators.top() != '(' && !operands.empty()
                && !ISUNARY(next_char) && (GETPREC(next_char) <= GETPREC(operators.top()))) {

               int rhs = operands.top();
               operands.pop();
               int lhs;
               while(!operators.empty() && !operands.empty() && operators.top() != '(' &&
                      ( (GETPREC(next_char) < GETPREC(operators.top())) ||
                        ((GETPREC(next_char) == GETPREC(operators.top())) && GETPREC(operators.top()) != 7))){

                     if(ISUNARY(operators.top()))
                        rhs = eval_unary_op(rhs,operators.top());
                     else if(!operands.empty()) {
                        lhs = operands.top();
                        operands.pop();
                        rhs = eval_bin_op(lhs,rhs,operators.top());
                     }

                     operators.pop();
               }

               operands.push(rhs);
            }

            if(ISBINARY(next_char))
               if(PREV == BINARY)
                  throw std::invalid_argument("Two binary operations in a row");
               else if(PREV == UNARY)
                  throw std::invalid_argument("A unary operator can't be followed by a binary operator");
               else
                  PREV = BINARY;
            else
               if(ISUNARY(next_char))
                  PREV = UNARY;

            operators.push(next_char);
         }
      }
   }

   catch(exception& e)
   {
      /* Format the message and throw to the calling function. */
      ostringstream oss;
      oss << e.what() << " @ char " << POS(tokens);
      throw std::invalid_argument(oss.str());
   }

   if(operands.size() > 1)
      throw std::invalid_argument("Evaluating all operators leaves multiple operands");
   if(operators.size() != 0)
      throw std::invalid_argument("Opening parenthesis with no closing parenthesis");

   int result = operands.top();
   operands.pop();
   return result;
}

/* Evals Unary operations. */
int Parser::eval_unary_op(int rhs, char op) const
{
   switch(op)
   {
      case '-':
        return (0 - rhs);
      case INC:
        return rhs + 1;
      case DEC:
        return rhs - 1;
      case '!':
        return (rhs == 0);
      default:
      {
        ostringstream oss;
        oss << "Not a valid unary operator: " << op;
        throw std::invalid_argument(oss.str());
      }
   }
}

/* Evals Binary operations. */
int Parser::eval_bin_op(int lhs, int rhs, char op) const
{
   switch(op)
   {
      case '^':
        return pow(lhs,rhs);
      case '*':
        return lhs * rhs;
      case '/':
         if(rhs == 0)
            /*
              This should be dealt with in the main loop and not here.
              Should only be an issue if the logic of the main loop is changed
              or invalid.
            */
            throw std::invalid_argument("Unhandled division by zero");
         return lhs / rhs;
      case '%':
         return lhs % rhs;
      case '+':
         return lhs + rhs;
      case SUB:
         return lhs - rhs;
      case '>':
         return lhs > rhs;
      case GE:
         return lhs >= rhs;
      case '<':
         return lhs < rhs;
      case LE:
         return lhs <= rhs;
      case EQ:
         return lhs == rhs;
      case NE:
         return lhs != rhs;
      case AND:
         return (lhs != 0) && (rhs != 0);
      case OR:
         return (lhs != 0) || (rhs != 0);
      default:{
         ostringstream oss;
         oss << "Not a valid binary operator: " << op;
         throw std::invalid_argument(oss.str());
      }
   }
}
