//
// NetworkTopology.ned - Complete network topology for Adaptive Priority Router
//

package ned;

import ned.DatarateChannel;

network AdaptivePriorityNetwork
{
    parameters:
        @display("bgb=800,400");
    submodules:
        client[3]: TrafficGenerator {
            parameters:
                @display("p=100,100,row,100");
        }
        router: AdaptiveRouter {
            parameters:
                queueCapacity = default(200);
                congestionThreshold = default(0.7);
                recoveryThreshold = default(0.5);
                checkInterval = default(0.1s);
                @display("p=350,200");
        }
        server: Server {
            @display("p=600,200");
        }

    connections:
        for i=0..2 {
            client[i].out --> {  delay = uniform(0.001s, 0.005s); } --> router.in++;
        }
        router.out --> {  delay = uniform(0.001s, 0.005s); } --> server.in;
}