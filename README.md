# fix-engine-equities-prod-observed — Instrumented Demo

Same 665us/1M zero-copy core as prod v1 7ffddde, but you SEE state changes live.

## What you observe
- [epoll] create add_fd recv re-arm ONESHOT
- Session State Machine: DISCONNECTED -> LOGON_SENT -> LOGON_RECV -> ACTIVE -> RESEND -> LOGOUT
- Gap detection: recv_seq > exp_seq => 35=2 ResendRequest 7= 16=
- mmap /dev/shm/fix_prod ready

## Build on Latitude 7480
