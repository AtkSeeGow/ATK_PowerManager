#pragma once

class NtpTimeSynchronizer {
public:
  void begin();
  void update(bool networkAvailable);

private:
  bool timeSynchronized = false;
};
