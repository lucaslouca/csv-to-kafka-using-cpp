/**
 * @file ProcessorBridge
 *
 * @brief Bridge to decouple the processor abstraction from the actual implementation.
 *
 * @author Lucas Louca
 *
 */
#ifndef PROCESSOR_BRIDGE_H
#define PROCESSOR_BRIDGE_H

#include <thread>
#include <memory>
#include "AbstractProcessor.h"
#include "impl/PollResult.h"

/*
https://refactoring.guru/design-patterns/bridge
*/

class ProcessorBridge
{
public:
  ProcessorBridge(const ProcessorBridge &original);

  // Conversion constructor
  ProcessorBridge(const AbstractProcessor &inner_processor);

  /*
  Another conversion constructor.

  Constructor with r-value reference to avoid (automatic) construction of a temporary when passing in
  a std::unique_ptr<AbstractProcessor>&& (when we std::move(...)) like in the example below:

  std::unique_ptr<AbstractProcessor> ptr = ...
  processors.emplace_back(std::move(ptr));

  Otherwise, without the '&&' (i.e. by value) would also work, but it would create a temporary from the
  passed in std::unique_ptr<AbstractProcessor>&&.

  Note:
  std::unique_ptr<AbstractProcessor> &innerProcessor does not work as we cannot bind a non-const
  reference to a r-value

  */
  ProcessorBridge(std::unique_ptr<AbstractProcessor> &&inner_processor);
  inline void set_queue(SafeQueue<PollResult> *queue);
  bool start();
  void join() const;
  ProcessorBridge &operator=(const ProcessorBridge &original);
  ~ProcessorBridge();

private:
  AbstractProcessor *m_processor_ptr;
  std::unique_ptr<std::thread> m_t;
};

inline void ProcessorBridge::set_queue(SafeQueue<PollResult> *queue)
{
  return m_processor_ptr->set_queue(queue);
}
#endif
