using Microsoft.AspNetCore.Http;
using Microsoft.Net.Http.Headers;
using PowerManager.Domain.Options;
using System;
using System.Net.Http.Headers;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace PowerManager.Api.Middleware
{
    public class MetricsBasicAuthMiddleware
    {
        private const string AuthenticationScheme = "Basic";
        private const string Realm = "metrics";

        private readonly RequestDelegate _next;
        private readonly MetricsBasicAuthOptions _options;

        public MetricsBasicAuthMiddleware(
            RequestDelegate next,
            MetricsBasicAuthOptions options)
        {
            _next = next ?? throw new ArgumentNullException(nameof(next));
            _options = options ?? throw new ArgumentNullException(nameof(options));
        }

        public async Task Invoke(HttpContext context)
        {
            if (!_options.IsConfigured)
            {
                context.Response.StatusCode = StatusCodes.Status503ServiceUnavailable;
                await context.Response.WriteAsync("Metrics Basic Auth is not configured.");
                return;
            }

            if (!IsAuthorized(context.Request.Headers[HeaderNames.Authorization]))
            {
                context.Response.Headers[HeaderNames.WWWAuthenticate] =
                    $"{AuthenticationScheme} realm=\"{Realm}\", charset=\"UTF-8\"";
                context.Response.StatusCode = StatusCodes.Status401Unauthorized;
                return;
            }

            await _next(context);
        }

        private bool IsAuthorized(string authorizationHeader)
        {
            if (!AuthenticationHeaderValue.TryParse(authorizationHeader, out var header) ||
                !AuthenticationScheme.Equals(header.Scheme, StringComparison.OrdinalIgnoreCase) ||
                string.IsNullOrWhiteSpace(header.Parameter))
            {
                return false;
            }

            try
            {
                var suppliedCredentials = Convert.FromBase64String(header.Parameter);
                var expectedCredentials = Encoding.UTF8.GetBytes($"{_options.Username}:{_options.Password}");

                return CryptographicOperations.FixedTimeEquals(
                    suppliedCredentials,
                    expectedCredentials);
            }
            catch (FormatException)
            {
                return false;
            }
        }
    }
}
