# Flycatcher


## Summary
A poller is created that enqueues CSV file paths into an in-memory queue, which is processed by a global pool of processor threads. The following steps highlight the functioning of the Poller framework:
1. A directory poller waits for new files in the configured input directory.
2. For each new file that the poller detects in the inbound directory, the poller enqueues the file path into an internal in-memory queue. 
3. A global pool of processor worker threads wait to process from the in-memory queue.
4. Processor worker threads pick up files from the internal queue, and perform the following actions:
   * Stream the file content to appropriate transformers.
   * Publish the result to the configured Kafka topic, serialized in Avro binary format.
   * Perform the required postprocessing, such as archiving the processed file.

## Configuration
Configuration is done in a single YAML file.

### Kafka
```yaml
kafka:
  bootstrap.servers: localhost:9092
  schema.registry.url: http://localhost:8081 # Required for fetching the schema ID for inclusion in the serialized message
  client.id: myclientid
```
### Transformations
You can due all sorts of transformations to the CSV column values before the final results is published. These can be defined as follows:
```yaml
transforms:
   # For column 'myid': remove any dashes and transform text to lowercase. i.e.: 
   # 99F3C0EA-57D8-4E04-9830-2E9226486F96 → 99f3c0ea57d84e0498302e9226486f96
  - type: unuuid
    column: myid

   # For column 'myid_type': change value '0' to "prefix_a:" and '1' to "prefix_b:"
  - type: map
    column: myid_type
    lookup:
      "0": "prefix_a:"
      "1": "prefix_b:"
   
   # For column 'myid_type': append the value from column 'myid' 
   # to the value at column 'myid_type' after being done with the 
   # previous transformation. i.e.: 
   # prefix_a: →  prefix_a:99f3c0ea57d84e0498302e9226486f96
  - type: append
    column: myid_type
    from_column: myid
```
### Avro Schema
The Avro schema is generated programmatically from the configuration file.

```yaml
# The Avro record field type for column if it should not be 'string'
column_type_transforms:
  latitude: float
  longitude: float
  timestamp: long

# Beginning of actual schema definition
type_map:
  lsm_in: # The Kafka topic we want to publish our messages
    key_column: maid
    columns: # Which CSV columns we want to include in the message (after mapping)
    - myid_type 
    - timestamp
    - latitude
    - longitude
    - country

# In case some CSV column names differ from Avro record field names,
# you can map them here
column_map:
  myid_type: userid
```

The above configuration will create the following Avro schema:

```json
{
    "type": "record",
    "name": "lsm_in_msg",
    "fields": [
        {
            "name": "userid",
            "type": "string"
        },
        {
            "name": "timestamp",
            "type": "long"
        },
        {
            "name": "latitude",
            "type": "float"
        },
        {
            "name": "longitude",
            "type": "float"
        },
        {
            "name": "country",
            "type": "string"
        }
    ]
}
```

## Dev Dependencies
### Debian
Make sure to build and install the libraries from source code as done below since the make script will look for the static libraries for statically linking them with our executable.  Static libraries often do not get installed when using `apt`.

```shell
# Install yaml-cpp
#sudo apt-get install libyaml-cpp-dev
curl -L https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.tar.gz -o yaml-cpp.tar.gz  && tar -xvf yaml-cpp.tar.gz && cd yaml-cpp-0.8.0 && mkdir build && cd build && cmake .. && make && sudo make install

# Install librdkafka
# sudo apt-get install librdkafka-dev # does not contain static lib!
sudo apt-get install libsnappy-dev

curl https://github.com/edenhill/librdkafka/archive/refs/tags/v1.9.2.tar.gz -o librdkafka.tar.gz  && \
tar -xvf librdkafka.tar.gz && \
cd librdkafka && ./configure && make && sudo make install && ldconfig

# Install avro-cpp under /usr/local/lib
git clone https://github.com/apache/avro && \
cd avro/lang/c++ && ./build.sh test && sudo ./build.sh install

# Install cpprestsdk
# sudo apt-get install libcpprest-dev # does not contain static lib!
curl https://github.com/microsoft/cpprestsdk/archive/refs/tags/2.10.18.tar.gz -o cpprestsdk.tar.gz  && \
tar -xvf cpprestsdk.tar.gz && \
cd cpprestsdk && cmake -DBUILD_SHARED_LIBS=OFF && make && sudo make install

# Install spdlog
# sudo apt install libspdlog-dev # does not contain static lib!
curl https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.tar.gz -o spdlog.tar.gz  && \
tar -xvf spdlog.tar.gz && \
cd spdlog && mkdir build && cd build && cmake .. && make -j && sudo make install

# Install libserdes: requires jansson, libcurl, avro-c and avro-cpp
# Install jansson
curl https://github.com/akheron/jansson/archive/refs/tags/v2.14.tar.gz -o jansson.tar.gz  && \
tar -xvf jansson.tar.gz && \
cd jansson && mkdir build && cd build && cmake .. && make && sudo make install

# Install avro-c
curl https://dlcdn.apache.org/avro/stable/c/avro-c-1.11.1.tar.gz -o avro-c.tar.gz && \
tar -xvf avro-c.tar.gz && \
cd avro-c && \
mkdir build && cd build && cmake .. && make && sudo make install

# Requires jansson, libcurl, avro-c and avro-cpp
git clone https://github.com/confluentinc/libserdes && \
# You might need to modify src/tinycthread.c/.h and rename cnd_timedwait_ms() to cnd_timedwait_ms_myserdes() and thrd_is_current() to thrd_is_current_myserdes() before building
# to avoid multiple definition errors when linking. This is due to a conflict with librdkafka. 
cd libserdes && ./configure --install-deps && make && sudo make install
```

