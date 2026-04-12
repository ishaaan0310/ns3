#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MultiClientChat");

int main (int argc, char *argv[])
{
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer serverNode;
    serverNode.Create(1);

    NodeContainer clientNodes;
    clientNodes.Create(3); // 3 clients

    InternetStackHelper stack;
    stack.Install(serverNode);
    stack.Install(clientNodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    Ipv4AddressHelper address;

    std::vector<Ipv4InterfaceContainer> interfaces;

    for (int i = 0; i < 3; i++)
    {
        NodeContainer pair(serverNode.Get(0), clientNodes.Get(i));
        NetDeviceContainer devices = p2p.Install(pair);

        std::ostringstream subnet;
        subnet << "10.1." << i+1 << ".0";

        address.SetBase(subnet.str().c_str(), "255.255.255.0");
        interfaces.push_back(address.Assign(devices));
    }

    // Server
    uint16_t port = 9;
    UdpEchoServerHelper server(port);
    ApplicationContainer serverApp = server.Install(serverNode.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(15.0));

    // Clients
    for (int i = 0; i < 3; i++)
    {
        UdpEchoClientHelper client(interfaces[i].GetAddress(0), port);
        client.SetAttribute("MaxPackets", UintegerValue(5));
        client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        client.SetAttribute("PacketSize", UintegerValue(512));

        ApplicationContainer clientApp = client.Install(clientNodes.Get(i));
        clientApp.Start(Seconds(2.0 + i));
        clientApp.Stop(Seconds(15.0));
    }

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
