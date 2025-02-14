#include "obstacles_subscriber.h"

Grid *grid_;
Globals *globals_;
int shm_grid_fd;
int shm_globals_fd;
volatile std::sig_atomic_t shutdown_flag = 0;
int fd;

void writeToPipe(int fd, const Obstacle &obstacle)
{
    if (fd == -1)
    {
        throw std::system_error(EINVAL, std::system_category(), "Invalid file descriptor");
    }

    ssize_t bytes_written = write(fd, &obstacle, sizeof(Obstacle));
    if (bytes_written == -1)
    {
        throw std::system_error(errno, std::system_category(), "Failed to write to FIFO");
    }
}

std::shared_ptr<void> map_shared_memory(int shm_fd, size_t size)
{
    void *addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED)
    {
        throw std::system_error(errno, std::system_category(), "mmap failed");
    }
    return std::shared_ptr<void>(addr, [size](void *ptr)
                                 { munmap(ptr, size); });
}

class SharedMemoryHandle
{
public:
    SharedMemoryHandle(const std::string &shm_name, int oflag, mode_t mode)
    {
        shm_fd = shm_open(shm_name.c_str(), oflag, mode);
        if (shm_fd == -1)
        {
            throw std::system_error(errno, std::system_category(), "shm_open (attach) failed");
        }
    }

    ~SharedMemoryHandle()
    {
        if (shm_fd != -1)
        {
            close(shm_fd);
        }
    }

    // Delete copy constructor and copy assignment operator
    SharedMemoryHandle(const SharedMemoryHandle &) = delete;
    SharedMemoryHandle &operator=(const SharedMemoryHandle &) = delete;

    // Allow moving of the handle
    SharedMemoryHandle(SharedMemoryHandle &&other) noexcept : shm_fd(other.shm_fd)
    {
        other.shm_fd = -1; // Do not close the file descriptor twice
    }

    SharedMemoryHandle &operator=(SharedMemoryHandle &&other) noexcept
    {
        if (this != &other)
        {
            if (shm_fd != -1)
            {
                close(shm_fd);
            }
            shm_fd = other.shm_fd;
            other.shm_fd = -1;
        }
        return *this;
    }

    int get_fd() const { return shm_fd; }

private:
    int shm_fd;
};

void sigint_handler(int signal)
{
    shutdown_flag = 1;

    // Detach shared memory for Grid
    if (grid_ != nullptr)
    {
        munmap((void *)grid_, sizeof(Grid));
        std::cout << "Detached grid shared memory." << std::endl;
    }

    // Detach shared memory for Globals
    if (globals_ != nullptr)
    {
        munmap((void *)globals_, sizeof(Globals));
        std::cout << "Detached globals shared memory." << std::endl;
    }

    // Optionally close shared memory file descriptors if they are stored globally
    if (shm_grid_fd != -1)
    {
        close(shm_grid_fd);
        shm_grid_fd = -1;
        std::cout << "Closed grid shared memory file descriptor." << std::endl;
    }
    if (shm_globals_fd != -1)
    {
        close(shm_globals_fd);
        shm_globals_fd = -1;
        std::cout << "Closed globals shared memory file descriptor." << std::endl;
    }
    close(fd);
}

class ObstaclesSubscriber
{
private:
    DomainParticipant *participant_;

    Subscriber *subscriber_;

    DataReader *reader_;

    Topic *topic_;

    TypeSupport type_;

    class SubListener : public DataReaderListener
    {
    public:
        SubListener() : samples_(0) {}

        ~SubListener() override {}

        void on_subscription_matched(
            DataReader *reader,
            const SubscriptionMatchedStatus &info) override
        {
            if (info.current_count_change == 1)
            {
                std::cout << "Subscriber matched." << std::endl;
                eprosima::fastdds::rtps::LocatorList locators;
                reader->get_listening_locators(locators);
                for (const eprosima::fastdds::rtps::Locator &locator : locators)
                {
                    print_transport_protocol(locator);
                }
            }
            else if (info.current_count_change == -1)
            {
                std::cout << "Subscriber unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                          << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
            }
        }

