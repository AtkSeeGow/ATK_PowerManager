using System.Collections.Generic;
using System.Runtime.Serialization;

namespace PowerManager.Domain
{
    [DataContract]
    public class DeviceOperationResult
    {
        [DataMember]
        public string Name { get; set; }

        [DataMember(Name = "isValid")]
        public bool IsValid => ErrorMessages.Count == 0;

        [DataMember]
        public Dictionary<string, string> SuccessMessages { get; } = new();

        [DataMember]
        public Dictionary<string, string> ErrorMessages { get; } = new();
    }
}
