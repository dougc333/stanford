/* Parser.c */
/* by Nathan Folkert */
/* A hand-made parser.  Normally one would take advantage of
programs like yacc or bison to produce such parsers, but I have
created this for you so that you can alter it and see how it 
works.  I have interpreted the regular expression grammar to be
right-associative (which may seem somewhat unintuitive), for the simple
reason that I felt it was the most straightforward way to parse the
structure.  Your general solution should not be bothered by the
associativity, however.  Good luck! */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

/* Change this definition to 0 to turn off error message output for parser
 */

#define ERR_MESG 1

char *specialChars = "{}()[]\\.?*+|#$^;:_,&-!%/";

RegExpressionT *NewRegularExpression(void);

RegExpressionT *E(char *e, int *place);
RegExpressionT *P(char *e, int *place);
RegExpressionT *A(char *e, int *place);
RegExpressionT *B(char *e, int *place);
RegExpressionT *C(char *e, int *place);
RegExpressionT *D(char *e, int *place);
RegExpressionT *S(char *e, int *place);

int ValidForE (int input);
int ValidForA (int input);
int ValidForB (int input);
int ValidForC (int input);
int ValidForD (int input);

int AddSymbol (char *e, int *place, CharSetT *cs);
int AddRange (char *e, int *place, CharSetT *cs);
CharSetT *BuildPrintableCharSet(void);

int NextCharacter(char *string, int place);
void AdvanceParser(char *string, int* place);
int PeekCharacter(char *string, int *place, int steps);

/* Functions Defined in Header */

/* The root of the parsing engine.  Simply calls the E production */
RegExpressionT *ParseRegExp(char *rxstr) {
  int place = 0;
  return E(rxstr, &place); 
}

/* Don't worry about freeing null expressions -- this takes care of
   checking for you */
void FreeRegularExpression(RegExpressionT *regexp) {
  if (!regexp) return; /* pesky null pointers */
  if (regexp->charset) free (regexp->charset);
  if (regexp->term) FreeRegularExpression(regexp->term);
  if (regexp->next) FreeRegularExpression(regexp->next);
  free (regexp);
}

/* reads until two adjacent newlines, or until the end of the file is
   reached, and returns what it read as a string.  No further processing
*/
char *ReadFromFile(FILE *infile) {
  char *buffer;
  int len = 0, allocn = 32;
  int c, next;
  
  if ((c = getc(infile)) == EOF) {
    return NULL; 
  }
  ungetc(c, infile);
  buffer = (char *)malloc(allocn);
  if (!buffer) {
#if ERR_MESG
    perror("ReadFromFile");
#endif
    return NULL;
  }
  while (1) {
    if (len >= allocn - 1) buffer = realloc(buffer, (allocn *= 2));
    if (!buffer) {
#if ERR_MESG
      perror("ReadFromFile");
#endif
      return NULL;
    }
    c = getc(infile);
    if (c == EOF) break;
    if (c == '\n') {
      next = getc(infile);
      if (next == '\n') break;
      else ungetc(next, infile);
    }
    buffer[len] = c;
    len++;
  }
  buffer[len] = '\0';
  return buffer;
}

/* creates a character set and sets all bits to 0 */
CharSetT *CreateCharSet(void) {
  CharSetT *s;
  int i;
  
  s = (CharSetT *)malloc(sizeof(CharSetT));
  if (!s) {
#if ERR_MESG
    perror("CreateCharSet");
#endif
    return NULL;
  }
  for (i = 0; i < (32/sizeof(int)); i++) s->bytearr[i] = 0;
  return s;
}

/* sets bit c in charset on */
void AddCharacter(int c, CharSetT *h) {
  int field, offset;

  if (c < 0 || c > 255) {
#if ERR_MESG
    fprintf(stderr, "AddCharacter: invalid arguments.\n");
#endif
    return;
  }
  field = c / (sizeof(int) * 8);
  offset = 1 << (c % (sizeof(int) * 8));
  h->bytearr[field] |= offset;
}