        void on_data_available(
            DataReader *reader) override
        {
            SampleInfo info;
            if (reader->take_next_sample(&my_message_, &info) == eprosima::fastdds::dds::RETCODE_OK)
            {
                if (info.valid_data)
                {
                    Obstacle obstacle;
                    obstacle.x = static_cast<unsigned char>(my_message_.x());
                    obstacle.y = static_cast<unsigned char>(my_message_.y());
                    unsigned char id = static_cast<unsigned char>(my_message_.id());
                    samples_++;
                    std::cout << "Index: " << static_cast<unsigned int>(id)
                              << " X: " << static_cast<unsigned int>(obstacle.x)
                              << " Y: " << static_cast<unsigned int>(obstacle.y) << std::endl;
                    writeToPipe(fd, obstacle);
                }
            }
        }

    public:
        ObstacleMessage my_message_;

        std::atomic_int samples_;

    private:
        void print_transport_protocol(const eprosima::fastdds::rtps::Locator &locator)
        {
            switch (locator.kind)
            {
            case LOCATOR_KIND_UDPv4:
                std::cout << "Using UDPv4" << std::endl;
                break;
            case LOCATOR_KIND_UDPv6:
                std::cout << "Using UDPv6" << std::endl;
                break;
            case LOCATOR_KIND_SHM:
                std::cout << "Using Shared Memory" << std::endl;
                break;
            default:
                std::cout << "Unknown Transport" << std::endl;
                break;
            }
        }
    } listener_;

public:
    ObstaclesSubscriber()
        : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr), type_(new ObstacleMessagePubSubType())
    {
    }

    virtual ~ObstaclesSubscriber()
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //! Initialize the subscriber
    bool init()
    {
        DomainParticipantQos participantQos;
        participantQos.name("obstacle_subscriber");

        //  // Explicit configuration of shm transport
        // participantQos.transport().use_builtin_transports = false;
        // auto shm_transport = std::make_shared<SharedMemTransportDescriptor>();
        // shm_transport->segment_size(10 * 1024 * 1024);
        // participantQos.transport().user_transports.push_back(shm_transport);

        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

        if (participant_ == nullptr)
        {
            return false;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the subscriptions Topic
        topic_ = participant_->create_topic("obstacles", type_.get_type_name(), TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            return false;
        }

        // Create the Subscriber
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (subscriber_ == nullptr)
        {
            return false;
        }

        // Create the DataReader
        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);

        if (reader_ == nullptr)
        {
            return false;
        }

        return true;
    }

    //! Run the Subscriber
    void run()
    {
        while (!shutdown_flag)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

ObstaclesSubscriber *subscriber = new ObstaclesSubscriber();

int main()
{
    std::string fifoPath = "/tmp/obstacles";
    fd = open(fifoPath.c_str(), O_WRONLY);
    SharedMemoryHandle shm_grid_fd(SHM_GRID_NAME, O_RDWR, 0666);
    auto grid_ptr = std::static_pointer_cast<Grid>(map_shared_memory(shm_grid_fd.get_fd(), SHM_GRID_SIZE));
    if (!grid_ptr)
    {
        std::cerr << "Failed to map Grid shared memory." << std::endl;
        return false;
    }
    grid_ = grid_ptr.get();

    SharedMemoryHandle shm_globals_fd(SHM_G_NAME, O_RDWR, 0666);
    auto globals_ptr = std::static_pointer_cast<Globals>(map_shared_memory(shm_globals_fd.get_fd(), SHM_G_SIZE));
    if (!globals_ptr)
    {
        std::cerr << "Failed to map Globals shared memory." << std::endl;
        return false;
    }
    globals_ = globals_ptr.get();

    globals_->sub = getpid();
    std::cout << "Set PID in SHM: " << globals_->sub << std::endl;

    // Initialize the global subscriber
    if (subscriber->init())
    {
        std::cout << "Subscriber initialized successfully." << std::endl;
    }
    else
    {
        std::cerr << "Failed to initialize subscriber." << std::endl;
        return 1; // Return an error code
    }
    std::signal(SIGINT, sigint_handler); // Register signal handler

    while (!shutdown_flag)
    {
        pause();
    }

    delete subscriber;
    std::cout << "Subscriber shutdown." << std::endl;
    return 0;
}