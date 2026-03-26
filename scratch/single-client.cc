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

    std::cout << (useUdp ? "Running UDP\n" : "Running TCP\n");

    // Create nodes
    NodeContainer nodes;
    nodes.Create(2); // 1 client + 1 server

    // Install internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Create point-to-point link
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices = p2p.Install(nodes);

    // Assign IP
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    uint16_t port = 8080;
    std::string protocol = useUdp ? "ns3::UdpSocketFactory" : "ns3::TcpSocketFactory";

    // Server (PacketSink)
    PacketSinkHelper sink(protocol,
        InetSocketAddress(Ipv4Address::GetAny(), port));

    ApplicationContainer serverApp = sink.Install(nodes.Get(1));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // Client (OnOff)
    OnOffHelper client(protocol,
        InetSocketAddress(interfaces.GetAddress(1), port));

    client.SetAttribute("DataRate", StringValue("1Mbps"));
    client.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp = client.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    std::cout << "Simulation started...\n";

    Simulator::Stop(Seconds(10.0));   // IMPORTANT
    Simulator::Run();

    std::cout << "Simulation finished!\n";

    Simulator::Destroy();
}
