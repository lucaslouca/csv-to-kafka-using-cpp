#ifndef POLLER_BRIDGE_H
#define POLLER_BRIDGE_H

#include <thread>
#include "AbstractPoller.h"
#include "impl/PollResult.h"

/*
https://refactoring.guru/design-patterns/bridge
*/

class PollerBridge
{
public:
  PollerBridge(const PollerBridge &original);
  PollerBridge(const AbstractPoller &innerReader);
  inline void set_queue(SafeQueue<PollResult> *queue);
  bool start();
  void join() const;
  PollerBridge &operator=(const PollerBridge &original);
  ~PollerBridge();

private:
  AbstractPoller *m_poller_ptr;
  std::unique_ptr<std::thread> m_t;
};

inline void PollerBridge::set_queue(SafeQueue<PollResult> *queue)
{
  return m_poller_ptr->set_queue(queue);
}
#endif
