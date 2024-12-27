#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "PollerBridge.h"
#include "ProcessorBridge.h"
#include "SafeQueue.h"
#include "impl/PollResult.h"

class Connector
{
public:
  Connector(const PollerBridge &poller, const std::vector<ProcessorBridge> processors);
  bool start();
  ~Connector();

private:
  PollerBridge m_poller;
  std::vector<ProcessorBridge> m_processors;
  SafeQueue<PollResult> m_queue;
};

#endif
