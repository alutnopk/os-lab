#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>
using namespace std;

//Function to be called when Ctrl+C command is given
void signal_callback_handler(int signum) {
   cout << "rok diya ;(  Signal-" << signum << endl;
   exit(signum); // Terminate program
}

int main(){
   signal(SIGINT, signal_callback_handler);
   while(1){
      cout << "chal raha hai.." << endl;
      sleep(1);
   }
   return 0;
}
