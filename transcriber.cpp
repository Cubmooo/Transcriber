#include <iostream>
#include <string>
#include <thread>

int main(){
    std::thread mic(fetchInput);
    std::thread FftThread(FFT);
    std::thread FftAnalyser(secondsToBeats);
    std::thread UI(InitialiseUI);
    std::thread UIInteraction(UIButtons);
    
    return 0;
}

int fetchInput(){
    return 0;
}

int FFT(){
    return 0;
}

int secondsToBeats(){
    return 0;
}

void InitialiseUI(){
    return;
}

void UIButtons(){

}