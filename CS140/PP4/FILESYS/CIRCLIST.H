#include <stdio.h>
class  CirListElem {
 public:
  CirListElem(int n) {next = NULL; num=n;}
  CirListElem *next;
  int num;
};
    
   

class CircularList {
 public:
  CircularList();
  ~CircularList();

  void Add(int item);
  void Remove(int item);
  int Next(void);
  int GetNumItems() {return size;}
 private:
  CirListElem *current;
  int size;
};
  
