#pragma once

struct TLBMonitoringFragment { 
  uint32_t header;
  uint32_t event_id;
  uint32_t orbit_id;
  uint32_t bc_id;
  uint32_t tap0;
  uint32_t tap1;
  uint32_t tap2;
  uint32_t tap3;
  uint32_t tap4;
  uint32_t tap5;
  uint32_t tav0;
  uint32_t tav1;
  uint32_t tav2;
  uint32_t tav3;
  uint32_t tav4;
  uint32_t tav5;
  uint32_t deadtime_veto_counter;
  uint32_t busy_veto_counter;
  uint32_t rate_limiter_veto_counter;
  uint32_t bcr_veto_counter;
}  __attribute__((__packed__));

