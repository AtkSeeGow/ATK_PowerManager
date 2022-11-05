using Microsoft.AspNetCore.Http;
using PowerManager.Domain.Options;
using System;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace PowerManager.Web.Middleware
{
    public class BasicAuthMiddleware
    {
        private readonly RequestDelegate next;
        private readonly AuthorizationOptions authorizationOptions;

        public BasicAuthMiddleware(RequestDelegate next, AuthorizationOptions authorizationOptions)
        {
            this.next = next;
            this.authorizationOptions = authorizationOptions;
        }

        public async Task Invoke(HttpContext context)
        {
            string authorization = context.Request.Headers["Authorization"];
            if (authorization != null && authorization.StartsWith("Basic "))
            {
                var encodedUsernamePassword = authorization.Split(' ', 2, StringSplitOptions.RemoveEmptyEntries)[1]?.Trim();
                var decodedUsernamePassword = Encoding.UTF8.GetString(Convert.FromBase64String(encodedUsernamePassword));

                var account = decodedUsernamePassword.Split(':', 2)[0];
                var password = decodedUsernamePassword.Split(':', 2)[1];

                if (account.Equals(this.authorizationOptions.Account, StringComparison.InvariantCultureIgnoreCase) &&
                    password.Equals(this.authorizationOptions.Password))
                {
                    await next.Invoke(context);
                    return;
                }
            }

            context.Response.Headers["WWW-Authenticate"] = "Basic";

            context.Response.StatusCode = (int)HttpStatusCode.Unauthorized;
        }
    }
}