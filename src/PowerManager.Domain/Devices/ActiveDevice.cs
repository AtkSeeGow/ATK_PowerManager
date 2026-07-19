using System;
using System.Collections.Generic;

namespace PowerManager.Domain.Devices
{
    /// <summary>
    /// 由裝置主動呼叫伺服器
    /// </summary>
    public class ActiveDevice : DeviceBase
    {
        /// <summary>
        /// 最後上報資料的時間（UTC）
        /// </summary>
        public DateTimeOffset? LastReportTime { get; set; }

        public IDictionary<string, string> Properties { get; set; }
    }
}
