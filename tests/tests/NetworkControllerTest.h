#ifndef NETWORKCONTROLLERTEST_H_INCLUDED
#define NETWORKCONTROLLERTEST_H_INCLUDED

#include "trillek-game.hpp"
#include "trillek-scheduler.hpp"
#include "controllers/network/network-controller.hpp"
#include "gtest/gtest.h"

using trillek::network::NetworkController;

class NetworkControllerTest: public ::testing::Test {
public:
    NetworkControllerTest() {};

    void SetUp() {
#if defined(_MSC_VER)
        // Visio C++ rely on GLFW clock
        // create the window
        auto& os = trillek::TrillekGame::GetOS();
        os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);
        glGetError(); // clear errors
#endif

        // start the scheduler in another thread
        tp = std::thread(
                   &trillek::TrillekScheduler::Initialize,
                   &trillek::TrillekGame::GetScheduler(),
                   5,
                   std::ref(systems));

        // Start the client network layer
        trillek::TrillekGame::GetNetworkClient().Initialize();
        trillek::TrillekGame::GetNetworkClient().SetTCPHandler<trillek::network::CLIENT>();

        trillek::TrillekGame::GetNetworkServer().Initialize();
        trillek::TrillekGame::GetNetworkServer().SetTCPHandler<trillek::network::SERVER>();
        trillek::TrillekGame::GetNetworkServer().Server_Start("localhost", 7777);
    }

protected:
    std::queue<trillek::SystemBase*> systems;
    std::thread tp;
};


namespace trillek { namespace network {

TEST_F(NetworkControllerTest, Message) {
    EXPECT_FALSE(trillek::TrillekGame::GetNetworkClient().Connect("localhost", 7777, "my_login", "bad password")) << "Wrong password should not pass authentication";
    ASSERT_TRUE(trillek::TrillekGame::GetNetworkClient().Connect("localhost", 7777, "my_login", "secret password")) << "Good pasword should authenticate";

    for(auto i = 0; i < 10; ++i) {
            network::Message packet{};
            std::string str("This is a big very big text ! #");
            packet << str.append(std::to_string(i));
            packet.Send(TEST_MSG, TEST_MSG);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto pkt = TrillekGame::GetNetworkClient().GetPacketHandler().GetQueue<TEST_MSG,TEST_MSG,CLIENT>().Poll();
    ASSERT_EQ(10,pkt.size());
    TrillekGame::NotifyCloseWindow();
    tp.join();
}
}
}
#endif // NETWORKCONTROLLERTEST_H_INCLUDED
