# gpsi_udp
Reads Pi GPS; senss sentences to specified UDP port

Accepts 1 command-line parameter:
  - port to broadcast on
  
e.g.:
  gps_udp 12345

To compile and link:
  cc gps_udp.c -o gps_udp
