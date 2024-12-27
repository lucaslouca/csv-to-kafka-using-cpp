#include "AbstractProcessor.h"

#include <iostream>

AbstractProcessor::AbstractProcessor(std::string name, std::shared_ptr<SignalChannel> sig_channel) : AbstractWorker(name, sig_channel)
{
}

void AbstractProcessor::step()
{
  PollResult d(""); // queue->dequeue();
  m_queue->dequeue_with_timeout(1000, d);
  if (!d.empty())
  {
    handle(d);
  }
}

AbstractProcessor::~AbstractProcessor()
{
  // std::cout << "AbstractProcessor: delete " << this << std::endl;
}
