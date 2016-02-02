//Mailbox.cc


#include "mailbox.h"



Mailbox::Mailbox() {
  //instantiate a single mailbox. 
}

Mailbox:: Send(int message) {
 m=message;
}

Mailbox::Receive(int *message) {
  //
  return &message=m;

}
