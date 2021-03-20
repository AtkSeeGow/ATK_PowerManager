using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using PowerManager.Domain;
using PowerManager.Domain.Options;
using System;

namespace PowerManager.Web.Api
{
    [Route("Api/[controller]/[action]")]
    [ApiController]
    [Authorize(AuthenticationSchemes = "PermissionHandler")]
    public class SerialController : Controller
    {
        private readonly HttpUtility httpUtility;
        private readonly SerialOptions serialOptions;

        public SerialController(
            HttpUtility httpUtility,
            SerialOptions serialOptions)
        {
            this.httpUtility = httpUtility;
            this.serialOptions = serialOptions;
        }

        [HttpGet]
        public ActionResult ReadDigitalValue()
        {
            var validResult = new ValidResult();
            var uri = new Uri(new Uri(serialOptions.Url), "/ReadDigitalValue/");
            this.httpUtility.Get(validResult, uri);
            return Ok(new { validResult });
        }

        [HttpGet]
        public ActionResult SwitchOpticalCoupler(int interval)
        {
            var validResult = new ValidResult();
            var uri = new Uri(new Uri(serialOptions.Url), $"/SwitchOpticalCoupler/{interval}");
            this.httpUtility.Get(validResult, uri);
            return Ok(new { validResult });
        }
    }
}
