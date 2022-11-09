using PowerManager.Domain;
using PowerManager.Domain.Options;
using PowerManager.Web.Utility;
using Prometheus;
using System;
using System.Threading.Tasks;

namespace PowerManager.Web.Reporter
{
    public class HeartbeatMetric
    {
        private readonly Gauge heartbeat;

        private readonly HttpUtility httpUtility;
        private readonly SerialOptions serialOptions;
        private readonly HostOptions hostOptions;

        public HeartbeatMetric(
            HttpUtility httpUtility,
            SerialOptions serialOptions,
            HostOptions hostOptions)
        {
            this.httpUtility = httpUtility;
            this.serialOptions = serialOptions;
            this.hostOptions = hostOptions;
            this.heartbeat = Metrics.CreateGauge("heartbeat", "", "name");
        }

        public void RegisterRequest()
        {
            var controllerHeartbeat = new ValidResult();
            var hostHeartbeat = new ValidResult();

            Task.WhenAll(
                this.httpUtility.Heartbeat(controllerHeartbeat, new Uri(new Uri(serialOptions.Url), "/ReadDigitalValue/")),
                this.httpUtility.Heartbeat(hostHeartbeat, new Uri(hostOptions.Url))).Wait();

            this.heartbeat.WithLabels("controller").Set(controllerHeartbeat.IsValid ? 1 : 0);
            this.heartbeat.WithLabels("host").Set(hostHeartbeat.IsValid ? 1 : 0);
        }
    }
}