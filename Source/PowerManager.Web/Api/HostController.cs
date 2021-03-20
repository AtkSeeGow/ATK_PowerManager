using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using PowerManager.Domain;
using PowerManager.Domain.Options;
using System;
using System.Net.Http;

namespace PowerManager.Web.Api
{
    [Route("Api/[controller]/[action]")]
    [ApiController]
    [Authorize(AuthenticationSchemes = "PermissionHandler")]
    public class HostController : Controller
    {
        private readonly HttpUtility httpUtility;
        private readonly HostOptions hostOptions;

        public HostController(
            HttpUtility httpUtility,
            HostOptions hostOptions)
        {
            this.httpUtility = httpUtility;
            this.hostOptions = hostOptions;
        }

        [HttpGet]
        public ActionResult Status()
        {
            var validResult = new ValidResult();
            var uri = new Uri(hostOptions.Url);
            this.httpUtility.Get(validResult, uri);
            return Ok(new { validResult });
        }
    }
}
