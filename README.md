# ISR
Improved Stub Resolver

```text
  _____  _____ _____
 |_   _|/ ____|  __ \
   | | | (___ | |__) |
   | |  \___ \|  _  /
  _| |_ ____) | | \ \
 |_____|_____/|_|  \_\

```
[aɪsər]

isr - improved stub resolver - aims to provide dynamic conditional forwarding, extensible with javascript, on unix-like systems.

This comes in handy in cases like these:
    * When your router won't support NAT loopback, and you want to access your local device through the same domain no matter if you are in the network or not
    * When you want to use your VPN for few specific domains under specific circumstances
    * Anything else you can think of!
