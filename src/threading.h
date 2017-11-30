#pragma once
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool;
void workerFunction(ThreadPool *pool);

class TaskStatus {
public:
  TaskStatus() : isDone(false){};

  void get() {
    // spinlocking on atomic variable
    while (isDone == false) {
    }
  }

  void taskDone() { isDone = true; }

private:
  std::atomic<bool> isDone;
};

class Task {
public:
  virtual void evaluate() = 0;
  std::unique_ptr<TaskStatus> status;
};

template <typename T> class SpecificTask : public Task {
public:
  SpecificTask(T ptr) : Task(), body(ptr) { status.reset(new TaskStatus()); }
  virtual ~SpecificTask() = default;
  void evaluate() override { (*body)(); }

private:
  T body;
};

class ThreadPool {
public:
  ThreadPool(int threadCount);
  inline bool hasTasks() const { return q.empty(); }

  // TEST FUNCTIONS I WILL REMOVE THEM LATER
  // this is an example of function that needs to merge multiple result, thats
  // why we pass in a queue ref
  template <typename T>
  void addRandomTasks(T funcPointer, std::queue<TaskStatus *> &resQ) {
    // std::unique_lock<std::mutex> queue_lock(queue_mutex);

    {
      std::unique_lock<std::mutex> queue_lock(queue_mutex);
      for (int x = 1; x <= 30; ++x) {

        SpecificTask<T> *curr = new SpecificTask<T>(funcPointer);
        resQ.push(curr->status.get());
        q.push(curr);
      }
    }
    sleepCondition.notify_all();
  }

  // TEST FUNCTION I WILL REMOVE THEM LATER
  template <typename T> TaskStatus *addExpensiveTast(T funcPointer) {

    SpecificTask<T> *curr = new SpecificTask<T>(funcPointer);
    auto *toReturn = curr->status.get();
    {
      std::unique_lock<std::mutex> queue_lock(queue_mutex);
      q.push(curr);
      std::cout << "added expensive task" << std::endl;
    }
    sleepCondition.notify_all();
    return toReturn;
  }

public:
  int m_threadCount;
  std::vector<std::thread> workers;
  std::queue<Task *> q;
  std::mutex queue_mutex;
  std::condition_variable sleepCondition;
};

ThreadPool::ThreadPool(int threadCount) : m_threadCount(threadCount) {
  for (int i = 0; i < m_threadCount; ++i) {
    // hear creating the threads
    workers.emplace_back(std::thread(workerFunction, this));
  }
}

void workerFunction(ThreadPool *pool) {

  while (true) {
    Task *task = nullptr;
    {
      std::unique_lock<std::mutex> queue_lock(pool->queue_mutex);
      if (!pool->q.empty()) {
        task = pool->q.front();
        pool->q.pop();
      }
    }
    if (task != nullptr) {
      task->evaluate();
      task->status->taskDone();
    } else {
      // here we go into sleep
      std::unique_lock<std::mutex> locker(pool->queue_mutex);
      pool->sleepCondition.wait(locker);
    }
  }
}

// test function to remove in the future
// void simpleFunct() {
//	int accum = 0;
//	for (int i = 0; i < 50000 * 10000; ++i) {
//		accum += i;
//	}
//	std::cout << "processed task with number   result is " << accum <<
// std::endl;
//}

// struct Test
//{
//	//this is the actual operator doing the work
//	void operator()()
//	{
//		std::queue<TaskStatus*>subQ;
//		pool->addRandomTasks(&simpleFunct, subQ);
//		while (!subQ.empty())
//		{
//			auto* ff = subQ.front();
//			ff->get();
//			subQ.pop();
//		}
//		std::cout << "all sub-tasks terminated" << std::endl;
//	}
//	ThreadPool* pool;
//};

// int main() {
//	ThreadPool pool(16);
//	std::cout << "done init" << std::endl;

//	while (true) {

//		Test t{ &pool };
//		int number;
//		std::cin >> number;
//		auto* status = pool.addExpensiveTast(&t);
//		status->get();
//		std::cout << "lock terminated" << std::endl;
//	}
//	return 0;
//}
