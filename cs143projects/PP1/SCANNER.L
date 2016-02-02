/* 
 *dougc@stanford.edu Doug Chang
 *Connie's port scanner, this strips out the port from the
 * source and destination addresses 
 * 
 */

/*
 * File:  scanner.l
 * ----------------
 * Lex inupt file to generate the scanner for the compiler.
 */

%{

#include "hash.h"
#include "scanner.h"
#include "utility.h" // for PrintDebug()
#include "errors.h"
#include <string.h>

int line_num;
int pos;
int oneTimeDest=0;
int oneTimeSrc = 0;
YYSTYPE yylval;
struct yyltype yylloc;
static void DoBeforeEachAction();
#define YY_USER_ACTION DoBeforeEachAction();

%}

WHITESPACE        [ \t]+
LINENUMBER        (\n)
DECIMAL           [0-9]+
HEX               0[xX][0-9a-fA-f]+
ASCII             [+\-*/%<>=!;,.[\](){}]
DESTADDRESS       "NIS3.SLAC.Stanford.EDU\."[0-9]+
SRCADDRESS     "epaphus.SLAC.Stanford.EDU\."[0-9]+



%%             /* BEGIN RULES SECTION */
 /*
  * All patterns and actions should be placed between the start and stop
  * %% markers which delimit the Rules section. 
  */ 



{WHITESPACE}   { 
                 
               }


{LINENUMBER}   {
                 line_num++;
                 pos = 1;
               }


{SRCADDRESS}  { if(oneTimeSrc < 1){
                 char p[100];
                 int in=0;

                 for(int i=strlen("epaphus.SLAC.Stanford.EDU.");i<strlen(yytext);i++){
		    p[in]=yytext[i];
                    in++;
                 }
	         p[in] = '\0';	
 
                 printf("source port num is:%s\n",p);
                 }
                 oneTimeSrc++;
               }




{DESTADDRESS}  { if(oneTimeDest < 1){
                 char port[100];
                 int index=0;

                 for(int i=strlen("NIS3.SLAC.Stanford.EDU.");i<strlen(yytext);i++){
		    port[index]=yytext[i];
                    index++;
                 }
	         port[index] = '\0';	
 
                 printf("destination port num is:%s\n",port);
                 }
                 oneTimeDest++;
               }

.              {

               }


%%
/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated lex.yy.c file.
 * This section is where you put definitions of helper functions.
 */


/*
 * Function: Inityylex()
 * --------------------
 * This function will be called before any calls to yylex().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the scanner (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yy_flex_debug that controls whether flex prints debugging information
 * about each token and what rule was matched. If set to false, no information
 * is printed. Setting it to true will give you a running trail that might
 * be helpful when debugging your scanner. Please be sure the variable is
 * set to false when submitting your final version.
 */
void Inityylex()
{
    PrintDebug("lex", "Initializing scanner");
    line_num = 1;
    pos = 1;
    yy_flex_debug = false;
   
}


/*
 * Function: DoBeforeEachAction()
 * ------------------------------
 * This function is installed as the YY_USER_ACTION. This is a place
 * to group code common to all actions. Is column number referenced from 1 or 0?
 * 
 */
static void DoBeforeEachAction()
{
    yylloc.first_line = line_num;
    yylloc.first_column = pos;
    yylloc.last_column = pos + yyleng-1;
    pos = pos + yyleng;
}
