void AddCharacterRange(int a, int b, CharSetT *cs) {
  int i;

  if (a < 0 || b < 0 || a > 255 || b > 255 || a > b) {
#if ERR_MESG
    fprintf(stderr, "AddCharacterRange: invalid arguments");
#endif
    return;
  }
  for (i = a; i <= b; i++)
    AddCharacter(i, cs);
}

void RemoveCharacterRange(int a, int b, CharSetT *cs) {
  int i;

  if (a < 0 || b < 0 || a > 255 || b > 255 || a > b) {
#if ERR_MESG
    fprintf(stderr, "RemoveCharacterRange: invalid arguments");
#endif
    return; 
  }
  for (i = a; i <= b; i++)
    RemoveCharacter(i, cs);
}

/* sets bit c in charset off */
void RemoveCharacter(int c, CharSetT *h) {
  int field, offset;

  if (c < 0 || c > 255) {
#if ERR_MESG
    fprintf(stderr, "RemoveCharacter: invalid arguments"); 
#endif
    return;
  }
  
  field = c / (sizeof(int) * 8);
  offset = 1 << (c % (sizeof(int) * 8));
  h->bytearr[field] &= (~offset);
}

/* checks if bit c in charset is on or off */
int InCharSet(int c, CharSetT *h) {
  int field, offset;

  if (c < 0 || c > 255) {
#if ERR_MESG
    fprintf(stderr, "InCharSet: invalid arguments");
#endif
    return 0;
  }

  field = c / (sizeof(int) * 8);
  offset = 1 << (c % (sizeof(int) * 8));
  return (h->bytearr[field] & offset);
}

/* inverts all bits in the charset from on to off and off to on */
void InvertCharSet(CharSetT *h) {
  int i;
  
  for (i = 0; i < (32/sizeof(int)); i++) h->bytearr[i] = ~h->bytearr[i];
}

/* Private Functions */

/* allocates and initializes regular expression structure */
RegExpressionT *NewRegularExpression(void) {
  RegExpressionT *regexp = (RegExpressionT *)
    malloc(sizeof(RegExpressionT));
  if (!regexp) {
#if ERR_MESG
    perror("NewRegularExpression");
#endif
    return NULL;
  }
  regexp->type = EMPTY;
  regexp->charset = NULL;
  regexp->term = NULL;
  regexp->next = NULL;
  return regexp;
}   

/* The Grammar: */

/* The approach to this parser is, in general, for any given production,
   looking at the next character to see what production to expand, and if
   the expansion is ever directly to a symbol, to advance the input string
   by one character.  The root level, E, expands the union operation.  It
   determines after it has parsed the first expression whether a union
   with another expression follows or if this expression is complete.  The
   production A represents a regular expression that is free of the union
   type (except possibly in parenthesized subexpressions, a caveat that
   applies to all following similar situations), while E represents a
   regular expression without that restriction 
*/

RegExpressionT *E(char *e, int *place) {
  int input;
  RegExpressionT *ex, *uex;

  input = NextCharacter(e, *place);
  switch(input) {
  case '\0': 
    return (NewRegularExpression()); /* empty expression = empty string
					(defined) */
  default: 
    if (!ValidForE(input)) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      return NULL; 
    }
    else ex = A(e, place);
    break;
  }
  if (!ex) return NULL;
  input = NextCharacter(e, *place);
  switch(input) {
  case '\0':
    return ex; /* end of string */
  case '|': 
    AdvanceParser(e, place); /* continue with unioned expression */
    uex = NewRegularExpression();
    uex->type = UNION;
    uex->term = ex;
    uex->next = E(e, place);
    if (!uex->next) { 
      FreeRegularExpression(uex);
      return NULL; 
    } else return uex;
  default: 
    FreeRegularExpression(ex);
#if ERR_MESG
    fprintf(stderr, "Bad regexp: character %c.\n", input);
    fprintf(stderr, "%s == at %d\n", e, *place);
#endif
    return NULL; /* indicates no other input accepted */
  }
  return NULL;
}

