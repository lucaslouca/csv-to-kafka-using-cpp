#include "ProcessorBridge.h"

#include <iostream>

#include "ThreadGuard.h"

ProcessorBridge::ProcessorBridge(const ProcessorBridge &original)
{
  m_processor_ptr = original.m_processor_ptr->clone();
}

ProcessorBridge::ProcessorBridge(const AbstractProcessor &inner_processor)
{
  m_processor_ptr = inner_processor.clone();
}

ProcessorBridge::ProcessorBridge(std::unique_ptr<AbstractProcessor> &&inner_processor)
{
  m_processor_ptr = inner_processor->clone();
}

bool ProcessorBridge::start()
{
  m_t = std::make_unique<std::thread>(&AbstractProcessor::run, m_processor_ptr);
  return true;
}

void ProcessorBridge::join() const { ThreadGuard g(*m_t); }

ProcessorBridge::~ProcessorBridge()
{
  delete m_processor_ptr;
}
