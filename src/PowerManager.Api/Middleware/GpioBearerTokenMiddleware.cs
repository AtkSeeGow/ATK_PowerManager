using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
using Microsoft.Net.Http.Headers;
using PowerManager.Domain.Options;
using System;
using System.Net.Http.Headers;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace PowerManager.Api.Middleware
{
    public class GpioBearerTokenMiddleware
    {
        private const string AuthenticationScheme = "Bearer";
        private const string Realm = "gpio";

        private readonly RequestDelegate _next;
        private readonly GpioBearerTokenOptions _options;
        private readonly ILogger<GpioBearerTokenMiddleware> _logger;

        public GpioBearerTokenMiddleware(
            RequestDelegate next,
            GpioBearerTokenOptions options,
            ILogger<GpioBearerTokenMiddleware> logger)
        {
            _next = next ?? throw new ArgumentNullException(nameof(next));
            _options = options ?? throw new ArgumentNullException(nameof(options));
            _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        }

        public async Task Invoke(HttpContext context)
        {
            var endpoint = context.GetEndpoint();
            if (endpoint?.Metadata.GetMetadata<GpioBearerTokenAttribute>() == null)
            {
                await _next(context);
                return;
            }

            context.Response.Headers[HeaderNames.CacheControl] = "no-store";

            if (!_options.IsConfigured)
            {
                _logger.LogError(
                    "Rejected GPIO request because the Bearer token is not configured.");
                context.Response.StatusCode = StatusCodes.Status503ServiceUnavailable;
                await context.Response.WriteAsJsonAsync(new
                {
                    error = "gpio_bearer_token_not_configured"
                });
                return;
            }

            if (!IsAuthorized(context.Request.Headers[HeaderNames.Authorization]))
            {
                _logger.LogWarning(
                    "Rejected unauthorized GPIO request {Method} {Path} from {RemoteIpAddress}.",
                    context.Request.Method,
                    context.Request.Path,
                    context.Connection.RemoteIpAddress);
                context.Response.Headers[HeaderNames.WWWAuthenticate] =
                    $"{AuthenticationScheme} realm=\"{Realm}\"";
                context.Response.StatusCode = StatusCodes.Status401Unauthorized;
                await context.Response.WriteAsJsonAsync(new { error = "unauthorized" });
                return;
            }

            await _next(context);
        }

        private bool IsAuthorized(string authorizationHeader)
        {
            if (!AuthenticationHeaderValue.TryParse(authorizationHeader, out var header) ||
                !AuthenticationScheme.Equals(
                    header.Scheme,
                    StringComparison.OrdinalIgnoreCase) ||
                string.IsNullOrEmpty(header.Parameter))
            {
                return false;
            }

            var suppliedToken = Encoding.UTF8.GetBytes(header.Parameter);
            var expectedToken = Encoding.UTF8.GetBytes(_options.Token);
            return CryptographicOperations.FixedTimeEquals(
                suppliedToken,
                expectedToken);
        }
    }
}
