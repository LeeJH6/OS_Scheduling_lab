/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 
*	    Student name : 
*	    Date : 
*	    Contents :
*/

#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <algorithm>
#include <random>
#include <unordered_map>
#include "sched.h"

class RR : public Scheduler{
    private:
        int time_slice_;
        int left_slice_;
        std::queue<Job> waiting_queue;

    public:
        RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead) {
            name = "RR_"+std::to_string(time_slice);
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
            */
            time_slice_ = time_slice; 
            left_slice_ = time_slice;
        }

        int run() override {
            
            // Check for new arrivals and add to waiting queue
            while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                waiting_queue.push(job_queue_.front());
                job_queue_.pop();
            }
            
            // If no current job and waiting queue has jobs, get one
            if (current_job_.name == 0 && !waiting_queue.empty()) {
                current_job_ = waiting_queue.front();
                waiting_queue.pop();
                left_slice_ = time_slice_;
            }
            
            // If no job to run, advance time to next arrival
            if (current_job_.name == 0) {
                if (!job_queue_.empty()) {
                    current_time_ = job_queue_.front().arrival_time;
                    return run(); // Recursive call to handle the new arrival
                } else {
                    return -1; // All done
                }
            }
            
            // Record first run time if this is the first execution
            if (current_job_.service_time == current_job_.remain_time) {
                current_job_.first_run_time = current_time_;
            }
            
            // Store the job name to return (before any potential job switch)
            int executing_job = current_job_.name;
            
            // Execute for 1 time unit
            current_time_++;
            current_job_.remain_time--;
            left_slice_--;
            
            // After execution, check if job is completed
            if (current_job_.remain_time == 0) {
                current_job_.completion_time = current_time_;
                end_jobs_.push_back(current_job_);
                
                
                // Get next job if available
                if (!waiting_queue.empty()) {
                    current_job_ = waiting_queue.front();
                    waiting_queue.pop();
                    left_slice_ = time_slice_;
                    current_time_ += switch_time_; // Add context switch time
                } else {
                    current_job_ = Job(); // Reset to empty job
                }
            }
            // After execution, check if time slice is exhausted
            else if (left_slice_ == 0) {
                
                // Check for new arrivals before switching
                while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                    waiting_queue.push(job_queue_.front());
                    job_queue_.pop();
                }
                
                // Only switch if there are other jobs waiting
                if (!waiting_queue.empty()) {
                    // Put current job back to waiting queue
                    waiting_queue.push(current_job_);
                    
                    // Get next job from waiting queue
                    current_job_ = waiting_queue.front();
                    waiting_queue.pop();
                    left_slice_ = time_slice_;
                    current_time_ += switch_time_; // Add context switch time
                } else {
                    // No other jobs available, continue with current job but reset slice
                    left_slice_ = time_slice_;
                }
            }
            
            return executing_job;
        }
                
};

class FeedBack : public Scheduler {
private:
    std::queue<Job> queue[4]; // 각 요소가 하나의 큐인 배열 선언
    int quantum[4] = {1, 1, 1, 1};
    int left_slice_;
    int current_queue;

public:
    FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
        if (is_2i) {
            name = "FeedBack_2i";
        } else {
            name = "FeedBack_1";
        }
        /*
        * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
        * 나머지는 자유롭게 수정 및 작성 가능
        */
        // Queue별 time quantum 설정
        if (name == "FeedBack_2i") {
            quantum[0] = 1;
            quantum[1] = 2;
            quantum[2] = 4;
            quantum[3] = 8;
        }
    }

    int run() override {
        return current_job_.name;
    }
};

class Lottery : public Scheduler{
    private:
        int counter = 0;
        int total_tickets = 0;
        int winner = 0;
        std::mt19937 gen;  // 난수 생성기
        
    public:
        Lottery(std::list<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "Lottery";
            // 난수 생성기 초기화
            uint seed = 10; // seed 값 수정 금지
            gen = std::mt19937(seed);
            total_tickets = 0;
            for (const auto& job : job_list_) {
                total_tickets += job.tickets;
            }
        }

        int getRandomNumber(int min, int max) {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(gen);
        }

        int run() override {
            return current_job_.name;
        }
};


class Stride : public Scheduler{
    private:
        // 각 작업의 현재 pass 값과 stride 값을 관리하는 맵
        std::unordered_map<int, int> pass_map;  
        std::unordered_map<int, int> stride_map;  
        const int BIG_NUMBER = 10000; // stride 계산을 위한 상수 (보통 큰 수를 사용)

    public:
        Stride(std::list<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "Stride";
                    // job_list_에 있는 각 작업에 대해 stride와 초기 pass 값(0)을 설정
            for (auto &job : job_list_) {
                // stride = BIG_NUMBER / tickets (tickets는 0이 아님을 가정)
                stride_map[job.name] = BIG_NUMBER / job.tickets;
                pass_map[job.name] = 0;
            }
        }

        int run() override {
            return current_job_.name;
        }
};