/* I've put the P-production here, because it is essentially the same as
   the E-production, save that it must end in ')' as opposed to '\0'.
*/

RegExpressionT *P(char *e, int *place) {
  int input;
  RegExpressionT *ex, *uex;

  input = NextCharacter(e, *place);
  switch(input) {
  case ')':
    return (NewRegularExpression()); /* empty expression = empty string
                                        (defined) */ 
  default:
    if (!ValidForE(input)) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      return NULL;
    }
    else ex = A(e, place);
    break;  
  }
  if (!ex) return NULL;
  input = NextCharacter(e, *place);
  switch(input) {
  case ')': 
    return ex; /* end of parenthesized subexpression */
  case '|':
    AdvanceParser(e, place); /* continue with unioned expression */
    uex = NewRegularExpression();
    uex->type = UNION; 
    uex->term = ex;
    uex->next = P(e, place);
    if (!uex->next) {
      FreeRegularExpression(uex);
      return NULL;
    } else return uex;
  default:
    FreeRegularExpression(ex);
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
    return NULL; /* indicates no other input accepted */
  }
  return NULL;
}  

/* The next level in our grammar, A, represents expressions without
   union.  It expands to B, and concats it, if appropriate characters
   follow to another expansion of A.  B represents expressions without
   concatenation, which includes only single characters, character sets, 
   parenthesized subexpressions, optional expressions, or closures of
   expressions. 
*/

RegExpressionT *A(char *e, int *place) {
  int input;
  RegExpressionT *ex, *cex;

  input = NextCharacter(e, *place);
  switch (input) {
  default: 
    if (!ValidForA(input)) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      return NULL; 
    }
    else ex = B(e, place);
    break;
  }
  if (!ex) return NULL;
  input = NextCharacter(e, *place);
  switch(input) {
  case '|': case '\0': case ')': 
    return ex; /* end of concatenation */
  default: 
    if (!ValidForA(input)) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      FreeRegularExpression(ex);
      return NULL;
    }
    cex = NewRegularExpression();
    cex->type = CONCAT;
    cex->term = ex;
    cex->next = A(e, place);
    if (!cex->next) {
      FreeRegularExpression(cex);
      return NULL; 
    }
    else return cex;
  }
  return NULL;
}

/* Level B in the parsing hierarchy represents expressions without
   concatenations.  B expands C, which represents only characters,
   character sets, or parenthesized subexpressions, and then determines
   whether a closure operation is applied -- Kleene closure or positive
   closure, or optional expressions.
 */

RegExpressionT *B(char *e, int *place) {
  int input;
  RegExpressionT *ex, *cex;
  
  input = NextCharacter(e, *place);
  switch(input) {
  default: 
    if (!ValidForB(input)) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      return NULL; 
    }
    else ex = C(e, place);
  }
  if (!ex) return NULL;
  input = NextCharacter(e, *place);
  switch(input) {
  case '*': 
    AdvanceParser(e, place); 
    cex = NewRegularExpression();
    cex->type = KLEENE;
    cex->term = ex;
    return cex;
  case '+': 
    AdvanceParser(e, place);
    cex = NewRegularExpression();
    cex->type = POSCLOSE;
    cex->term = ex;
    return cex;
  case '?': 
    AdvanceParser(e, place); 
    cex = NewRegularExpression();
    cex->type = OPTION;
    cex->term = ex;
    return cex;
  case '|': case '\0': case ')': 
    return ex;
  default: 
    if (!ValidForB(input)) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      FreeRegularExpression(ex);
      return NULL;
    } 
    else return ex;
  }
}

