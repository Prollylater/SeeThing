#include <iostream>
#include "Mat.h"


int main(){

Mat<float> a(2,2,2);
   


    a.atChannel(0, 0,1) = 5.4f;
    a.atChannel(0, 0,0) = 0.4f;
    a.atChannel(1, 1,0) = 3.4f;
    a.atChannel(1, 1,1) = 0.4f;
    a.atChannel(1, 0,1) = 2.4f;
    a.atChannel(1, 0,0) = 1.4f;
    a.atChannel(0, 1,1) = 2.4f;
    a.atChannel(0, 1,0) = 1.4f;
    
    std::cout<< a<<std::endl;

    std::cout<<"Testing +, -, *, /"<< std::endl;
    Mat<float> ab = a+a;
    std::cout<< ab<<std::endl;
    ab = ab*a;
    std::cout<< ab<<std::endl;
    Mat<float> cb = a+3.0f;
    std::cout<< cb<<std::endl;
    Mat<float> dc = cb/2.0f;
    std::cout<< dc<<std::endl;
    cb = cb/2.0f;
    std::cout<< cb<<std::endl;
    
   
    std::cout<<"Testing +=, -=, *="<< std::endl;

    a += a;
    std::cout<< a<<std::endl;

    a += 2.0f;
    std::cout<< a<<std::endl;

    a *= 2.0f;
    std::cout<< a<<std::endl;


    a -= 2.0f;
    std::cout<< a<<std::endl;

    a -= a;
    std::cout<< a<<std::endl;

  
    
return 0;
}