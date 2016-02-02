/* hong lam honglam@stanford.edu
 * doug chang dougc@stanford.edu
 * dpp.l Preprocessor scanner
 * ---------------------------------------------------------
 */

%{
#include "utility.h"
#include "errors.h"
#include "hash.h"

void InitPreProcessor(void);
symtabADT hashTable;

%}

%x comment



%%
	int line_num = 1;
\n				{
				  ++line_num;
				  putchar('\n');
				}
"/*"				{
                          BEGIN(comment);
                        }
<comment>[^*\n]*
<comment>"*"+[^*/\n]*
<comment>\n			{
				  ++line_num;
				  putchar('\n');
				}
<comment>"*/"		{
                          BEGIN(INITIAL);
                        }
<comment><<EOF>>	{
				  putchar(EOF);
                  	  ReportError(line_num,err_unterm_comment,yytext);
				  return 0;
		      	}
"//"[^\n]*

 ^#define[ ][A-Z]+[ ][^\n]*                { char key[yyleng], value[yyleng];
                          int i=8, j=0, k=0;

                          while( (yytext[i]!=' ') && (yytext[i]!='\t') &&(yytext[i]!='\n') && (i<yyleng)){
                            key[j]=yytext[i];
			    i++;	
                            j++;
                          }

			  while(i<yyleng){
			    i++;
		            value[k]=yytext[i];
                            k++;
			  }
                          
                          key[j] = '\0';
		          value[k] = '\0';
                          /*printf("#define match, key:%s, value:%s\n",key,value);*/
                       
                          if(Lookup(hashTable,key)==NULL){
                            Enter(hashTable,key,value);      
                          }
                          else{
                            ChangeValue(hashTable,key,value);    
			  }
                                        
                        }

\"[^\n^"]*\"  		{/*printf("in string test rule\n");*/
			 ECHO;
		        }

#[A-Z]+                 { /*printf("in rule #[A-Z]+");*/
                          char temp[yyleng];
                          int i=1, j=0;
                          while((yytext[i]!=' ') && (i<yyleng)){
                              temp[j]=yytext[i];
			      i++;	
                              j++;
                          }

                          temp[j] = '\0';
                          /*printf("temp is:%s\n",temp);*/
                          char *value = Lookup(hashTable,temp);

                          if(value!=NULL){
                             /*printf("hash match, yytext is:%s hashtable returned value:%s linenumber is:%d\n",yytext,value,line_num);*/
                             
                             for(int i=0;i<strlen(value);i++){
                               putc(value[i],stdout);
                             }
                          }
                          else{
                             ReportError(line_num,err_inval_dir);
                          }
                        }


#[^A-Z].*               { /*printf("yytext is:%s, line_num is:%d\n",yytext,line_num);*/
                          ReportError(line_num,err_inval_dir );
                         
                        }

           
.|\n                   {/* I want the nonmatches to be echoed to stdout */
                        /* printf("last rule");*/
                         ECHO;
                       }                              

%%

void InitPreProcessor(void)
{
   hashTable = NewSymbolTable();
}
