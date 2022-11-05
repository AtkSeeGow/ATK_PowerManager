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
        private readonly Gauge controllerHeartbeat;
        private readonly Gauge hostHeartbeat;

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
            this.controllerHeartbeat = Metrics.CreateGauge("controller_heartbeat", "");
            this.hostHeartbeat = Metrics.CreateGauge("host_heartbeat", "");
        }

        public void RegisterRequest()
        {
            var controllerHeartbeatValidResult = new ValidResult();
            var hostHeartbeatValidResult = new ValidResult();

            Task.WhenAll(
                this.httpUtility.Heartbeat(controllerHeartbeatValidResult, new Uri(new Uri(serialOptions.Url), "/ReadDigitalValue/")),
                this.httpUtility.Heartbeat(hostHeartbeatValidResult, new Uri(hostOptions.Url))).Wait();

            this.controllerHeartbeat.IncTo(controllerHeartbeatValidResult.IsValid ? 1 : 0);
            this.hostHeartbeat.IncTo(hostHeartbeatValidResult.IsValid ? 1 : 0);
        }
    }
}