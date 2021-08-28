using Microsoft.Extensions.Logging;
using PowerManager.Domain;
using System;
using System.IO;
using System.Net.Http;

namespace PowerManager.Web
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

        public void Get(ValidResult validResult, Uri uri)
        {
            try
            {
                var client = httpClientFactory.CreateClient();

                var response = client.GetAsync(uri).Result;

                if (response.IsSuccessStatusCode)
                {
                    using var stream = response.Content.ReadAsStreamAsync().Result;
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
    }
}