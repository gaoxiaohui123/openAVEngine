#include <iostream>  
using namespace std;  
  
class TestLib  
{  
    public:  
        void display();  
        int display(int a);
};  
void TestLib::display() {  
    cout<<"First display"<<endl;  
}  
  
int TestLib::display(int a) {
    return (10 * a);
}  
extern "C" {  
    TestLib obj;  
    void display() {  
        obj.display();   
      }  
    int display_int(int a) {
        return obj.display(a);
      }  
}

//g++ -o libpycallclass.so -shared -fPIC pycallclass.cpp