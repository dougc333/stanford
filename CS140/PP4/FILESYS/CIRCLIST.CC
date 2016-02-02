#include "circlist.h"

CircularList::CircularList() {
  current = NULL;
  size = 0;
    
}

CircularList::~CircularList() {
  CirListElem *prev;
  
  //  while (current != NULL) {
  for (int i = 0; i < size; i++) {
    prev = current;
    current = current->next;
    delete prev;
  }
}


void CircularList::Add(int item) {
  CirListElem *element;
  
  element = new CirListElem(item);

  if (current == NULL) {
    current = element;
    current->next = element;
  } else {
    element->next = current->next;
    current->next = element;
    current = current->next;
  }
  size++;
}

void CircularList::Remove(int item) {
  CirListElem *slider;
  CirListElem *victim;

  slider = current->next;
  while ((slider != current) &&  (slider->next->num != item)) {
    slider = slider->next;
  }

  if (slider->next->num == item) {
    victim = slider->next;
    slider->next = victim->next;
    if (victim == current)
      current = slider--;
    delete victim;
    size--;
  }
}

int CircularList::Next(void) {
  current = current->next;
  return current->num;
}
