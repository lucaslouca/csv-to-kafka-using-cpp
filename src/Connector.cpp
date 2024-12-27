#include "Connector.h"
#include "logging/Logging.h"
#include "Version.h"
#include <iostream>

static std::string name = "Connector";

Connector::Connector(const PollerBridge &poller_,
                     const std::vector<ProcessorBridge> processors_)
    : m_poller(poller_), m_processors(processors_)
{
  m_poller.set_queue(&m_queue);
  for (auto &processor : m_processors)
  {
    processor.set_queue(&m_queue);
  }
}

bool Connector::start()
{
  std::stringstream ss;
  ss << "Started - Git version: "
     << " tag="
     << GIT_TAG
     << ", revision="
     << GIT_REV
     << ", branch="
     << GIT_BRANCH;

  Logging::INFO(ss.str(), name);

  m_poller.start();
  for (auto &processor : m_processors)
  {
    processor.start();
  }

  // Join
  m_poller.join();
  for (auto &processor : m_processors)
  {
    processor.join();
  }
  return true;
}

Connector::~Connector() {}
