#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    bool useUdp = false;

    CommandLine cmd;
    cmd.AddValue("useUdp", "Use UDP instead of TCP", useUdp);
    cmd.Parse(argc, argv);

    std::cout << (useUdp ? "Running UDP Multi-Client\n" : "Running TCP Multi-Client\n");

    // Create nodes
    NodeContainer clients;
    clients.Create(3);

    NodeContainer server;
    server.Create(1);

    // Install internet stack
    InternetStackHelper stack;
    stack.Install(clients);
    stack.Install(server);

    // Point-to-point setup
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // Create links (IMPORTANT: separate links)
    NetDeviceContainer d1 = p2p.Install(clients.Get(0), server.Get(0));
    NetDeviceContainer d2 = p2p.Install(clients.Get(1), server.Get(0));
    NetDeviceContainer d3 = p2p.Install(clients.Get(2), server.Get(0));

    // Assign IPs
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i1 = address.Assign(d1);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i2 = address.Assign(d2);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i3 = address.Assign(d3);

    uint16_t port = 8080;
    std::string protocol = useUdp ? "ns3::UdpSocketFactory" : "ns3::TcpSocketFactory";

    // Server
    PacketSinkHelper sink(protocol,
        InetSocketAddress(Ipv4Address::GetAny(), port));

    ApplicationContainer serverApp = sink.Install(server.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // Client 1
    OnOffHelper client1(protocol,
        InetSocketAddress(i1.GetAddress(1), port));
    client1.SetAttribute("DataRate", StringValue("1Mbps"));
    client1.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer app1 = client1.Install(clients.Get(0));
    app1.Start(Seconds(2.0));
    app1.Stop(Seconds(10.0));

    // Client 2
    OnOffHelper client2(protocol,
        InetSocketAddress(i2.GetAddress(1), port));
    client2.SetAttribute("DataRate", StringValue("1Mbps"));
    client2.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer app2 = client2.Install(clients.Get(1));
    app2.Start(Seconds(3.0));
    app2.Stop(Seconds(10.0));

    // Client 3
    OnOffHelper client3(protocol,
        InetSocketAddress(i3.GetAddress(1), port));
    client3.SetAttribute("DataRate", StringValue("1Mbps"));
    client3.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer app3 = client3.Install(clients.Get(2));
    app3.Start(Seconds(4.0));
    app3.Stop(Seconds(10.0));

    std::cout << "Simulation started...\n";

    Simulator::Stop(Seconds(10.0));   // IMPORTANT
    Simulator::Run();

    std::cout << "Simulation finished!\n";

    Simulator::Destroy();
}