RegExpressionT *C(char *e, int *place) {
  int input; 
  RegExpressionT *ex;
  
  input = NextCharacter(e, *place);
  switch(input) {
  case '[': 
    AdvanceParser(e, place);
    ex = D(e, place);
    if (!ex) return NULL;
    if (!(NextCharacter(e, *place) == ']')) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      FreeRegularExpression(ex);
      return NULL;
    }
    AdvanceParser(e, place);
    return ex;
  case '(': 
    AdvanceParser(e, place);
    ex = P(e, place);
    if (!ex) return NULL;
    if (!(NextCharacter(e, *place) == ')')) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      FreeRegularExpression(ex);
      return NULL;
    }
    AdvanceParser(e, place);
    return ex;
  default: 
    if (!ValidForC(input)) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      return NULL; 
    }
    else return S(e, place);
  }
  return NULL;
}

RegExpressionT *D(char *e, int *place) {
  int input, peek, nextchar, result, invert = 0;
  RegExpressionT *ex;
  CharSetT *cs;
	
  input = NextCharacter(e, *place);
  if (input == '^') {
    invert = 1;
    AdvanceParser(e, place);
  }
  cs = CreateCharSet();
  while(1) {
    input = NextCharacter(e, *place);
    switch(input) {
    case ']': 
      if (invert) InvertCharSet(cs);
      ex = NewRegularExpression();
      ex->type = CHARSET;
      ex->charset = cs;
      return ex;
    default: 
      if (!ValidForD(input)) {
#if ERR_MESG
	fprintf(stderr, "Bad regexp: character %c.\n", input);
	fprintf(stderr, "%s == at %d\n", e, *place);
#endif
	free (cs); 
	return NULL;
      }
      if (input == '\\') peek = 2;
      else if (input == '#') peek = 4; 
      else peek = 1;
      nextchar = PeekCharacter(e, place, peek);
      if (nextchar == '-') result = AddRange(e, place, cs);
      else result = AddSymbol(e, place, cs);
      if (result == 0) {
#if ERR_MESG
	fprintf(stderr, "Failed to create character set.\n");
	fprintf(stderr, "%s == at %d\n", e, *place);
#endif
	free (cs);
	return NULL;
      }
    }
  }
  return NULL;
}

