#include<iostream>
#include<string>
#include"Parser.cpp"

int main()
{
   string interactive;
   Parser parser;

   cout << parser.getSentence() << " = " << parser.eval("1+2*3") << endl;
   cout << parser.getSentence() << " = " << parser.eval("2+2^2*3") << endl;
   cout << parser.getSentence() << " = " << parser.eval("1==2") << endl;
   cout << parser.getSentence() << " = " << parser.eval("1+3 > 2") << endl;
   cout << parser.getSentence() << " = " << parser.eval("(4>=4) && 0") << endl;
   cout << parser.getSentence() << " = " << parser.eval("(1+2)*3") << endl;
   cout << parser.getSentence() << " = " << parser.eval("++++2-5*(3^2)") << endl;

   cout << "Enter an expression or q to quit." << endl;

   try
   {
      while(true)
      {
         getline (std::cin,interactive);
         if(interactive == "q" || interactive == "Q")
            break;
         parser.newSentence(interactive);
         cout << parser.eval() << endl;
      }
   }
   catch(exception& e)
   {
      cout << e.what() << endl;
   }

   return 0;
}
