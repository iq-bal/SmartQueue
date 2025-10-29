//
// Node module definitions
//

package ned;

simple TrafficGenerator
{
    parameters:
        string packetType = default("data");
        int priorityLevel = default(1);
        double sendInterval @unit(s) = default(exponential(0.1s));
        @class(::TrafficGenerator);
        @display("i=block/source");
        @signal[packetSent](type=long);
        @statistic[packetsSent](title="packets sent"; source=packetSent; record=count,vector);
    gates:
        output out;
}

simple Server
{
    parameters:
        @class(::Server);
        @display("i=block/sink");
        @signal[packetReceived](type=long);
        @signal[delay](type=double);
        @signal[voiceDelay](type=double);
        @signal[videoDelay](type=double);
        @signal[dataDelay](type=double);
        @statistic[packetsReceived](title="packets received"; source=packetReceived; record=count,vector);
        @statistic[endToEndDelay](title="end-to-end delay"; source=delay; unit=s; record=histogram,vector,mean,max);
        @statistic[voiceEndToEndDelay](title="voice end-to-end delay"; source=voiceDelay; unit=s; record=histogram,vector,mean,max);
        @statistic[videoEndToEndDelay](title="video end-to-end delay"; source=videoDelay; unit=s; record=histogram,vector,mean,max);
        @statistic[dataEndToEndDelay](title="data end-to-end delay"; source=dataDelay; unit=s; record=histogram,vector,mean,max);
    gates:
        input in;
}