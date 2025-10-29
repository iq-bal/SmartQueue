//
// Router.ned - Adaptive//
// Router module definition
//

package ned;

simple AdaptiveRouter
{
    parameters:
        double congestionThreshold = default(0.7);
        double recoveryThreshold = default(0.5);
        int queueCapacity = default(200);
        double checkInterval @unit(s) = default(0.1s);
        @class(::AdaptiveRouter);
        @display("i=abstract/router");
        @signal[queueLength](type=long);
        @signal[packetDropped](type=long);
        @signal[modeChanged](type=long);
        @signal[delay](type=double);
        @statistic[queueLength](title="queue length"; source=queueLength; record=vector,mean,max);
        @statistic[packetsDropped](title="packets dropped"; source=packetDropped; record=count,vector);
        @statistic[routerMode](title="router mode"; source=modeChanged; record=vector);
        @statistic[processingDelay](title="processing delay"; source=delay; unit=s; record=histogram,vector,mean);
    gates:
        input in[];
        output out;
}