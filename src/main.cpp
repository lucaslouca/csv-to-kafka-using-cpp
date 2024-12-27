
#include "logging/Logging.h"
#include "Connector.h"
#include "SignalChannel.h"
#include "impl/CsvProcessor.h"
#include "impl/CsvProcessorBuilder.h"
#include "impl/DirectoryPollerBuilder.h"
#include "impl/KafkaPoller.h"
#include "impl/KafkaDeliveryReportCb.h"
#include "config/ConfigParser.h"
#include <librdkafka/rdkafkacpp.h>
#ifdef __linux__
#include "impl/DirectoryPoller.h"
#elif __APPLE__
#include "impl/DirectoryPollerMacOS.h"
#endif
#include <iostream>
#include <yaml-cpp/yaml.h>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <getopt.h>
#include <signal.h>
#include <future> // for async()
#include <sys/types.h>
#include <sys/stat.h>

static std::string name = "main";

/**
 * Print usage.
 *
 */
void usage(const std::string me)
{
  std::cerr << "Usage: " << me << " [options]\n"
                                  "Produces Avro encoded messages to Kafka from CSV objects\n"
                                  "\n"
                                  "Options:\n"
                                  " -d <directory>    Watch directory\n"
                                  " -c <config>       Configuration file\n"
                                  "\n"
                                  "\n"
                                  "Example:\n"
                                  "  "
            << me << " -d tmp -c lsm2kafka.yaml\n\n";
  exit(1);
}

/**
 * Create a return a shared channel for SIGINT signals.
 *
 */
