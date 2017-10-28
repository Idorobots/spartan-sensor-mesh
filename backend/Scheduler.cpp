#include "Scheduler.hpp"

Task::Task(Scheduler *s, Priority p, Function f): scheduler(s), priority(p), fun(f) {}

void Task::sleep(Timestamp ms) {
  this->state = SLEEPING;
  this->wTime = this->scheduler->now() + ms * 1000;
}

void Task::kill() {
  this->state = KILLED;
}

void Task::updateTime(Timestamp time) {
  this->rTime += time;
  this->vTime += time * this->priority;
}

char buf[21];

String uint64String(Timestamp num) {
  const char map[] = "0123456789";
  char *p = &buf[21];
  *p = 0;

  do {
    *(--p) = map[num % 10];
    num /= 10;
  } while (num != 0);

  return String(p);
}

String Task::toString() const {
  return "pid: 0x" + String((size_t) this, HEX) + ", " +
      "state: " + String(this->state) + ", " +
      "priority: " + String(this->priority) + ", " +
      "real: " + uint64String(this->rTime) + " us, " +
      "virtual: " + uint64String(this->vTime) + " us, " +
      "wake: " + uint64String(this->wTime) + " us";
}

Scheduler::Scheduler() {}

Scheduler::~Scheduler() {
  if (this->running != nullptr) {
    foreach<Task*>(this->running, [](Task *t) {
      delete t;
    });
    delete this->running;
  }
  if (this->waiting != nullptr) {
    foreach<Task*>(this->waiting, [](Task *t) {
      delete t;
    });
    delete this->waiting;
  }
}

Timestamp Scheduler::now() {
  Timestamp t = micros();

  if (t < this->prevTime) {
    this->overflows++;
  }
  this->prevTime = t;
  return t + this->overflows * 0x100000000ULL;
}

void Scheduler::begin() {}

Task* Scheduler::spawn(Priority priority, Function f) {
  Task *pid = new Task(this, priority, f);
  pid->updateTime(this->now());
  this->running = new List<Task*>(pid, this->running);
  return pid;
}

inline void pushBack(List<Task*> *list) {
  Task *n = list->next->item;
  list->next->item = list->item;
  list->item = n;
}

void Scheduler::reschedule() {
  List<Task*> *t = this->running;

  while (t != nullptr && t->next != nullptr && t->next->item->vTime < t->item->vTime) {
    pushBack(t);
    t = t->next;
  }
}

void Scheduler::rescheduleWaiting() {
  List<Task*> *t = this->waiting;

  while (t != nullptr && t->next != nullptr && t->next->item->wTime < t->item->wTime) {
    pushBack(t);
    t = t->next;
  }
}

inline void moveHead(List<Task*> **from, List<Task*> **to) {
  List<Task*> *head = *from;
  *from = head->next;
  head->next = *to;
  *to = head;
}

void Scheduler::wake(Timestamp time) {
  while (this->waiting != nullptr && this->waiting->item->wTime < time) {
    Task *t = this->waiting->item;
    t->state = RUNNING;
    t->wTime = 0;
    moveHead(&this->waiting, &this->running);
    this->reschedule();
  }
}

void Scheduler::run() {
  this->wake(now());

  if (this->running == nullptr) {
    return;
  }

  Task *t = this->running->item;

  Timestamp start = now();
  t->fun(t);
  Timestamp stop = now();

  switch (t->state) {
    case RUNNING:
      t->updateTime(stop - start);
      this->reschedule();
      break;

    case SLEEPING:
      moveHead(&this->running, &this->waiting);
      this->rescheduleWaiting();
      break;

    case KILLED: {
        List<Task*> *killed = this->running;
        this->running = killed->next;
        killed->next = nullptr;
        delete killed->item;
        delete killed;
      }
      break;
  }
}

String Scheduler::monitor() {
  String out = "Current time: " + uint64String(this->now()) + " us\r\n";
  out += "Running tasks:\r\n";
  foreach<Task*>(this->running, [&out](Task *t) {
    out += " - " + t->toString() + "\r\n";
  });
  out += "Waiting tasks:\r\n";
  foreach<Task*>(this->waiting, [&out](Task *t) {
    out += " - " + t->toString() + "\r\n";
  });
  return out;
}
