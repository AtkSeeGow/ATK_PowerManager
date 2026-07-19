using System;
using System.Collections.Generic;
using System.Linq;

namespace PowerManager.Domain.Devices
{
    /// <summary>
    /// 由伺服器主動呼叫裝置
    /// </summary>
    public class PassiveDevice : DeviceBase
    {
        public string Url { get; set; }

        public IList<int> MonitoredGpioPins { get; set; } = new List<int>();

        public Uri HeartbeatUri => new(new Uri(Url), "/api/heartbeat");

        public Uri GetDigitalValueUri(int pin)
        {
            return BuildUri("api/gpio", $"pin={pin}");
        }

        public Uri SetDigitalValueUri(int pin, bool value)
        {
            return BuildUri("api/gpio", $"pin={pin}&value={(value ? 1 : 0)}");
        }

        public Uri PulseDigitalValueUri(int pin, bool value, int durationMilliseconds)
        {
            return BuildUri(
                "api/gpio/pulse",
                $"pin={pin}&value={(value ? 1 : 0)}&durationMs={durationMilliseconds}");
        }

        public static PassiveDevice FindByName(
            IList<PassiveDevice> passiveDevices,
            string name)
        {
            return passiveDevices.FirstOrDefault(item => item.Name == name);
        }

        private Uri BuildUri(string path, string query)
        {
            var baseUri = new Uri(Url.EndsWith("/") ? Url : $"{Url}/");
            return new Uri(baseUri, $"{path}?{query}");
        }
    }
}
