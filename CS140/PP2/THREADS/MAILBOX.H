// mailbox.h




class Mailbox()
{
 public:
 Mailbox();
 ~Mialbox();
 
 void Send( int message);
 void Receive (int *message);
 void SelfTest();

 private:
 int message;

};




