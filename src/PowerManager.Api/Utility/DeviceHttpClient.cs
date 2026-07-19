using Microsoft.Extensions.Logging;
using PowerManager.Domain;
using System;
using System.Net.Http;
using System.Threading.Tasks;

namespace PowerManager.Api.Utility
{
    public class DeviceHttpClient
    {
        private readonly ILogger<DeviceHttpClient> _logger;
        private readonly IHttpClientFactory _httpClientFactory;

        public DeviceHttpClient(
            ILogger<DeviceHttpClient> logger,
            IHttpClientFactory httpClientFactory)
        {
            _logger = logger;
            _httpClientFactory = httpClientFactory;
        }

        public Task GetAsync(DeviceOperationResult result, Uri uri)
        {
            var httpClient = _httpClientFactory.CreateClient();
            return SendAsync(result, HttpMethod.Get, uri, httpClient);
        }

        public Task PostAsync(DeviceOperationResult result, Uri uri)
        {
            var httpClient = _httpClientFactory.CreateClient();
            return SendAsync(result, HttpMethod.Post, uri, httpClient);
        }

        public Task CheckHeartbeatAsync(DeviceOperationResult result, Uri uri)
        {
            var httpClient = _httpClientFactory.CreateClient("Heartbeat");
            return SendAsync(result, HttpMethod.Get, uri, httpClient);
        }

        private async Task SendAsync(
            DeviceOperationResult result,
            HttpMethod method,
            Uri uri,
            HttpClient httpClient)
        {
            try
            {
                using var request = new HttpRequestMessage(method, uri);
                using var response = await httpClient.SendAsync(request);

                if (response.IsSuccessStatusCode)
                {
                    var value = (await response.Content.ReadAsStringAsync())
                        .Replace("\r", "")
                        .Replace("\n", "");
                    result.SuccessMessages.Add("Value", value);
                    return;
                }

                var error = $"RequestUri:{uri}, StatusCode:{response.StatusCode}";
                _logger.LogDebug(error);
                result.ErrorMessages.Add(Guid.NewGuid().ToString(), error);
            }
            catch (Exception exception)
            {
                _logger.LogError(exception, "Device request to {RequestUri} failed.", uri);
                result.ErrorMessages.Add(
                    Guid.NewGuid().ToString(),
                    $"RequestUri:{uri}, Message:{exception.Message}");
            }
        }
    }
}
