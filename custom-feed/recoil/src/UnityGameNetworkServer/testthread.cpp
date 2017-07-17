#include <thread>

class GameNetworkServer
{
    public:
    GameNetworkServer()
    {
        _threadStarted=true;
        _updateThread = new std::thread(&GameNetworkServer::Update, this);

    }

    ~GameNetworkServer()
    {
        delete _updateThread;
    }

    void Update();
    void StopThread();

    private:
        std::thread* _updateThread;
        bool _threadStarted;
      
};

void GameNetworkServer::Update()
{
    while(_threadStarted)
    {
    }
}

void GameNetworkServer::StopThread()
{
    _threadStarted=false;
    _updateThread->join();
}

int main()
{
    GameNetworkServer testServer;
    testServer.StopThread();
    return 0;
}
