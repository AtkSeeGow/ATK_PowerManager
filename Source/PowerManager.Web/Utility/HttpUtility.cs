using Microsoft.Extensions.Logging;
using PowerManager.Domain;
using System;
using System.IO;
using System.Net.Http;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;

namespace PowerManager.Web.Utility
{
    public class HttpUtility
    {
        private readonly ILogger<HttpUtility> logger;
        private readonly IHttpClientFactory httpClientFactory;

        public HttpUtility(
            ILogger<HttpUtility> logger,
            IHttpClientFactory httpClientFactory)
        {
            this.logger = logger;
            this.httpClientFactory = httpClientFactory;
        }

        public async Task Get(ValidResult validResult, Uri uri)
        {
            var httpClient = httpClientFactory.CreateClient();
            await this.package(validResult, uri, httpClient);
        }

        public async Task Heartbeat(ValidResult validResult, Uri uri)
        {
            var httpClient = httpClientFactory.CreateClient("Heartbeat");
            await this.package(validResult, uri, httpClient);
        }

        #region Private

        private async Task package(ValidResult validResult, Uri uri, HttpClient httpClient)
        {
            try
            {
                var response = await httpClient.GetAsync(uri);

                if (response.IsSuccessStatusCode)
                {
                    using var stream = await response.Content.ReadAsStreamAsync();
                    using (var streamReader = new StreamReader(stream))
                    {
                        var value = streamReader.ReadToEnd().Replace("\r", "").Replace("\n", "");
                        validResult.SuccessMessages.Add("Value", value);
                    }
                }
                else
                {
                    var message = $"RequestUri:{uri}, StatusCode:{response.StatusCode}";
                    logger.LogDebug(message);
                    validResult.ErrorMessages.Add(Guid.NewGuid().ToString(), message);
                }
            }
            catch (Exception exception)
            {
                logger.LogError(exception.ToString());
                validResult.ErrorMessages.Add(Guid.NewGuid().ToString(), $"RequestUri:{uri}, Message:{exception.Message}");
            }
        }

        #endregion
    }
}