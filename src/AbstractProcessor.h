#ifndef ABSTRACT_PROCESSOR_H
#define ABSTRACT_PROCESSOR_H

#include "AbstractWorker.h"
#include <vector>
#include <transformers/AbstractTransformer.h>
#include <string>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <mutex>

class AbstractProcessor : public AbstractWorker
{
private:
  void step() override;
  virtual void clean() = 0;
  virtual void handle(PollResult d) = 0;

public:
  AbstractProcessor(std::string name, std::shared_ptr<SignalChannel> sig_channel);
  virtual ~AbstractProcessor();
  virtual AbstractProcessor *clone() const = 0;

protected:
  std::vector<std::unique_ptr<AbstractTransformer>> *m_transformers;
  std::atomic<size_t> *m_active_processors;
  std::condition_variable *m_log_cv;
  std::mutex *m_log_cv_mutex;
};

#endif
