#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"
#include <queue>
#include <future>
#include <unistd.h>
#include <thread>

template <typename T>
T MessageQueue<T>::receive()
{
        std::unique_lock<std::mutex> lck(_mutex);
        _cond.wait(lck, [this] { return !_queue.empty(); });

        T msg = std::move(_queue.back());
        //_queue.pop_front();
        _queue.clear();

        return msg;
    }

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
        std::unique_lock<std::mutex> lck(_mutex);
        std::cout << "Message: " << msg << " is in the queue" << "\n";
        _queue.emplace_back(std::move(msg));
        _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::Red;
}


void TrafficLight::waitForGreen(int id)
{
    while (true) {
        TrafficLightPhase response = _messages.receive();
        if (response == TrafficLightPhase::Green) {
            std::cout << "Intersection " << id << " is green" << "\n";
            break;
        } else {
            std::cout << "Intersection " << id << " is red" << "\n";
        }

    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{

    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{

  std::chrono::time_point<std::chrono::system_clock> timeOfLastCycle = std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock> timeOfCurrentCycle = std::chrono::system_clock::now();

  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distribution(4000, 6000);
  int cycleTime =  distribution(eng);

  while (true) {

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    timeOfCurrentCycle = std::chrono::system_clock::now();
    long time = std::chrono::duration_cast<std::chrono::milliseconds>(timeOfCurrentCycle - timeOfLastCycle).count();

    if (time >= cycleTime) {
      
      if (_currentPhase == TrafficLightPhase::Red) {
        _currentPhase = TrafficLightPhase::Green;
        timeOfLastCycle = std::chrono::system_clock::now();
      } else {
        _currentPhase = Red;
        timeOfLastCycle = std::chrono::system_clock::now();
      }

      _messages.send(std::move(_currentPhase));
    }
  }
}