### macOS
```shell
# Install yaml-cpp
brew install yaml-cpp

# Install librdkafka
git clone https://github.com/edenhill/librdkafka && \
cd librdkafka && ./configure --install-deps && make && sudo make install

# Install avro-cpp under /usr/local
git clone https://github.com/apache/avro && \
cd avro/lang/c++ && ./build.sh test && sudo ./build.sh install

# Install cpprestsdk
brew install cryptol
brew install cpprestsdk

# Install libserdes
# Requires jansson, libcurl, avro-c and avro-cpp
brew install snappy

curl https://dlcdn.apache.org/avro/stable/c/avro-c-1.11.1.tar.gz -o avro-c-1.11.1.tar.gz && \
tar -xvf avro-c-1.11.1.tar.gz && \
cd avro-c-1.11.1 && \
mkdir build && \
cd build && \
cp /opt/homebrew/Cellar/snappy/1.1.9/include/snappy-c.h ../src &&\
cmake ../ && \
make && \
sudo make install && \

brew install jansson
brew install curlpp

git clone https://github.com/confluentinc/libserdes && \
cd libserdes && ./configure --install-deps ./configure --CPPFLAGS="-I/opt/homebrew/include" --LDFLAGS="-L/opt/homebrew/lib" && \ 
make && \
sudo make install
```

## IDE Setup (Visual Studio Code)

1. Install `clang-format`:
   ```shell
   brew install clang-format

   # Find clang-format location
   which clang-format
   ```
2. Install `Clang-Format` VS Code extension by xaver
3. Modify `.vscode/settings.json` to point to the right `clang-format` binary
4. Install `CodeLLDB` VS Code extension by Vadim Chugunov

## Setup Kafka and Schemas [Locally]
### Without Docker
You can fireup Zookeeper and Kafka by simply downloading Kafka and running the below command:
```shell
# Start Zookeeper
./bin/zookeeper-server-start.sh config/zookeeper.properties

# Start Kafka
./bin/kafka-server-start.sh config/server.properties

# Create a topic
./bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --topic lsm_in_test

# Consume messages from the topic
./bin/kafka-console-consumer.sh --bootstrap-server localhost:9092 --topic lsm_in_test --from-beginning
```
It is recommended to just spin up a docker container(s) as described in the next section which also comes with the required schema regsitry and an (optional) web based control center for managing topics and schemas.

### With Docker
In the project's root directory run in Terminal:
```shell
docker-compose up -d
```

#### Create a Topic
Go to http://localhost:9021 → _Cluster 1_ → _Topics_. → _Create topic_ to create a new topic named: _lsm_in_test_. Click on _Create with defaults_.

#### Define the Topic Schema
This step is not required as flycatcher generates and registers schemas programmatically.

Set the _key_ schema to simply be `"string"` a save it. For the _value_ schema use `code-samples/lsm_in_test_schema.json`.

#### Delete a Schema from the Schema Registry
I use Postman to do an HTTP `DELETE` request on the following resource:
```shell
DELETE 10.105.37.13:8091/subjects/lsm_in_test-value/
```


#### Kafdrop - Kafka Web UI
Kafdrop is a web UI for viewing Kafka topics and browsing consumer groups. The tool displays information such as brokers, topics, partitions, consumers, and lets you view messages.

Download Kafdrop from [here](https://github.com/obsidiandynamics/kafdrop) and run it:

```shell
java --add-opens=java.base/sun.nio.ch=ALL-UNNAMED \
    -jar kafdrop-3.30.0.jar \
    --kafka.brokerConnect=localhost:9092 \
    --schemaregistry.connect=http://localhost:8081
```

### Track CPU Usage
#### Linux
```shell
top -H -p $(pgrep flycatcher)
```

#### Mac OS X
```shell
brew install htop
htop -p $(pgrep flycatcher)
```