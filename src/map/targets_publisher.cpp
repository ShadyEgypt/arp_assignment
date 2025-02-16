#include "targets_publisher.h"
#define LOCAL_IP "192.168.0.125"
#define PORT 6000

Grid *grid_;
Globals *globals_;
int shm_grid_fd;
int shm_globals_fd;
volatile std::sig_atomic_t shutdown_flag = 0;

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
    exit(0);
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

class TargetsPublisher
{
private:
    DomainParticipant *participant_;
    Publisher *publisher_;
    Topic *topic_;
    DataWriter *writer_;
    TypeSupport type_;
    class PubListener : public DataWriterListener
    {
    public:
        std::atomic_int matched_;

        PubListener() : matched_(0) {}
        ~PubListener() override {}

        void on_publication_matched(DataWriter *writer, const PublicationMatchedStatus &info) override
        {
            if (info.current_count_change == 1)
            {
                matched_ = info.total_count;
                std::cout << "Publisher matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                matched_ = info.total_count;
                std::cout << "Publisher unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                          << " is not a valid value for PublicationMatchedStatus current count change." << std::endl;
            }
        }
    } listener_;

public:
    bool publish();
    TargetsPublisher() : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr), type_(new TargetMessagePubSubType) {}
    ~TargetsPublisher()
    {
        if (participant_)
        {
            participant_->delete_contained_entities();
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
    }

    bool init()
    {
        DomainParticipantQos participantQos;
        participantQos.name("target_publisher");

        // * Configure the current participant as SERVER
        participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::CLIENT;

        // * Add custom user transport with TCP port 0 (automatic port assignation)
        auto data_transport = std::make_shared<TCPv4TransportDescriptor>();
        data_transport->add_listener_port(0);
        participantQos.transport().user_transports.push_back(data_transport);

        // * Define the server locator to be on interface
        constexpr uint16_t server_port = PORT;
        Locator_t server_locator;
        IPLocator::setIPv4(server_locator, LOCAL_IP);
        IPLocator::setPhysicalPort(server_locator, server_port);
        IPLocator::setLogicalPort(server_locator, server_port);

        // *Add the server
        participantQos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(server_locator);
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);
        if (!participant_)
        {
            std::cerr << "Failed to create participant!" << std::endl;
            return false;
        }

        // Initialize other components like topic, publisher, writer, etc.
        type_.register_type(participant_);

        topic_ = participant_->create_topic("targets", type_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (!topic_)
            return false;

        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if (!publisher_)
            return false;

        writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);
        if (!writer_)
            return false;

        return true;
    }
};

bool TargetsPublisher::publish()
{
    std::cout << "Targets: " << grid_->target_count << std::endl;
    // listener_.matched_ > 0 &&
    if (grid_ != nullptr)
    {
        for (int i = 0; i < grid_->target_count; ++i)
        {
            TargetMessage msg;
            msg.id(i);
            msg.x(static_cast<unsigned long>(grid_->targets[i].x));
            msg.y(static_cast<unsigned long>(grid_->targets[i].y));
            writer_->write(&msg); // Assuming `write` requires a pointer to the data
            std::cout << "Published target ID: " << msg.id() << " at position (" << msg.x() << ", " << msg.y() << ")" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return true;
    }
    else
    {
        std::cout << "No targets to publish or grid pointer is null." << std::endl;
    }
    return false;
}

TargetsPublisher publisher;

void signal_handler(int sig)
{
    if (sig == SIGUSR1)
    {
        std::cout << "SIGUSR1 received, starting publishing..." << std::endl;
        // TargetsPublisher should be a globally accessible object or managed differently
        publisher.publish();
    }
}

int main()
{
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

    globals_->pub = getpid();
    std::cout << "Set PID in SHM: " << globals_->pub << std::endl;

    // Initialize the global publisher
    if (publisher.init())
    {
        std::cout << "Publisher initialized successfully." << std::endl;
    }
    else
    {
        std::cerr << "Failed to initialize publisher." << std::endl;
        return 1; // Return an error code
    }
    signal(SIGUSR1, signal_handler);
    std::signal(SIGINT, sigint_handler); // Register signal handler

    while (!shutdown_flag)
    {
        pause();
    }

    return 0;
}