RegExpressionT *S(char *e, int *place) {
  int input, number[3];
  RegExpressionT *ex;
  CharSetT *cs;

  input = NextCharacter(e, *place);  
  switch(input) {
  case '{':
    cs = CreateCharSet();
    while(1) {
      AdvanceParser(e, place);
      input = NextCharacter(e, *place);
      if (input == '}') break;
      switch(input) {
      case 's': AddCharacter(' ', cs); break;
      case 'n': AddCharacter('\n', cs); break;
      case 'r': AddCharacter('\r', cs); break;
      case 't': AddCharacter('\t', cs); break;
      case 'f': AddCharacter('\f', cs); break;
      default: 
#if ERR_MESG
	fprintf(stderr, "Bad regexp: character %c.\n", input);
	fprintf(stderr, "%s == at %d\n", e, *place);
#endif
	free(cs); return NULL;
      }
    }
    AdvanceParser(e, place);
    ex = NewRegularExpression();
    ex->type = CHARSET;
    ex->charset = cs;
    return ex;
  case '$':
    AdvanceParser(e, place);
    ex = NewRegularExpression();
    ex->type = CHARSET;
    ex->charset = BuildPrintableCharSet();
    return ex;
  case '.':
    AdvanceParser(e, place);
    ex = NewRegularExpression();
    ex->type = ANYCHAR;
    return ex;
  case '&':          
    AdvanceParser(e, place);
    ex = NewRegularExpression();
    ex->type = EMPTY;
    return ex;
  case '\\':
    AdvanceParser(e, place);
    input = NextCharacter(e, *place);
    AdvanceParser(e, place);
    ex = NewRegularExpression();
    ex->type = ATOM;
    ex->value = input;
    return ex;
  case '#':
    AdvanceParser(e, place);
    number[0] = NextCharacter(e, *place); AdvanceParser(e, place);
    number[1] = NextCharacter(e, *place); AdvanceParser(e, place);
    number[2] = NextCharacter(e, *place); AdvanceParser(e, place);
    if (!isdigit(number[0]) || !isdigit(number[1]) || !isdigit(number[2]))
      {
#if ERR_MESG
	fprintf(stderr, "Specified character value not a number.\n");
	fprintf(stderr, "%s == at %d\n", e, *place);
#endif
	return NULL;
      }
    input = (((number[0] - '0') * 100) + ((number[1] - '0') * 10) +
	     number[2] - '0');
    if (input >= 256) {
#if ERR_MESG
      fprintf(stderr, "Invalid character value: %d.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      return NULL;
    }
    ex = NewRegularExpression();
    ex->type = ATOM;
    ex->value = input;
    return ex;
  default: 
    if (strchr(specialChars, input)) {
#if ERR_MESG
      fprintf(stderr, "Bad regexp: character %c.\n", input);
      fprintf(stderr, "%s == at %d\n", e, *place);
#endif
      return NULL;
    }
    AdvanceParser(e, place);
    ex = NewRegularExpression();
    ex->type = ATOM;
    ex->value = input;
    return ex;       
  }
  return NULL;
}

int ValidForE(int c) {
  if (c == '[' || c == '(' || c == '\\' || c == '{' || c == '.' || c ==
      '&' || c == '#' || c == '\0' || c == '$') return 1;
  if (!strchr(specialChars, c)) return 1;
  return 0;
}  

int ValidForA(int c) {
  if (c == '[' || c == '(' || c == '\\' || c == '{' || c == '.' || c ==
      '&' || c == '#' || c == '$') return 1;
  if (!strchr(specialChars, c)) return 1;
  return 0;
}

int ValidForB(int c) {
  if (c == '[' || c == '(' || c == '\\' || c == '{' || c == '.' || c ==
      '&' || c == '#' || c == '$') return 1;
  if (!strchr(specialChars, c)) return 1;
  return 0;
} 

int ValidForC(int c) {
  if (c == '\\' || c == '{' || c == '.' || c == '&'
      || c == '#' || c == '$') return 1;
  if (!strchr(specialChars, c)) return 1;
  return 0;
}

int ValidForD(int c) {
  if (c == '\\' || c == '#') return 1;
  if (!strchr(specialChars, c)) return 1;
  return 0;
}

int AddSymbol(char *e, int *place, CharSetT *cs) {
  int input, symbol, number[3];

  input = NextCharacter(e, *place);
  symbol = input;
  AdvanceParser(e, place);
  if (input == '\\') {
    symbol = NextCharacter(e, *place); 
    AdvanceParser(e, place);
  } 
  if (input == '#') {
    number[0] = NextCharacter(e, *place); AdvanceParser(e, place);
    number[1] = NextCharacter(e, *place); AdvanceParser(e, place);
    number[2] = NextCharacter(e, *place); AdvanceParser(e, place);
    if (!isdigit(number[0]) || !isdigit(number[1]) || !isdigit(number[2]))
      return 0;
    else symbol = ((number[0] - '0') * 100 + (number[1] - '0') * 
		   10 + number[2] - '0');
    if (symbol > 255) return 0;
  }
  AddCharacter(symbol, cs);
  return 1;
}

int AddRange(char *e, int *place, CharSetT *cs) {
  int first, second, input, number1[3], number2[3];
  
  input = NextCharacter(e, *place);
  first = input;
  AdvanceParser(e, place);
  if(input == '\\') {
    first = NextCharacter(e, *place);
    AdvanceParser(e, place);
  }
  if (input == '#') {
    number1[0] = NextCharacter(e, *place); AdvanceParser(e, place);
    number1[1] = NextCharacter(e, *place); AdvanceParser(e, place);
    number1[2] = NextCharacter(e, *place); AdvanceParser(e, place);
    if (!isdigit(number1[0]) || !isdigit(number1[1]) || !isdigit(number1[2]))
      return 0;
    else first = ((number1[0] - '0') * 100 + (number1[1] - '0') * 
		  10 + number1[2] - '0');
    if (first > 255) return 0;
  } else if (strchr(specialChars, first)) return 0;
  if (NextCharacter(e, *place) != '-') return 0;
  AdvanceParser(e, place);
  input = NextCharacter(e, *place);
  AdvanceParser(e, place);
  second = input;
  if(input == '\\') {
    second = NextCharacter(e, *place);
    AdvanceParser(e, place);
  }
  else if (input == '#') {
    number2[0] = NextCharacter(e, *place); AdvanceParser(e, place);
    number2[1] = NextCharacter(e, *place); AdvanceParser(e, place);
    number2[2] = NextCharacter(e, *place); AdvanceParser(e, place);
    if (!isdigit(number2[0]) || !isdigit(number2[1]) || !isdigit(number2[2]))
      return 0;
    else second = ((number2[0] - '0') * 100 + (number2[1] - '0') * 
		   10 + number2[2] - '0');
    if (second > 255) return 0;
  } else if (strchr(specialChars, second)) return 0;
  if (first >= second) return 0;
  AddCharacterRange(first, second, cs);

  return 1;
}

int NextCharacter(char *string, int place) {
  int c, i = place;
  
  while(1) {
    c = string[i];
    if (!isspace(c)) return c;
    i++;
  }
  return -1; /* never reached */
}

void AdvanceParser(char *string, int *place) {
  int c, i = *place;
  
  while(1) {
    c = string[i];
    if (!isspace(c)) {
      *place = i + 1; 
      return;
    }
    i++;
  }
}

int PeekCharacter(char *string, int *place, int steps) {
  int old = *place, value, i;
  
  for (i = 0; i < steps; i++) AdvanceParser(string, place);
  value = NextCharacter(string, *place);
  *place = old;
  return value;		
}


/* Testing functions */
void RecursivePrint(RegExpressionT *e);

void TestRegExp(RegExpressionT *e) {
  if (!e) { printf("invalid\n"); return; }
  RecursivePrint(e);
  printf("\n");
}

void RecursivePrint(RegExpressionT *e) {
  switch(e->type) {
  case ATOM: printf("%c", e->value); goto check; 
  case CHARSET: printf("@"); goto check; 
  case ANYCHAR: printf("."); goto check;
  case EMPTY: printf("&"); goto check;
  case KLEENE: printf("("); RecursivePrint(e->term); printf(")");
    printf("*"); goto check2;
  case POSCLOSE: printf("("); RecursivePrint(e->term); printf(")");
    printf("+"); goto check2;
  case OPTION: printf("("); RecursivePrint(e->term); printf(")");
    printf("?"); goto check2;
  case UNION: printf("("); RecursivePrint(e->term); printf("|");
    RecursivePrint(e->next); printf(")"); break;
  case CONCAT: printf("("); RecursivePrint(e->term);
    RecursivePrint(e->next); printf(")"); break;
  }
  return;
 check:
  if(e->term)printf("\nError 1 - nonterminal\n");
 check2:
  if(e->next)printf("\nError 2 - nonterminal\n");
}

CharSetT *BuildPrintableCharSet(void) {
  int i;
  CharSetT *cs = CreateCharSet();

  for (i = 0; i < 256; i++) {
    if (isprint(i))
      AddCharacter(i, cs);
  }
  return cs;
}

void PrintCharSet(CharSetT *cs) {
  int i, j = 0;

  for (i = 0; i < 256; i++) {
    if(InCharSet(i, cs)) { 
      if (isprint(i)) {
	printf("%c", i);
	j++;
      } else {
	printf(" (%.3d) ", i);
	j += 7;
      }
    }
    if (j > 40) {
      printf("\n");
      j = 0;
    }
  }
  printf("\n");
}






