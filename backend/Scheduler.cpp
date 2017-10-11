#include "Scheduler.hpp"

Task::Task(uint16_t id, Function f, uint32_t init): pid(id), fun(f), rTime(init), vTime(init) {}

void Task::sleep(uint32_t time) {
  this->rTime += time;
  this->vTime += time;
}

String Task::toString() const {
  return "pid: " + String(this->pid) + "," +
      "real: " + String(this->rTime) + " ms, " +
      "virtual: " + String(this->vTime) + " ms";
}

Scheduler::Scheduler() {}

Scheduler::~Scheduler() {
  if (this->tasks != nullptr) {
    foreach<Task*>(this->tasks, [](Task *t) {
      delete t;
    });
    delete this->tasks;
  }
}

void Scheduler::begin() {}

uint16_t Scheduler::spawn(Function f) {
  uint16_t pid = this->lastPid;
  this->push(new List<Task*>(new Task(pid, f, millis()), nullptr));
  this->lastPid++;
  return pid;
}

List<Task*> *Scheduler::pop() {
  if (this->tasks != nullptr) {
    List<Task*> *first = this->tasks;
    this->tasks = first->next;
    first->next = nullptr;
    return first;
  } else {
    return nullptr;
  }
}

void Scheduler::push(List<Task*> *t) {
  t->next = this->tasks;
  this->tasks = t;

  while (t->next != nullptr && t->next->item->vTime < t->item->vTime) {
    Task *n = t->next->item;
    t->next->item = t->item;
    t->item = n;
    t = t->next;
  }
}

void Scheduler::run() {
  uint32_t currTime = millis();

  List<Task*> *first = this->pop();
  if (first == nullptr) {
    return;
  }

  Task *t = first->item;
  if (t->rTime > currTime) {
    return;
  }

  t->fun(t);
  t->sleep(millis() - currTime);
  this->push(first);
}

String Scheduler::monitor() const {
  String out = "Task monitor (" + String(millis()) + "ms):\r\n";
  foreach<Task*>(this->tasks, [&out](Task *t) {
    out += " - " + t->toString() + "\r\n";
  });
  return out;
}
