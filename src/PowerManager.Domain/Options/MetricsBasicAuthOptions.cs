namespace PowerManager.Domain.Options
{
    public class MetricsBasicAuthOptions
    {
        public string Username { get; set; }

        public string Password { get; set; }

        public bool IsConfigured =>
            !string.IsNullOrWhiteSpace(Username) &&
            !string.IsNullOrEmpty(Password);
    }
}
