/* test file for testing scanner implementation */

#include <stdio.h>
#include "scanner.h"

#define noFileName "NoFile"
#define fileName   "student"


int main(int argc, char *argv[]){
 
  Scanner s =NULL;

  printf("test.c calling NewScannerFromFileName w/no filename\n");
  s = NewScannerFromFilename("NoFile", "@#$%^&", false );
  printf("end NewScannerFromFileName\n");
  return 0;

}
