/**
 * @mainpage CS361 - Thread race 2000
 * @section Description
 * 
 * <Made using some code from lectures on semaphores, courtesy of Mark Boady>
 * 
 * Make commands:
 * 
 *  make
 * 
 * will build the binary.
 * 
 *  make run
 * 
 * will run three races with 2, 4 and 10 racers.
 * 
 *  make clean
 * 
 * will clear out compiled code.
 * 
 *  make doc
 * 
 * will build the doxygen files.
 */

/**
 * @file
 * @author Nicholas Pelletier
 * @date 2022-2023
 * @section Description
 * 
 * <Main file for the racing of threads given a gamemaster thread and a user given number of race threads. The race is to 20 spaces and the racer threads look for movement given a queue of D6 rolls by the gamemaster thread.>
*/
#include <iostream>
//including sstream for changing ids to strings
#include <sstream>
//Include string for making strings...
#include <string>
//Include Random for sleep time.
#include <random>
//chrono for seeding an slep
#include <chrono>
//yea a queue
#include <queue>
//need vectors for quality of life
#include <vector>
//using the semaphores h file alloted to us by Boady.
#include "semaphore.h"
//Include Threads
#include <thread>

/**
 * Game Master rolls dice and populates queue
 * @param racers is the number of racers
 * @param q is the queue to add to
 * @param standings is the final resulting finish order
 * @param semaQ is the semaphore for the Queue
 * @param semaCO is the semaphore for the I/O
 * @param semaStand is the semaphore for the Standings
 */
void gameMaster(int racers, std::queue<unsigned int> &q, std::vector<std::string> &standings, semaphore &semaQ, semaphore &semaCO, semaphore &semaStand);

/**
 * racer that takes the rolls from the queue and moves spaces.
 * @param q is the queue to add to
 * @param standings is the final resulting finish order
 * @param semaQ is the semaphore for the Queue
 * @param semaCO is the semaphore for the I/O
 * @param semaStand is the semaphore for the Standings
 */
void racerThread(std::queue<unsigned int> &q, std::vector<std::string> &standings, semaphore &semaQ, semaphore &semaCO, semaphore &semaStand);

//Random Time to Wait between 0-2 seconds
std::chrono::seconds waitTime();

/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */

int main(int argc, char *argv[])
{
    std::string theThreads;
    int racers;
    int numThreads;
    bool wasDigit;

    //check for more than one input. Let user know.
    if(argc>2){
        std::cout << "Only enter one commandline argument, thanks." << std::endl;
    }
    //Let user know that they need to give an input if they didn't give one.
    else if(argc==1){
        std::cout << "Usage: race num_tests" << std::endl;
    }
    else{
        //setup input from value.
        theThreads = argv[1];
        //check if that input is actually a number.
        for(size_t i=0; i< theThreads.length(); i++ )
        {
            if(isdigit(theThreads[i])){
                wasDigit=true;
                continue;
            }
            else{
                //leave loop if it's not a number
                wasDigit=false;
                break;
            }
        }
        //let user know they need to enter a number.
        if(wasDigit==false){
            std::cout<<"You need to enter a number"<<std::endl;
            return 0;
        }
        //string to int.
        racers=stoi(theThreads);
    }
    if(wasDigit && racers>0){
        //Notify of race start.
        std::cout<<"Starting Race with " + std::to_string(racers) + " threads." <<std::endl;
        //set up necessary variables and semaphores.
        std::queue<unsigned int> q;
        std::vector<std::string> standings;
        semaphore semaQ, semaCO, semaStand;
        //Set up threads
        std::thread threads[racers];
        std::thread gameM(gameMaster, std::ref(racers), std::ref(q), std::ref(standings), std::ref(semaQ), std::ref(semaCO), std::ref(semaStand));
        for(size_t i=0; i<racers; i++){
            threads[i]= std::thread(racerThread, std::ref(q), std::ref(standings), std::ref(semaQ), std::ref(semaCO), std::ref(semaStand));
        }
        //join threads
        gameM.join();
        for(size_t j=0; j<racers; j++){
            threads[j].join();
        }
        //once all have run post standings
        for(size_t k=0; k<standings.size(); k++){
            std::cout<<std::to_string(k+1) << ": " << standings[k] <<std::endl;
        }
    }
    else if(racers==0){
        std::cout<<"You need to enter more than 0 racers."<<std::endl;
    }
    return 0;
}

//Wait a random time to simulate hard work
std::chrono::seconds waitTime(){
	int time2Wait = std::rand()%3;
	std::chrono::seconds t
			= std::chrono::seconds(time2Wait);
	return t;
}
//Game Master rolls dice and populates queue
void gameMaster(int racers, std::queue<unsigned int> &q, std::vector<std::string> &standings, semaphore& semaQ, semaphore& semaCO, semaphore& semaStand){
    //Initiate a random number generator for number of rolls and rolls.
    std::default_random_engine eng(std::random_device {}());
    std::uniform_int_distribution<int> dis(0,5);
    std::uniform_int_distribution<int> dice(1,6);
    int ranNum;
    //Create rolls while the standings aren't full.
    while(standings.size()!=racers){
        std::vector<int> rolls; 
        ranNum=dis(eng);
        //create rolls.
        for(size_t i=0; i<ranNum; i++){
            semaQ.wait();
            //add rolls to queue
            q.push(dice(eng));
            semaQ.signal();
        }
        //Fall Asleep
		std::this_thread::sleep_for(waitTime());
    }
}
//racer that takes the rolls from the queue and moves spaces.
void racerThread(std::queue<unsigned int> &q, std::vector<std::string> &standings, semaphore &semaQ, semaphore &semaCO, semaphore &semaStand){
    //initial needed variables.
    int movedSpaces=0;
    int currentSpaces=0;
    bool hasRun=false;
    //notify of leaving the gate.
    semaCO.wait();
    std::cout << "Thread " << std::this_thread::get_id() << " has left the gate."<<std::endl;
    semaCO.signal();
    while(movedSpaces<20){
        hasRun=false;
        //If the queue's not empty, then read from it.
        semaQ.wait();
        if(!q.empty()){
            semaStand.wait();
            currentSpaces=q.front();
            //move forward spaces
            movedSpaces+=currentSpaces;
            q.pop();
            semaCO.wait();
            //notify of movement.
            std::cout << "Thread " << std::this_thread::get_id() << " moved forward " << currentSpaces << " spaces." <<std::endl;
            semaCO.signal();
            //notify self that we have run.
            hasRun=true;
            if(movedSpaces<20){
                semaStand.signal();
            }
        }
        semaQ.signal();
        //If it ran sleep.
        if(hasRun){
            std::this_thread::sleep_for(waitTime());
        }

    }
    //We have crossed the finish line, yay! 
    semaCO.wait();
    std::cout << "Thread " << std::this_thread::get_id() << " has crossed the finish line."<<std::endl;
    semaCO.signal();
    //followed through stackoverflow via NutCracker on converting thread ids to string types. Modified to remove excess variables.
    std::stringstream ss;
    ss<<std::this_thread::get_id();
    standings.push_back(ss.str());
    semaStand.signal();
}