std::shared_ptr<SignalChannel> listen_for_sigint(sigset_t &sigset)
{
  std::shared_ptr<SignalChannel> sig_channel = std::make_shared<SignalChannel>();

#ifdef __linux__
  // Listen for sigint event line Ctrl^c
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigaddset(&sigset, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

  std::thread signal_handler{
      [&sig_channel, &sigset]()
      {
        int signum = 0;

        // wait untl a signal is delivered
        sigwait(&sigset, &signum);
        sig_channel->m_shutdown_requested.store(true);

        // notify all waiting workers to check their predicate
        sig_channel->m_cv.notify_all();
        std::cout << "Received signal " << signum << "\n";
        return signum;
      }};
  signal_handler.detach();
#elif __APPLE__
  std::thread signal_handler{
      [&sig_channel]()
      {
        int kq = kqueue();

        /* Two kevent structs */
        struct kevent *ke = (struct kevent *)malloc(sizeof(struct kevent));

        /* Initialise struct for SIGINT */
        signal(SIGINT, SIG_IGN);
        EV_SET(ke, SIGINT, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);

        /* Register for the events */
        if (kevent(kq, ke, 1, NULL, 0, NULL) < 0)
        {
          perror("kevent");
          return false;
        }

        memset(ke, 0x00, sizeof(struct kevent));

        // Camp here for event
        if (kevent(kq, NULL, 0, ke, 1, NULL) < 0)
        {
          perror("kevent");
        }

        switch (ke->filter)
        {
        case EVFILT_SIGNAL:
          std::cout << "Received signal " << strsignal(ke->ident) << "\n";
          sig_channel->m_shutdown_requested.store(true);
          sig_channel->m_cv.notify_all();
          break;
        default:
          break;
        }

        return true;
      }};
  signal_handler.detach();
#endif

  return sig_channel;
}

/**
 * Parse commandline arguments and fill in config file path and directory to watch.
 *
 */
void parse_args(int argc, char *argv[], std::string &config_file, std::string &dir_to_watch)
{
  int opt;
  while ((opt = getopt(argc, argv, "d:c:")) != -1)
  {
    switch (opt)
    {
    case 'd':
      dir_to_watch = optarg;
      break;
    case 'c':
      config_file = optarg;
      break;
    default:
      std::cerr << "Unknown option -" << (char)opt << std::endl;
      usage(argv[0]);
    }
  }

  if (dir_to_watch.empty())
  {
    std::cerr << "Directory to watch cannot be empty" << std::endl;
    exit(1);
  }
  else
  {
    struct stat info;
    if (stat(dir_to_watch.c_str(), &info) != 0)
    {
      std::cerr << "Cannot access '" << dir_to_watch << "'" << std::endl;
      exit(1);
    }
    else if (!(info.st_mode & S_IFDIR))
    {
      std::cerr << "'" << dir_to_watch << "' is not a directory" << std::endl;
      exit(1);
    }
  }

  if (config_file.empty())
  {
    std::cerr << "A config file must be provided" << std::endl;
    exit(1);
  }
  else
  {
    struct stat info;
    if (stat(config_file.c_str(), &info) != 0)
    {
      std::cerr << "Cannot access '" << config_file << "'" << std::endl;
      exit(1);
    }
    else if (info.st_mode & S_IFDIR)
    {
      std::cerr << "'" << config_file << "' is a directory" << std::endl;
      exit(1);
    }
  }
}

int main(int argc, char *argv[])
{
  std::string config_file;
  std::string dir_to_watch;

  /*************************************************************************
   *
   * COMMANDLINE ARGUMENTS
   *
   *************************************************************************/
  parse_args(argc, argv, config_file, dir_to_watch);

  /*************************************************************************
   *
   * SIGINT CHANNEL
   *
   *************************************************************************/
  sigset_t sigset;
  std::shared_ptr<SignalChannel> sig_channel = listen_for_sigint(sigset);

  /*************************************************************************
   *
   * LOGGER
   *
   *************************************************************************/
  std::atomic<size_t> active_processors = 0;
  std::condition_variable log_cv;
  std::mutex log_cv_mutex;
  Logging::LogProcessor log_processor(&active_processors, &log_cv, &log_cv_mutex);

  log_processor.start();

  /*************************************************************************
   *
   * CONFIGURATION
   *
   *************************************************************************/
  ConfigParser &config = ConfigParser::instance(config_file);

  /*************************************************************************
   *
   * KAFKA
   *
   *************************************************************************/
  std::map<std::string, SchemaConfig> schemas = config.schemas();
  std::map<std::string, std::string> kafka_config = config.kafka();

  RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  std::string errstr;
  if (conf->set("bootstrap.servers", kafka_config["bootstrap.servers"], errstr) != RdKafka::Conf::CONF_OK)
  {
    Logging::ERROR(errstr, name);
    kill(getpid(), SIGINT);
  }
  if (conf->set("client.id", kafka_config["client.id"], errstr) != RdKafka::Conf::CONF_OK)
  {
    Logging::ERROR(errstr, name);
    kill(getpid(), SIGINT);
  }

  /* Set the delivery report callback.
   * This callback will be called once per message to inform
   * the application if delivery succeeded or failed.
   * See dr_msg_cb() above.
   * The callback is only triggered from ::poll() and ::flush().
   *
   * IMPORTANT:
   * Make sure the DeliveryReport instance outlives the Producer object,
   * either by putting it on the heap or as in this case as a stack variable
   * that will NOT go out of scope for the duration of the Producer object.
   */
  KafkaDeliveryReportCb ex_dr_cb;
  if (conf->set("dr_cb", &ex_dr_cb, errstr) != RdKafka::Conf::CONF_OK)
  {
    Logging::ERROR(errstr, name);
    kill(getpid(), SIGINT);
  }

  RdKafka::Producer *kafka_producer = RdKafka::Producer::create(conf, errstr);
  if (!kafka_producer)
  {
    Logging::ERROR("Failed to create Kafka producer: " + errstr, name);
    kill(getpid(), SIGINT);
  }

  KafkaPoller kafka_poller(kafka_producer, sig_channel);
  kafka_poller.start();

  /*************************************************************************
   *
   * DIRECTORY WATCHER
   *
   *************************************************************************/
  DirectoryPoller poller = DirectoryPoller::builder("DirectoryPoller")
                               .with_directory(dir_to_watch)
                               .with_sig_channel(sig_channel)
                               .build();

  /*************************************************************************
   *
   * FILE PROCESSORS
   *
   *************************************************************************/
  unsigned int processor_thread_count = std::max<unsigned int>(1, std::thread::hardware_concurrency() - 5); // - main, LogProcessor, DirectoryPoller, KafkaPoller, Signal

  std::vector<std::unique_ptr<AbstractTransformer>> transformers = config.transformers();
  std::vector<ProcessorBridge> processors;

  std::pair<std::string, int> ma;
  if (config.has_key("max_age"))
  {
    ma = config.max_age();
  }

  for (size_t i = 1; i <= processor_thread_count; ++i)
  {
    auto builder = CsvProcessor::builder("CsvProcessor " + std::to_string(i))
                       .with_active_processors_counter(&active_processors)
                       .with_logging_cv(&log_cv)
                       .with_logging_mutex(&log_cv_mutex)
                       .with_transformers(&transformers)
                       .with_kafka_producer(kafka_producer)
                       .with_schemas(&schemas)
                       .with_sig_channel(sig_channel);

    if (config.has_key("max_age"))
    {
      builder.with_drop_max_age(&ma);
    }

    std::unique_ptr<AbstractProcessor> ptr = builder.build();
    processors.emplace_back(std::move(ptr));
  }
  Logging::INFO("Spawned " + std::to_string(processor_thread_count) + " processor threads", name);

  /*************************************************************************
   *
   * START MAIN LOOP
   *
   *************************************************************************/
  Connector connector(poller, processors);
  connector.start();

  /*************************************************************************
   *
   * SHUTDOWN
   *
   *************************************************************************/
  kafka_poller.join();

  log_processor.stop();
  log_processor.join();

  delete conf;
  delete kafka_producer;
  return 0;
}
