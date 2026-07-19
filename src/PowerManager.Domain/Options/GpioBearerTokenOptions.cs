namespace PowerManager.Domain.Options
{
    public class GpioBearerTokenOptions
    {
        public const int MinimumTokenLength = 32;

        public string Token { get; set; }

        public bool IsConfigured => Token?.Length >= MinimumTokenLength;
    }
}
